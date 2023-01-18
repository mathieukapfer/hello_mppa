#include "sample8_sizing.h"

// C-native functions
__attribute__((mppa_native))
int sample8_helloworld(__global const int* magic);

__attribute__((mppa_native))
int sample8_add(__global const int* A, __global const int* B, __global int* C);


// OpenCL-C wrapper kernels
#ifdef ENABLE_WG_SIZE
__attribute__((reqd_work_group_size(1, 1, 1)))
#endif
__kernel void simple_add(__global const int* A, __global const int* B, __global int* C) {
    int offset = get_global_id(0) * (TABLE_SIZE * VECTOR_SIZE) ;
  sample8_add(&A[offset], &B[offset], &C[offset]);
}
