
// C-native functions
__attribute__((mppa_native))
int sample7_helloworld(__global const int* magic);

__attribute__((mppa_native))
int sample7_add(__global const int* A, __global const int* B, __global int* C);


// OpenCL-C wrapper kernels
#ifdef ENABLE_WG_SIZE
__attribute__((reqd_work_group_size(1, 1, 1)))
#endif
__kernel void simple_add(__global const int* A, __global const int* B, __global int* C) {
  int offset = get_global_id(0);
  sample7_add(&A[offset], &B[offset], &C[offset]);
}
