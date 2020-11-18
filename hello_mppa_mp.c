#include <sched.h>

#include <stdio.h>
#include <omp.h>
#include <mppa_cos.h>

int main(void)
{

  omp_set_num_threads(8);

  // example of parallel (execute this line on all cpu)
#pragma omp parallel
  printf("hello \n");

  // example of parallel inside loop (dispatch index on all cpu
#pragma omp parallel for
  for (int index=0; index<10; index++)
    {
      printf("Thread %3d is running on CPU %3d - (index:%d)\n",
             mppa_cos_get_cluster_id(),
             mppa_cos_get_pe_id(),
             index);
    }

  return 0;
}
