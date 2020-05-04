
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
  FILE *fp;

  fp = fopen("temp", "r");
  if (fp == NULL) {
    //  fprintf(stderr, "fopen() failed! errno = %d\n", errno);
    //  perror("fopen()");  输出报错信息
    fprintf(stderr, "fopen(): %s\n", strerror(errno));
    exit(1);
  }
  puts("ok!");
  fclose(fp);
  exit(0);
};
