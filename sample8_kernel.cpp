#include <stdio.h>
#include <stdint.h>
#include <mppa_cos.h>

#include "sample8_kernel.hpp"
#include "sample8_sizing.h"
#include "papi_util.h"

extern "C" {
void mppa_cos_l2_inval(void);
}

int sample8_helloworld() {
  // mppa_cos_dinval();
  printf("Hello from cluster:%d, pe:%d\n",
         mppa_cos_get_cluster_id(),
         mppa_cos_get_pe_id());

  return 0;
}

typedef int32_t int32xX_t __attribute__((__vector_size__(VECTOR_SIZE*sizeof(int32_t))));


// #define UNROLL
#define IT_MAX 1

int sample8_add(const int* a, const int* b, int* c) {

  int32xX_t *A = (int32xX_t *) a;
  int32xX_t *B = (int32xX_t *) b;
  int32xX_t *C = (int32xX_t *) c;

  //for (int it = 0; it < IT_MAX; it++) {

    // papi_start();

    for (size_t i = 0; i < TABLE_SIZE; i++) {
      C[i] = A[i] + B[i];
    }

    // papi_stop();

    // papi_display(it == (IT_MAX - 1) ? 1 : 0, TABLE_SIZE);

    //}

  // printf("\nptr:%p - miss l2:%ld ", A, mppa_cos_l2_get_miss_counter());
  // sample8_helloworld();
  // printf("%d+%d=%d\n", *A, *B, *C);
  // fflush(stdout);
  return 0;
}
