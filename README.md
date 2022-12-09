https://mathieukapfer.github.io/hello_mppa/

1. The generated Toc will be an ordered list
{:toc}


# Helper

All command below is written in main `makefile`
This "default" makefile carry an helper menu !
To get it, just type `make`:

It will produce the following output with the list of available target and related description
```
$ make

 Example of:

 Openmp example:
  - openmp-x86:    openmp x86
  - openmp-kvx:    openmp on mppa simulator (4 PE)
  - openmp-kvx-16: openmp on mppa simulator (16 PE)
  - openmp-kalray: same as above with Kalray makefile

 Opencl example:
  - opencl-1:      opencl with kernel code as nested string in code
  - opencl-2:      opencl with kernel in dedicated file (offline and online compilation)
  - opencl-3:      opencl with kernel using C/C++ native library
  - opencl-4:      opencl and out of order mode
  - opencl-5:      opencl and separate queue for code and data
```

To compile an example, just type `make` with the selected target as parameter!


# Hello mppa - part I - `openmp`

For openmp beginner, have a look at
[this](http://mksoft.fr/wiki/doku.php?id=code-parallel:open-mp)

## Test on host x86
### file: hello_mp.c

```c
/*
 * Example of open mp - *host* example
 * compile with: make hello_mp
 */

#include <sched.h>
#include <stdio.h>
#include <omp.h>

int main(void)
{
  // Usage of 'parallel': execute this line on all cpus
#pragma omp parallel
  printf("hello \n");


  // Usage of 'parallel for': dispatch index on all cpus
#pragma omp parallel for
  for (int index=0; index<10; index++)
    {
      int thread_num = omp_get_thread_num();
      int cpu_num = sched_getcpu();
      printf("Thread %3d is running on CPU %3d - (index:%d)\n",
             thread_num, cpu_num, index);
    }

  return 0;
}
```

### compile
Thanks to this makefile:
```makefile
CFLAGS=-fopenmp
CXXFLAGS=-fopenmp
```
compile with
```shell
$ make -f makefile.openmp hello_mp
cc -fopenmp    hello_mp.c   -o hello_mp
```
### execution

```shell
$ ./hello_mp
hello
hello
hello
hello
Thread   1 is running on CPU   3 - (index:3)
Thread   1 is running on CPU   3 - (index:4)
Thread   1 is running on CPU   3 - (index:5)
Thread   2 is running on CPU   1 - (index:6)
Thread   2 is running on CPU   1 - (index:7)
Thread   0 is running on CPU   1 - (index:0)
Thread   0 is running on CPU   1 - (index:1)
Thread   0 is running on CPU   1 - (index:2)
Thread   3 is running on CPU   2 - (index:8)
Thread   3 is running on CPU   2 - (index:9)
```

## Test on mppa

### Install toolchain [internal only]

Get the ACE (AccessCore for Embedded application):

    `git:software/applications/KAF_libraries `

Checkout the release you want

### Source environment

In `KAF_libraries` directories,

  - do once (or when you change KAF_LIBRARIES version)
      `./get_packages.sh && cd -`

  - do on each session:
      `source kEnv/kvxtools/.switch_env`

### file: hello_mppa_mp.c
```c
#include <sched.h>

#include <stdio.h>
#include <omp.h>
// add support for cos (Cluster OS) - for log purpose
// to get id of PE (process element) and cluster where we are running
#include <mppa_cos.h>

int main(void)
{
  // fix number of threads
  omp_set_num_threads(4);

  // Usage of 'parallel': execute this line on all cpus
#pragma omp parallel
  printf("hello \n");

  // Usage of 'parallel for': dispatch index on all cpus
#pragma omp parallel for
  for (int index=0; index<16; index++)
    {
      int thread_num = omp_get_thread_num();

      printf("Thread %-3d is running on MPPA cluster:%3d, PE:%d - (index:%d)\n",
             thread_num,
             mppa_cos_get_cluster_id(),
             mppa_cos_get_pe_id(),
             index);
    }

  return 0;
}

```
### compile

Thanks this makefile:

```makefile
CC=kvx-cos-gcc
CXX=kvx-cos-g++
CFLAGS=-fopenmp
CXXFLAGS=-fopenmp
```

compile with for 4 PE (default mode)

```shell
$ make -f makefile.simple hello_mppa_mp
kvx-cos-gcc -fopenmp    hello_mppa_mp.c   -o hello_mppa_mp
```

### execution on simulator
```shell
$ kvx-cluster -- hello_mppa_mp
hello
hello
hello
hello
Thread   0 is running on CPU   1 - (index:3)
Thread   0 is running on CPU   0 - (index:0)
Thread   0 is running on CPU   2 - (index:6)
Thread   0 is running on CPU   1 - (index:4)
Thread   0 is running on CPU   2 - (index:7)
Thread   0 is running on CPU   1 - (index:5)
Thread   0 is running on CPU   0 - (index:1)
Thread   0 is running on CPU   3 - (index:8)
Thread   0 is running on CPU   0 - (index:2)
Thread   0 is running on CPU   3 - (index:9)
```

### execution on simulator with `profile` log
The option `--profile` generate dir tree with all asm instructions exectuted by cluster and PE of cluster
```shell
$ kvx-cluster --profile -- hello_mppa_mp
...
mkapfer@coolup25:/work1/mkapfer/Projects/hello$ tree profile/
profile/
├── Cluster_0
│   ├── PE.0
│   ├── PE.1
│   ├── PE.2
│   ├── PE.3
│   └── RM.16
├── Cluster_1
├── Cluster_2
├── Cluster_3
└── Cluster_4


```

### execution on simulator - more than 4 PE (Process Element)
if you replace the line  `omp_set_num_threads(4);` by  `omp_set_num_threads(8);` and try to compile and execute the `hello_mppa_mp` example, you probably got this :

```shell
$ kvx-cluster -- hello_mppa_mp

libgomp: Thread creation failed: No more processes
```

You have reached the max number of PE in the default config, but not the max PE of each MPPA cluster which is 16 !. To fix this default configuration, add `--defsym=MPPA_COS_NB_CORES_LOG2=4` flag to the linker like this:

```makefile
CFLAGS=-fopenmp -Wl,--defsym=MPPA_COS_NB_CORES_LOG2=4
```
NOTE: The meaning of `...LOG2=4` value is that the max number of PE is `1 << 4` (2 power of 4) that is equal to 16.


Then after build and execution you got the execution on 8 PE as limited `by omp_set_num_threads(8);` (but you can go now until 16)

```shell
$ rm hello_mppa_mp && make -f makefile.simple hello_mppa_mp
kvx-cos-gcc -fopenmp -Wl,--defsym=MPPA_COS_NB_CORES_LOG2=4    hello_mppa_mp.c   -o hello_mppa_mp

$ kvx-cluster -- hello_mppa_mp
hello
hello
hello
hello
hello
hello
hello
hello
Thread   0 is running on CPU   6 - (index:8)
Thread   0 is running on CPU   0 - (index:0)
Thread   0 is running on CPU   4 - (index:6)
Thread   0 is running on CPU   1 - (index:2)
Thread   0 is running on CPU   2 - (index:4)
Thread   0 is running on CPU   3 - (index:5)
Thread   0 is running on CPU   5 - (index:7)
Thread   0 is running on CPU   0 - (index:1)
Thread   0 is running on CPU   1 - (index:3)
Thread   0 is running on CPU   7 - (index:9)
```


### compile with 'kalray' style makefile

Thanks to makefile below that include `Makefile.kalray` from toolchain:
```makefile
lflags=-fopenmp -Wl,--defsym=MPPA_COS_NB_CC=1 -Wl,--defsym=MPPA_COS_NB_CORES_LOG2=4 -Wl,--defsym=MPPA_COS_THREAD_PER_CORE_LOG2=0
cflags=-fopenmp -Wl,--defsym=MPPA_COS_NB_CC=1 -Wl,--defsym=MPPA_COS_NB_CORES_LOG2=4 -Wl,--defsym=MPPA_COS_THREAD_PER_CORE_LOG2=0

system:=cos

hello_mppa_mp-srcs := hello_mppa_mp.c
cluster-bin += hello_mppa_mp

include $(KALRAY_TOOLCHAIN_DIR)/share/make/Makefile.kalray
```

type the command:
```shell
$ make -f makefile.my_kalray hello_mppa_mp
  KVX_COS_CC		output/build/hello_mppa_mp_build/hello_mppa_mp.c.o
  KVX_COS_LD		output/bin/hello_mppa_mp
```

and execute with:
```shell
$ kvx-cluster --  output/bin/hello_mppa_mp
$ kvx-cluster --profile --  output/bin/hello_mppa_mp
```
NOTE: location of binaries have change now to `output/bin`

### execution on target

`$ kvx-jtag-runner --exec-file=cluster0:output/bin/hello_mppa_mp`


```
(kvxtools) mkapfer@coolup04:/work1/mkapfer/hello_mppa$ kvx-jtag-runner --exec-file=cluster0:output/bin/hello_mppa_mp
Cluster0@0.0: PE 12: hello
Cluster0@0.0: PE 1: hello
Cluster0@0.0: PE 11: hello
Cluster0@0.0: PE 15: hello
Cluster0@0.0: PE 7: hello
Cluster0@0.0: PE 0: hello
Cluster0@0.0: PE 3: hello
Cluster0@0.0: PE 2: hello
Cluster0@0.0: PE 9: hello
Cluster0@0.0: PE 10: hello
Cluster0@0.0: PE 4: hello
Cluster0@0.0: PE 13: hello
Cluster0@0.0: PE 8: hello
Cluster0@0.0: PE 5: hello
Cluster0@0.0: PE 6: hello
Cluster0@0.0: PE 14: hello
Cluster0@0.0: PE 5: Thread 5   is running on MPPA cluster:  0, PE:5 - (index:5)
Cluster0@0.0: PE 9: Thread 9   is running on MPPA cluster:  0, PE:9 - (index:9)
Cluster0@0.0: PE 13: Thread 13  is running on MPPA cluster:  0, PE:13 - (index:13)
Cluster0@0.0: PE 0: Thread 0   is running on MPPA cluster:  0, PE:0 - (index:0)
Cluster0@0.0: PE 14: Thread 14  is running on MPPA cluster:  0, PE:14 - (index:14)
Cluster0@0.0: PE 15: Thread 15  is running on MPPA cluster:  0, PE:15 - (index:15)
Cluster0@0.0: PE 3: Thread 3   is running on MPPA cluster:  0, PE:3 - (index:3)
Cluster0@0.0: PE 8: Thread 8   is running on MPPA cluster:  0, PE:8 - (index:8)
Cluster0@0.0: PE 4: Thread 4   is running on MPPA cluster:  0, PE:4 - (index:4)
Cluster0@0.0: PE 6: Thread 6   is running on MPPA cluster:  0, PE:6 - (index:6)
Cluster0@0.0: PE 12: Thread 12  is running on MPPA cluster:  0, PE:12 - (index:12)
Cluster0@0.0: PE 10: Thread 10  is running on MPPA cluster:  0, PE:10 - (index:10)
Cluster0@0.0: PE 11: Thread 11  is running on MPPA cluster:  0, PE:11 - (index:11)
Cluster0@0.0: PE 1: Thread 1   is running on MPPA cluster:  0, PE:1 - (index:1)
Cluster0@0.0: PE 2: Thread 2   is running on MPPA cluster:  0, PE:2 - (index:2)
Cluster0@0.0: PE 7: Thread 7   is running on MPPA cluster:  0, PE:7 - (index:7)
```

# Hello mppa - part II - `opencl`

For opencl beginner, have a look at
[this](http://mksoft.fr/wiki/doku.php?id=code-parallel:open-cl&s[]=opencl)

## clinfo
Need a little setup to make clinfo detect the MPPA target:

    $ LD_PRELOAD=$KALRAY_TOOLCHAIN_DIR/lib/libOpenCL.so clinfo


```
(kvxtools) mkapfer@coolup04:/work1/mkapfer/hello_mppa$   LD_PRELOAD=$KALRAY_TOOLCHAIN_DIR/lib/libOpenCL.so clinfo
Number of platforms                               1
  Platform Name                                   Portable Computing Language
  Platform Vendor                                 The pocl project
  Platform Version                                OpenCL 1.2
  Platform Profile                                EMBEDDED_PROFILE
  Platform Extensions

  Platform Name                                   Portable Computing Language
Number of devices                                 1
  Device Name                                     MPPA Coolidge
  Device Vendor                                   KALRAY Corporation
  Device Vendor ID                                0x0
  Device Version                                  OpenCL 1.2
  Driver Version                                  MPPA OpenCL Driver 1.0
  Device OpenCL C Version                         OpenCL C 1.2
  Device Type                                     Accelerator
  Device Profile                                  EMBEDDED_PROFILE
  Device Available                                Yes
  Compiler Available                              Yes
  Linker Available                                Yes
  Max compute units                               5
  Max clock frequency                             1000MHz
  Device Partition                                (core)
    Max number of sub-devices                     5
    Supported partition types                     equally, by counts, None, None, by <unknown> (0x100000000)
  Max work item dimensions                        3
  Max work item sizes                             16x16x16
  Max work group size                             16
  ...
```


## first sample of code

We use the sample of code: [opencl-sample1.cpp](opencl-sample1.cpp)

### compile with mppa opencl backend

Source the mppa environment, then use the following minimal makefile below
```
CFLAGS   += -isystem $(KALRAY_TOOLCHAIN_DIR)/include
CXXFLAGS += -isystem $(KALRAY_TOOLCHAIN_DIR)/include
LDFLAGS  += -L$(KALRAY_TOOLCHAIN_DIR)/lib -lOpenCL
```
or ready to use makefile below:
```
make -f makefile.opencl opencl-sample1
```

### execute on host

```
(kvxtools) $ ./opencl-sample1
Using platform: Portable Computing Language
Using device: MPPA Coolidge
 result:
0 2 4 3 5 7 6 8 10 9
```

### execute on host with MPPA POCL log enable

```
(kvxtools) mkapfer@coolup04:/work1/mkapfer/hello_mppa$ POCL_DEBUG=1 ./opencl-sample1
Using platform: Portable Computing Language
[2020-11-28 15:23:39.714182855]POCL: in fn pocl_mppa_init_device at line 136:
  |   GENERAL |  Platform(s) constructor proceed
[2020-11-28 15:23:41.200970277]POCL: in fn pocl_mppa_init_device at line 154:
  |   GENERAL |  Platform(s) booted
Using device: MPPA Coolidge
[2020-11-28 15:23:41.204993045]POCL: in fn compile_and_link_program at line 660:
  |   GENERAL |  building from sources for device 0
[2020-11-28 15:23:41.297313613]POCL: in fn compile_and_link_program at line 660:
  |   GENERAL |  building from sources for device 0
[2020-11-28 15:23:41.386616623]POCL: in fn POclCreateCommandQueue at line 42:
  |   GENERAL |  Create Command queue on device 0
[2020-11-28 15:23:41.386668811]POCL: in fn pocl_mppa_write at line 28:
  |   GENERAL |  Writing 40 bytes to device address (nil), offset 0 from src_host_ptr 0x7fff52339600
[2020-11-28 15:23:41.386758699]POCL: in fn pocl_mppa_write at line 28:
  |   GENERAL |  Writing 40 bytes to device address (nil), offset 0 from src_host_ptr 0x7fff52339630
[2020-11-28 15:23:41.405389986]POCL: in fn pocl_mppa_compile_kernel at line 59:
  |   GENERAL |  Final compilation: clang --target=kvx-unknown-cos -g -fPIC -ffunction-sections -O2  -c /nfs/home/mkapfer/.cache/pocl/kcache/ON/DBHEJJBOJMCDOLPMGPDCGONPKNPEHAAKJHPGB/program_final.bc -o /nfs/home/mkapfer/.cache/pocl/kcache/ON/DBHEJJBOJMCDOLPMGPDCGONPKNPEHAAKJHPGB/program_final.bc.o
[2020-11-28 15:23:41.484102054]POCL: in fn pocl_mppa_compile_kernel at line 59:
  |   GENERAL |  Final compilation: clang --target=kvx-unknown-cos -g -fPIC -ffunction-sections -O2  -c /nfs/home/mkapfer/.cache/pocl/kcache/ON/DBHEJJBOJMCDOLPMGPDCGONPKNPEHAAKJHPGB/program_final_spmd.bc -o /nfs/home/mkapfer/.cache/pocl/kcache/ON/DBHEJJBOJMCDOLPMGPDCGONPKNPEHAAKJHPGB/program_final_spmd.bc.o
[2020-11-28 15:23:41.521160088]POCL: in fn pocl_mppa_compile_kernel at line 92:
  |   GENERAL |  Final link: kvx-cos-gcc -fPIC /nfs/home/mkapfer/.cache/pocl/kcache/ON/DBHEJJBOJMCDOLPMGPDCGONPKNPEHAAKJHPGB/program_final.bc.o /nfs/home/mkapfer/.cache/pocl/kcache/ON/DBHEJJBOJMCDOLPMGPDCGONPKNPEHAAKJHPGB/program_final_spmd.bc.o -shared -o /nfs/home/mkapfer/.cache/pocl/kcache/ON/DBHEJJBOJMCDOLPMGPDCGONPKNPEHAAKJHPGB/program_final.bc.so -Wl,--soname=/nfs/home/mkapfer/.cache/pocl/kcache/ON/DBHEJJBOJMCDOLPMGPDCGONPKNPEHAAKJHPGB/program_final.bc.so -Wl,--wrap=malloc -Wl,--wrap=calloc -Wl,--wrap=realloc -Wl,--wrap=free -Wl,--wrap=memalign -Wl,--wrap=pthread_create -nostdlib   -lgcc -Wl,--defsym=_KVX_DIVMOD_ZERO_RETURN_ZERO=1
[2020-11-28 15:23:41.532971932]POCL: in fn pocl_binary_serialize at line 632:
  |   GENERAL |  serializing program.so: /nfs/home/mkapfer/.cache/pocl/kcache/ON/DBHEJJBOJMCDOLPMGPDCGONPKNPEHAAKJHPGB/program_final.bc.so
[2020-11-28 15:23:41.533841657]POCL: in fn pocl_binary_serialize at line 652:
  |   GENERAL |  serializing program.bc: /nfs/home/mkapfer/.cache/pocl/kcache/ON/DBHEJJBOJMCDOLPMGPDCGONPKNPEHAAKJHPGB/program.bc
[2020-11-28 15:23:41.533868598]POCL: in fn pocl_binary_serialize at line 657:
  |   GENERAL |  serializing program_spmd.bc: /nfs/home/mkapfer/.cache/pocl/kcache/ON/DBHEJJBOJMCDOLPMGPDCGONPKNPEHAAKJHPGB/program_spmd.bc
[2020-11-28 15:23:41.533891420]POCL: in fn serialize_kernel_cachedir at line 359:
  |   GENERAL |  Kernel simple_add: recur serializing cachedir /nfs/home/mkapfer/.cache/pocl/kcache/ON/DBHEJJBOJMCDOLPMGPDCGONPKNPEHAAKJHPGB/simple_add
[2020-11-28 15:23:41.538995078]POCL: in fn POclSetKernelArg at line 73:
  |   GENERAL |  Kernel      simple_add || SetArg idx   0 ||     int* || Local 0 || Size      8 || Value 0x7fff523395c0 || *Value 0x5632cc0fbfc0 || *(uint32*)Value:        0
[2020-11-28 15:23:41.539014003]POCL: in fn POclSetKernelArg at line 73:
  |   GENERAL |  Kernel      simple_add || SetArg idx   1 ||     int* || Local 0 || Size      8 || Value 0x7fff523395c0 || *Value 0x5632cc049fb0 || *(uint32*)Value:        0
[2020-11-28 15:23:41.539025274]POCL: in fn POclSetKernelArg at line 73:
  |   GENERAL |  Kernel      simple_add || SetArg idx   2 ||     int* || Local 0 || Size      8 || Value 0x7fff523395c0 || *Value 0x5632cc03fe90 || *(uint32*)Value:        0
[2020-11-28 15:23:41.539036465]POCL: in fn POclEnqueueNDRangeKernel at line 240:
  |   GENERAL |  Preferred WG size multiple 1
[2020-11-28 15:23:41.539046013]POCL: in fn POclEnqueueNDRangeKernel at line 456:
  |   GENERAL |  Queueing kernel simple_add with local size 10 x 1 x 1 group sizes 1 x 1 x 1...
[2020-11-28 15:23:41.539093091]POCL: in fn pocl_mppa_run at line 410:
  |   GENERAL |  Setting args 0 with pointer 0x910030a80
[2020-11-28 15:23:41.539114852]POCL: in fn pocl_mppa_run at line 410:
  |   GENERAL |  Setting args 1 with pointer 0x910030b00
[2020-11-28 15:23:41.539130942]POCL: in fn pocl_mppa_run at line 410:
  |   GENERAL |  Setting args 2 with pointer 0x910030b80
[2020-11-28 15:23:41.539141912]POCL: in fn mppa_start_kernel at line 337:
  |   GENERAL |  Sending command to start ndrange kernel, wg: 1 x 1 x 1, wi: 10 x 1 x 1
[2020-11-28 15:23:41.539406948]POCL: in fn mppa_start_kernel at line 359:
  |   GENERAL |  Kernel simple_add started successfully
[2020-11-28 15:23:41.539448986]POCL: in fn pocl_mppa_read at line 12:
  |   GENERAL |  Reading 40 bytes from device address (nil), offset 0 to dst_host_ptr 0x7fff52339660
 result:
[2020-11-28 15:23:41.543080473]POCL: in fn pocl_mppa_final_device at line 242:
  |   GENERAL |  Platform destructor proceed
[2020-11-28 15:23:41.544322213]POCL: in fn pocl_mppa_final_device at line 245:
  |   GENERAL |  Platform destroyed
0 2 4 3 5 7 6 8 10 9 (kvxtools) mkapfer@coolup04:/work1/mkapfer/hello_mppa$
```


## select how to managed kernel code

Open the source file [opencl-sample2.cpp](opencl-sample2.cpp), and select how to managed kernel code:
 - FromBinary: comment both 'define' line below
 - FromSource with cl file: uncomment the first 'define' only
 - FromSource nested here:  uncomment both define

```
...
// #define CL_PRG_FROM_SOURCES
// #define CL_PRG_FROM_STRING
...
```

### compile and run

```
make -f makefile.my_kalray_open_cl && POCL_DEBUG=1 ./output/bin/opencl_sample2
```
## opencl disptach: how to link opencl kernel with a C/C++ library

The program is made up of
 - main: [opencl-sample3.cpp](opencl-sample3.cpp),
 - kernel code: [opencl-sample3.cl](opencl-sample3.cl)
 - c/C++ library: [sample3_kernel.cpp](sample3_kernel.cpp)

### compile and run

```
TEST_NAME=sample4 KERNEL_NAME=sample4 make -f makefile.my_kalray_open_cl_dispatch && ./output/bin/opencl_sample4
```

## opencl disptach & out of order

This is axample of out of order queue that allow the host to push buffer asynchronously to the target:

 - main: [opencl-sample4.cpp](opencl-sample4.cpp),
 - kernel code: [opencl-sample4.cl](opencl-sample4.cl)
 - c/C++ library: [sample4_kernel.cpp](sample4_kernel.cpp)

### compile and run

```
make -f makefile.my_kalray_open_cl_dispatch_out_of_order && ./output/bin/opencl_sample4
```


## opencl disptach & 2 messages queues

Instead of previous 'out of order' mode, this example show how to use two queues to allow the host to push set of data and one command separatly:

 - main: [opencl-sample5.cpp](opencl-sample5.cpp),
 - kernel code: [opencl-sample4.cl](opencl-sample4.cl)
 - C/C++ library: [sample4_kernel.cpp](sample4_kernel.cpp)

### compile and run

```
TEST_NAME=sample5 KERNEL_NAME=sample4 make -f makefile.my_kalray_open_cl_dispatch && ./output/bin/opencl_sample5
```
