
#include <sched.h>

#include <stdio.h>
#include <omp.h>
#include <mppa_cos.h>

int main(int argc, char **argv)
{

  int nb_threads=4;
  if (argc>1)
    nb_threads = atoi(argv[1]);

  omp_set_num_threads(nb_threads);

  // example of parallel (execute this line on all cpu)
#pragma omp parallel
  printf("hello \n");

  // example of parallel inside loop (dispatch index on all cpu
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
