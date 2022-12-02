#ifndef SAMPLE4_KERNEL
#define SAMPLE4_KERNEL
// Declaration of C-callable function
extern "C" {

  int sample4_helloworld(const int* magic);

  int sample4_add(const int* A, const int* B, int* C);

}
#endif