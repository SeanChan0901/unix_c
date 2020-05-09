#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
#pragma omp parallel
  // omp 标识
  {
    puts("Hello");
    puts("World");
  }
  exit(0);
}