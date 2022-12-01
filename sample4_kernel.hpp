#ifndef SAMPLE3_KERNEL
#define SAMPLE3_KERNEL
// Declaration of C-callable function
extern "C" {

  int sample3_helloworld(const int* magic);

  int sample3_add(const int* A, const int* B, int* C);

}
#endif