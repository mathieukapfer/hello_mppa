//#define CL_VERSION_1_2
//#define CL_TARGET_OPENCL_VERSION 120
//#define USE_KERNEL_FUNCTOR
#define CL_HPP_MINIMUM_OPENCL_VERSION 120
#define CL_HPP_TARGET_OPENCL_VERSION 120

#define CL_HPP_ENABLE_PROGRAM_CONSTRUCTION_FROM_ARRAY_COMPATIBILITY

#include <iostream>
#include <unistd.h>

// OpenCL api
//#include <CL/cl.hpp>
#include <CL/cl2.hpp>

// KAF api
#include <kaf_orb.hpp>
#include <kaf_services/opencl-whitebox/opencl-whitebox_service.hpp>

// Vecor & table sizing
#include "sample8_sizing.h"


// Time measurement
#include <chrono>

// Optional code
#define PROF
// #define KEEP_PROF_OPENCL
#define CHECK
#define LOG_ERR

using namespace std;
// Select how to managed kernel code:
//  - FromBinary: comment all line below
//  - FromSource with cl file: uncomment first 'define' only
//  - FromSource nested here:  uncomment both define


int main(){

  // Create context
  KAF::KAF_ObjectRequestBroker kaf_orb;
  kaf_orb.add_kernel_search_paths("output/opencl_kernels");
  kaf_orb.init();
  cl::Context context = kaf_orb.get_cpp_opencl_object();
  KAF::KAF_DeviceConfig device_config;
  // define mppa ressources in term of cluster
  device_config.cu_count=1; // define number of cluster linked to the device
  device_config.first_cu=0; // define the first cluster linked to the device
#ifdef KEEP_PROF_OPENCL
    device_config.flags |= CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE;
#else
    device_config.flags = CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE;
#endif


  std::shared_ptr<KAF::KAF_Device> new_device = kaf_orb.create_new_device(device_config);

  // Create program from binary (offline compilation)
  const char program_file[] ="opencl-sample8.cl.pocl";
  KAF::KAF_OpenCLWhiteBox_service my_cl_application_inst(new_device);
  std::shared_ptr<KAF::KAF_Program> basic_program =
        my_cl_application_inst.create_kaf_program(program_file);
  std::unique_ptr<KAF::KAF_Kernel> basic_kernel =
        basic_program->get_kernel("simple_add");

  // test sizing
  static const int NB_LOOP = 5;
  static const int NB_VALUES = 80*16*10; //273*12*14;

    // create buffers on the device
    //cl_mem_flags mem_flag = CL_MEM_READ_WRITE ;

    // create buffers on the device localted in SMEM !
    cl_mem_flags mem_flag = CL_MEM_READ_WRITE | CL_MEM_LOCAL_0_MPPA;

    cl::Buffer buffer_A(context, mem_flag, sizeof(int)*NB_VALUES);
    cl::Buffer buffer_B(context, mem_flag, sizeof(int)*NB_VALUES);
    cl::Buffer buffer_C(context, mem_flag, sizeof(int)*NB_VALUES);


    // Prepare data set
    int A[NB_LOOP][NB_VALUES];
    int B[NB_LOOP][NB_VALUES];
    int C[NB_LOOP][NB_VALUES];
#ifdef CHECK
    int Check[NB_LOOP][NB_VALUES];
#endif

    for(int loop_index=0; loop_index < NB_LOOP; loop_index++)
      for(int index=0; index<NB_VALUES; index++) {
          A[loop_index][index] = rand()%255;
          B[loop_index][index] = rand()%255;
#ifdef CHECK
          Check[loop_index][index] = A[loop_index][index] + B[loop_index][index];
#endif
      }


    //create queue to which we will push commands for the device.
    cl::CommandQueue queue = new_device->get_cpp_opencl_object();

    //run the kernel
    cl::Kernel kernel_add= basic_kernel->get_cpp_opencl_object();

    // std::cout << "err: " << err << std::endl;
    // std::cout << "program: " << program() << std::endl;
    // std::cout << "kernel_add: " << kernel_add() << std::endl;

    kernel_add.setArg(0,buffer_A);
    kernel_add.setArg(1,buffer_B);
    kernel_add.setArg(2,buffer_C);


    cl::Event event_in[2];
    cl::Event event_exec ;
    vector<cl::Event> events_in;
    vector<cl::Event> events_exec;

    for(int loop_index=0; loop_index<NB_LOOP; loop_index++) {
      // start timer
#ifdef PROF
      auto start = chrono::steady_clock::now();
#endif

      //write arrays A and B to the device
      queue.enqueueWriteBuffer(buffer_A,CL_TRUE,0,sizeof(int)*NB_VALUES, &A[loop_index][0], NULL, &event_in[0]);
      queue.enqueueWriteBuffer(buffer_B,CL_TRUE,0,sizeof(int)*NB_VALUES, &B[loop_index][0], NULL, &event_in[1]);

      events_in.push_back(event_in[0]);
      events_in.push_back(event_in[1]);

      // execute
#ifdef PROF
      auto step1 = chrono::steady_clock::now();
#endif
      cl_int err = queue.enqueueNDRangeKernel(
        kernel_add,cl::NullRange,
        cl::NDRange(NB_VALUES/ (TABLE_SIZE * VECTOR_SIZE) ),   // global size
        cl::NDRange(16),          // local size
        //cl::NullRange,          // local size (default value)
        //NULL, NULL              // in order mode
        &events_in, &event_exec   // out of order mode
        );
      if (err < 0) {
        cout << "enqueueNDRangeKernel error:" << err << std::endl;
      }

      // read result C from the device to array C
      events_exec.push_back(event_exec);
#ifdef PROF
      auto step2 = chrono::steady_clock::now();
#endif
      queue.enqueueReadBuffer(buffer_C,CL_TRUE, 0, sizeof(int)*NB_VALUES, &C[loop_index][0], &events_exec, NULL);


      // stop timer
#ifdef PROF
      auto end = chrono::steady_clock::now();
#endif

#ifdef CHECK
      // check resulat
      bool res = true;
      for(int i=0;i<NB_VALUES;i++) {
        if (C[loop_index][i] != Check[loop_index][i]) {
          res = false;
#ifdef LOG_ERR
          std::cout << "i:" << i << ":"
                    << " A[i]=" << A[loop_index][i]
                    << " B[i]=" << B[loop_index][i]
                    << " C[i]=" << C[loop_index][i]
                    << " != " << Check[loop_index][i] << endl;
#endif
        }
      }
      if (!res) {
        std::cout << "Error !" << endl;
      }
#endif

#ifdef PROF
      std::cout << "Elapsed time: "
                << " write buf: " << chrono::duration_cast<chrono::nanoseconds>(step1 - start).count()
                << " ndrange: "  << chrono::duration_cast<chrono::nanoseconds>(step2 - step1).count()
                << " read buf: " << chrono::duration_cast<chrono::nanoseconds>(end - step2).count()
                << " total: " << chrono::duration_cast<chrono::nanoseconds>(end - start).count()
                << " (ns)" << endl;
#endif
    }
    // !!!!!!!!!!!!!!!!!!!!!! when ???????????????????????????
    queue.finish();

    // TODO: release KAF


    return 0;

}