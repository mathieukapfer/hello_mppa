//#define CL_VERSION_1_2
//#define CL_TARGET_OPENCL_VERSION 120
//#define USE_KERNEL_FUNCTOR
#define CL_HPP_MINIMUM_OPENCL_VERSION 120
#define CL_HPP_TARGET_OPENCL_VERSION 120

#define CL_HPP_ENABLE_PROGRAM_CONSTRUCTION_FROM_ARRAY_COMPATIBILITY

#include <iostream>
//#include <CL/cl.hpp>
#include <CL/cl2.hpp>

// Select how to managed kernel code:
//  - FromBinary: comment all line below
//  - FromSource with cl file: uncomment first 'define' only
//  - FromSource nested here:  uncomment both define

// #define CL_PRG_FROM_SOURCES
// #define CL_PRG_FROM_STRING

void file_read(const char * filename, char* &program_buffer, size_t &program_size) {

  FILE *program_handle;
      //char *program_buffer, *program_log;
      //size_t program_size, log_size;
      //int err;

    //Read program file and place content into buffer
    program_handle = fopen(filename, "r");
    if (program_handle == NULL) {
        perror("Couldn't find the program file");
        exit(1);
    }

    fseek(program_handle, 0, SEEK_END);
    program_size = ftell(program_handle);
    rewind(program_handle);
    program_buffer = (char*)malloc(program_size + 1);
    program_buffer[program_size] = '\0';
    fread(program_buffer, sizeof(char), program_size, program_handle);
    fclose(program_handle);

    return;
}

int main(){
  cl_int err;

    //get all platforms (drivers)
    std::vector<cl::Platform> all_platforms;
    cl::Platform::get(&all_platforms);
    if(all_platforms.size()==0){
        std::cout<<" No platforms found. Check OpenCL installation!\n";
        exit(1);
    }
    cl::Platform default_platform=all_platforms[0];
    std::cout << "Using platform: "<<default_platform.getInfo<CL_PLATFORM_NAME>()<<"\n";

    //get default device of the default platform
    std::vector<cl::Device> all_devices;
    default_platform.getDevices(CL_DEVICE_TYPE_ALL, &all_devices);
    if(all_devices.size()==0){
        std::cout<<" No devices found. Check OpenCL installation!\n";
        exit(1);
    }
    cl::Device default_device=all_devices[0];
    std::cout<< "Using device: "<<default_device.getInfo<CL_DEVICE_NAME>()<<"\n";

    cl::Context context({default_device});


#ifdef CL_PRG_FROM_SOURCES
    cl::Program::Sources sources;

#ifdef CL_PRG_FROM_STRING
    // kernel calculates for each element C=A+B
    std::string kernel_code=
            "   void kernel simple_add(global const int* A, global const int* B, global int* C){       "
            "       C[get_global_id(0)]=A[get_global_id(0)]+B[get_global_id(0)];                 "
            "   }                                                                               ";
    sources.push_back({kernel_code.c_str(),kernel_code.length()});
#else
    const char filename[] ="opencl-sample2.cl";
    char *program_buffer;
    size_t program_size;

    file_read(filename, program_buffer, program_size);
    sources.push_back({program_buffer, program_size});
    // todo : add free(program_buffer)
#endif

    cl::Program program(context,sources);

#else
    // Create program from binary (offline compilation)
    const char filename[] ="output/opencl_kernels/opencl-sample2.cl.pocl";
    cl::Program::Binaries binaries;

    char *program_buffer;
    size_t program_size;

    file_read(filename, program_buffer, program_size);
    binaries.push_back({static_cast<void *>(program_buffer), program_size});

    std::vector<cl_int> binaryStatus;
    cl::Program program(context, {default_device}, {binaries}, &binaryStatus, &err);

    std::cout << "err:" << err << std::endl;
    std::cout << "binaryStatus[0]:" << binaryStatus[0] << std::endl;

#endif

    if(program.build({default_device})!=CL_SUCCESS){
      std::cout<<" Error building: "<<program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(default_device)<<"\n";
      exit(1);
    }

    // create buffers on the device
    cl::Buffer buffer_A(context,CL_MEM_READ_WRITE,sizeof(int)*10);
    cl::Buffer buffer_B(context,CL_MEM_READ_WRITE,sizeof(int)*10);
    cl::Buffer buffer_C(context,CL_MEM_READ_WRITE,sizeof(int)*10);

    int A[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    int B[] = {0, 1, 2, 0, 1, 2, 0, 1, 2, 0};

    //create queue to which we will push commands for the device.
    cl::CommandQueue queue(context,default_device);

    //write arrays A and B to the device
    queue.enqueueWriteBuffer(buffer_A,CL_TRUE,0,sizeof(int)*10,A);
    queue.enqueueWriteBuffer(buffer_B,CL_TRUE,0,sizeof(int)*10,B);


    //run the kernel
#ifdef USE_KERNEL_FUNCTOR
    cl::KernelFunctor simple_add(cl::Kernel(program,"simple_add"),queue,cl::NullRange,cl::NDRange(10),cl::NullRange);
    simple_add(buffer_A,buffer_B,buffer_C);
#else
    //alternative way to run the kernel
    cl::Kernel kernel_add=cl::Kernel(program,"simple_add", &err);

    std::cout << "err: " << err << std::endl;
    std::cout << "program: " << program() << std::endl;
    std::cout << "kernel_add: " << kernel_add() << std::endl;

    kernel_add.setArg(0,buffer_A);
    kernel_add.setArg(1,buffer_B);
    kernel_add.setArg(2,buffer_C);
    queue.enqueueNDRangeKernel(kernel_add,cl::NullRange,cl::NDRange(10),cl::NullRange);
    queue.finish();
#endif


    int C[10];
    //read result C from the device to array C
    queue.enqueueReadBuffer(buffer_C,CL_TRUE,0,sizeof(int)*10,C);

    std::cout<<" result: \n";
    for(int i=0;i<10;i++){
        std::cout<<C[i]<<" ";
    }

    return 0;

}