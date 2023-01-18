#ifndef SAMPLE8_KERNEL
#define SAMPLE8_KERNEL


// Declaration of C-callable function
extern "C" {

  int sample8_helloworld(const int* magic);

  int sample8_add(const int* A, const int* B, int* C);

}
#endif