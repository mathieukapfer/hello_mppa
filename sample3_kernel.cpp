#include <stdio.h>
#include <mppa_cos.h>

#include "sample3_kernel.hpp"


int sample3_helloworld() {
  //printf("hello");
  printf("Hello from cluster:%d, pe:%d\n",
         mppa_cos_get_cluster_id(),
         mppa_cos_get_pe_id());
  return 0;
}

int sample3_add(const int* A, const int* B, int* C){
  *C = *A + *B;
  return 0;
}
