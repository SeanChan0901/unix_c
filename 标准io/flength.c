#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
  
  FILE *fp;
  int count = 0;

  //  参数错误
  if (argc < 2) {
    fprintf(stderr, "usage : %s: <file_name>\n", argv[0]);
    exit(1);
  }

  // 打开文件
  fp = fopen(argv[1], "r");
  if (fp == NULL) {
    // 打开失败
    perror("fopen()");
    exit(0);
  }
  // 寻找末尾位置
  fseek(fp, 0, SEEK_END);
  
  // 打印出来
  printf("%ld\n", ftell(fp));

  /*
  // 用fgetc来获取文件长度
  // 读取文件的字符串长度
  while (fgetc(fp) != EOF) {
    count++;
  }

  //输出文件字符个数
  printf("count = %d\n", count);
  */

  fclose(fp);
  exit(0);
};