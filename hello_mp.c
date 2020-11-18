//
// open mp *host* example
//
// compile with:
//
//   make hello_mp

#include <sched.h>
#include <stdio.h>
#include <omp.h>

int main(void)
{
  // example of parallel (execute this line on all cpu)
#pragma omp parallel
  printf("hello \n");


  // example of parallel inside loop (dispatch index on all cpu
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
