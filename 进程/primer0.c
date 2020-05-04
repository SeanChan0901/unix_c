#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define LEFT 30000000
#define RIGHT 30000200

int main() {
  int i, j, mark = 1;
  for (i = LEFT; i <= RIGHT; i++) {
    mark = 1;
    for (j = 2; j < (i / 2); j++) {
      if (i % j == 0) {
        mark = 0;
      }
    }
    if (mark) printf("%d is a primier\n", i);
  }
  exit(0);
}