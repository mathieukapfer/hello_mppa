//#define CL_VERSION_1_2
//#define CL_TARGET_OPENCL_VERSION 120
//#define USE_KERNEL_FUNCTOR
#define CL_HPP_MINIMUM_OPENCL_VERSION 120
#define CL_HPP_TARGET_OPENCL_VERSION 120

#define CL_HPP_ENABLE_PROGRAM_CONSTRUCTION_FROM_ARRAY_COMPATIBILITY

#include <iostream>
 #include <unistd.h>
//#include <CL/cl.hpp>
#include <CL/cl2.hpp>

#include <kaf_orb.hpp>
#include <kaf_services/opencl-whitebox/opencl-whitebox_service.hpp>

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
  device_config.cu_count=5; // define number of cluster linked to the device
  device_config.first_cu=0; // define the fist cluster linked to the device
  device_config.flags |= CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE;
  std::shared_ptr<KAF::KAF_Device> new_device = kaf_orb.create_new_device(device_config);

  // Create program from binary (offline compilation)
  const char program_file[] ="opencl-sample4.cl.pocl";
  KAF::KAF_OpenCLWhiteBox_service my_cl_application_inst(new_device);
  std::shared_ptr<KAF::KAF_Program> basic_program =
        my_cl_application_inst.create_kaf_program(program_file);
  std::unique_ptr<KAF::KAF_Kernel> basic_kernel =
        basic_program->get_kernel("simple_add");

    // create buffers on the device
    cl::Buffer buffer_A(context,CL_MEM_READ_WRITE,sizeof(int)*10);
    cl::Buffer buffer_B(context,CL_MEM_READ_WRITE,sizeof(int)*10);
    cl::Buffer buffer_C(context,CL_MEM_READ_WRITE,sizeof(int)*10);
    cl::Buffer buffer_magic(context,CL_MEM_READ_WRITE,sizeof(int));

    int A[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    int B[] = {0, 1, 2, 0, 1, 2, 0, 1, 2, 0};
    int magic = 99;

    //create queue to which we will push commands for the device.
    cl::CommandQueue queue = new_device->get_cpp_opencl_object();

    //write arrays A and B to the device
    queue.enqueueWriteBuffer(buffer_A,CL_TRUE,0,sizeof(int)*10,A);
    queue.enqueueWriteBuffer(buffer_B,CL_TRUE,0,sizeof(int)*10,B);
    queue.enqueueWriteBuffer(buffer_magic,CL_TRUE,0,sizeof(int),&magic);

    //run the kernel
    cl::Kernel kernel_add= basic_kernel->get_cpp_opencl_object();

    // std::cout << "err: " << err << std::endl;
    // std::cout << "program: " << program() << std::endl;
    // std::cout << "kernel_add: " << kernel_add() << std::endl;

    kernel_add.setArg(0,buffer_A);
    kernel_add.setArg(1,buffer_B);
    kernel_add.setArg(2,buffer_C);
    kernel_add.setArg(3,buffer_magic);

    printf("enqueueNDRangeKernel\n");
    queue.enqueueNDRangeKernel(kernel_add,cl::NullRange,cl::NDRange(10),cl::NullRange);

    for (int i=0; i<5; i++) {
      sleep(1);
      printf("enqueueWriteBuffer %d\n", i);
      queue.enqueueWriteBuffer(buffer_magic, CL_FALSE, 0, sizeof(int),&i);
    }

    queue.finish();


    int C[10];
    //read result C from the device to array C
    queue.enqueueReadBuffer(buffer_C,CL_TRUE, 0, sizeof(int)*10, C);

    std::cout<<" result: \n";
    for(int i=0;i<10;i++) {
        std::cout<<C[i]<<" ";
    }

    return 0;

}