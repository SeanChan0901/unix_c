#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
  putchar('a');
  write(1, "b", 1);

  putchar('a');
  write(1, "b", 1);

  putchar('a');
  write(1, "b", 1);

  // 实际上打印出来的是，bbbaaa
  exit(0);
};