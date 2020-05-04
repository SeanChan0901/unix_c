#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
  FILE *fp;
  char *linebuf = NULL;  // 这行的字符串
  size_t linesize = 0; // 行长度

  // 判断命令参数
  if (argc < 2) {
    fprintf(stderr, "Usage:%s - <filename>\n", argv[0]);
    exit(1);
  }

  // 打开一个文件
  fp = fopen(argv[1], "r");
  if (fp == NULL) {
    // 打开文件失败，设置ERRNO
    perror("fopen()");
    exit(1);
  }

  while (1) {
    // getline会给你回填一些地址
    // 1）这一行字符串的char指针的地址，也就是char**
    // 2) 这一行字符串的长度（size_t）的地址
    if (getline(&linebuf, &linesize, fp) < 0) break;
    printf("%ld\n", strlen(linebuf));
  }

  fclose(fp);
  exit(0);
}