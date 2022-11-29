
// C-native functions
__attribute__((mppa_native))
int sample3_helloworld();

__attribute__((mppa_native))
int sample3_add(__global const int* A, __global const int* B, __global int* C);


// OpenCL-C wrapper kernels
#ifdef ENABLE_WG_SIZE
__attribute__((reqd_work_group_size(1, 1, 1)))
#endif
__kernel void simple_add(__global const int* A, __global const int* B, __global int* C) {
  int offset = get_global_id(0);
  sample3_helloworld();
  sample3_add(&A[offset], &B[offset], &C[offset]);
}
