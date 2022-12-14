#ifndef SAMPLE7_KERNEL
#define SAMPLE7_KERNEL
// Declaration of C-callable function
extern "C" {

  int sample7_helloworld(const int* magic);

  int sample7_add(const int* A, const int* B, int* C);

}
#endif