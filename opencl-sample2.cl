#ifdef ENABLE_WG_SIZE
__attribute__((reqd_work_group_size(1, 1, 1)))
#endif

__kernel void simple_add(__global const int* A, __global const int* B, __global int* C) {
  int offset = get_global_id(0);
  //printf("global_id:%d, local_id:%d \n", offset, get_local_id(0));
  C[offset]= A[offset] + B[offset];
 }
