#include <unistd.h>
#include <vector>
#include <iostream>
#include <omp.h>

int main()
{
  int i=0;

  omp_set_num_threads(2);

#pragma omp parallel
  printf("hello %d\n", ++i);
  std::vector<int> v = {1,2,3,4};

  // use int in loop
#pragma omp parallel for
  for (int it=0; it < 4; it++) {
    printf("%i \n", v.at(it));
    sleep(1);
  }

  // keep iterator in loop !
#pragma omp parallel
  for (auto it = v.begin(); it != v.end(); ++it) {
#pragma omp single nowait
    {
      printf("**it:%p - %i\n", &it, *it);
      sleep(1);
    }
  }

  return 0;
}
