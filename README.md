1. The generated Toc will be an ordered list
{:toc}

# Hello mppa - part I - `openmp`

TODO: Fix mppa example (replace CPU per Cluster:%d, PE:%d)

## Test on host x86
### code

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
```
$ make hello_mp
cc -fopenmp    hello_mp.c   -o hello_mp
```
### execution

```
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

### Install toolchain
  - Get the ACE (AccessCore for Embedded application):
  `git:software/applications/KAF_applications `

### Source environment
  - Source the environment files:
    `source KAF_applications/kEnv/kvxtools/opt/kalray/accesscore/kalray.sh`

### code
```C
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
  for (int index=0; index<10; index++)
    {
      printf("Thread %3d is running on CPU %3d - (index:%d)\n",
             mppa_cos_get_cluster_id(), // get which cluster is used
             mppa_cos_get_pe_id(),      // get which PE is running
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

compile with:

```
$ make -f makefile.simple hello_mppa_mp
kvx-cos-gcc -fopenmp    hello_mppa_mp.c   -o hello_mppa_mp
```

### execution (on simulator)
```
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

### execution with profiling log (on simulator)
The option `--profile` generate dir tree with all asm instructions exectuted by cluster and PE of cluster
```
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

### execution on more than 4 PE (Process Element)
if you replace the line  `omp_set_num_threads(4);` by  `omp_set_num_threads(8);` and try to compile and execute the `hello_mppa_mp` example, you probably got this :

```
$ kvx-cluster -- hello_mppa_mp

libgomp: Thread creation failed: No more processes
```

You have reached the max number of PE in the default config, but not the max PE of each MPPA cluster which is 16 !. To fix this default configuration, add `--defsym=MPPA_COS_NB_CORES_LOG2=4` flag to the linker like this:

```
CFLAGS=-fopenmp -Wl,--defsym=MPPA_COS_NB_CORES_LOG2=4
```
NOTE: The meaning of `...LOG2=4` value is that the max number of PE is `1 << 4` (2 power of 4) that is equal to 16.


Then after build and execution you got the execution on 8 PE as limited `by omp_set_num_threads(8);` (but you can go now until 16)

```
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
```Makefile
lflags=-fopenmp -Wl,--defsym=MPPA_COS_NB_CC=1 -Wl,--defsym=MPPA_COS_NB_CORES_LOG2=4 -Wl,--defsym=MPPA_COS_THREAD_PER_CORE_LOG2=0
cflags=-fopenmp -Wl,--defsym=MPPA_COS_NB_CC=1 -Wl,--defsym=MPPA_COS_NB_CORES_LOG2=4 -Wl,--defsym=MPPA_COS_THREAD_PER_CORE_LOG2=0

system:=cos

hello_mppa_mp-srcs := hello_mppa_mp.c
cluster-bin += hello_mppa_mp

include $(KALRAY_TOOLCHAIN_DIR)/share/make/Makefile.kalray
```

type the command:
```
$ make -f makefile.my_kalray all
  KVX_COS_CC		output/build/hello_mppa_mp_build/hello_mppa_mp.c.o
  KVX_COS_LD		output/bin/hello_mppa_mp
```

and execute with:
```
$ kvx-cluster --  output/bin/hello_mppa_mp
$ kvx-cluster --profile --  output/bin/hello_mppa_mp
```
NOTE: location of binaries have change now to `output/bin`


# Hello mppa - part II - `opencl`

## clinfo
Need a little setup to make clinfo detect the MPPA target:

    LD_PRELOAD=$KALRAY_TOOLCHAIN_DIR/lib/libOpenCL.so clinfo
