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

// Time measurement
#include <chrono>

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
  device_config.first_cu=0; // define the fist cluster linked to the device
  device_config.flags |= CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE;

  // Try this to remove profiling ***********************************************************!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  // device_config.flags = CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE;


  std::shared_ptr<KAF::KAF_Device> new_device = kaf_orb.create_new_device(device_config);

  // Create program from binary (offline compilation)
  const char program_file[] ="opencl-sample7.cl.pocl";
  KAF::KAF_OpenCLWhiteBox_service my_cl_application_inst(new_device);
  std::shared_ptr<KAF::KAF_Program> basic_program =
        my_cl_application_inst.create_kaf_program(program_file);
  std::unique_ptr<KAF::KAF_Kernel> basic_kernel =
        basic_program->get_kernel("simple_add");

  // test sizing
    static const int NB_LOOP = 20;
    static const int NB_VALUES = 100;

    // create buffers on the device
    cl_mem_flags mem_flag = CL_MEM_READ_WRITE; // | CL_MEM_LOCAL_0_MPPA;
    cl::Buffer buffer_A(context, mem_flag, sizeof(int)*NB_VALUES);
    cl::Buffer buffer_B(context, mem_flag, sizeof(int)*NB_VALUES);
    cl::Buffer buffer_C(context, mem_flag, sizeof(int)*NB_VALUES);


    // Prepare data set
    int A[NB_LOOP][NB_VALUES];
    int B[NB_LOOP][NB_VALUES];
    int C[NB_LOOP][NB_VALUES];
    int Check[NB_LOOP][NB_VALUES];

    for(int loop_index=0; loop_index < NB_LOOP; loop_index++)
      for(int index=0; index<NB_VALUES; index++) {
          A[loop_index][index] = rand()%255;
          B[loop_index][index] = rand()%255;
          Check[loop_index][index] = A[loop_index][index] + B[loop_index][index];
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
      auto start = chrono::steady_clock::now();

      //write arrays A and B to the device
      queue.enqueueWriteBuffer(buffer_A,CL_TRUE,0,sizeof(int)*NB_VALUES, &A[loop_index][0], NULL, &event_in[0]);
      queue.enqueueWriteBuffer(buffer_B,CL_TRUE,0,sizeof(int)*NB_VALUES, &B[loop_index][0], NULL, &event_in[1]);

      events_in.push_back(event_in[0]);
      events_in.push_back(event_in[1]);

      queue.enqueueNDRangeKernel(kernel_add,cl::NullRange,cl::NDRange(NB_VALUES), cl::NullRange, &events_in, &event_exec);

      //read result C from the device to array C
      events_exec.push_back(event_exec);
      queue.enqueueReadBuffer(buffer_C,CL_TRUE, 0, sizeof(int)*NB_VALUES, &C[loop_index][0], &events_exec, NULL);


      // stop timer
      auto end = chrono::steady_clock::now();

      bool res = true;
      for(int i=0;i<NB_VALUES;i++) {
        if (C[loop_index][i] != Check[loop_index][i]) {
          res = false;
          std::cout << "i:" << i << ":"
                    << " A[i]=" << A[loop_index][i]
                    << " B[i]=" << B[loop_index][i]
                    << " C[i]=" << C[loop_index][i]
                    << " != " << Check[loop_index][i] << endl;
        }
      }
      if (!res) {
        std::cout << "Error !" << endl;
      }

      std::cout << "Elapsed time in microseconds: "
                << chrono::duration_cast<chrono::nanoseconds>(end - start).count() / 1000
                << " us" << endl;
    }

    // !!!!!!!!!!!!!!!!!!!!!! when ???????????????????????????
    queue.finish();

    // TODO: release KAF


    return 0;

}