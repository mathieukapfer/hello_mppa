#include <stdio.h>
#include <mppa_cos.h>

#include "sample7_kernel.hpp"

extern "C" {
void mppa_cos_l2_inval(void);
}


int sample7_helloworld(const int* magic) {
  for (int i=0; i<5; i++) {
    // mppa_cos_dinval();
    printf("Hello from cluster:%d, pe:%d, magic:%d\n",
           mppa_cos_get_cluster_id(),
           mppa_cos_get_pe_id(),
           *magic);
    sleep(1);
  }

  return 0;
}

int sample7_add(const int* A, const int* B, int* C) {
  *C = *A + *B;
  // printf("%d+%d=%d\n", *A, *B, *C);
  // fflush(stdout);
  return 0;
}
