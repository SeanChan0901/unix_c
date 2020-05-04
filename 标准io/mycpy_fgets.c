#include <stdio.h>
#include <stdlib.h>

#define BUFFERSIZE 1024

// 接受命令参数
int main(int argc, char *argv[]) {
  FILE *fps, *fpd;       // 拷贝的源文件和目标文件
  char buf[BUFFERSIZE];  // 用于接受读取的字符串

  // 命令错误
  if (argc < 3) {
    fprintf(stderr, "Usage:%s <src_file> <dest_file>\n", argv[0]);
    exit(1);
  }
  fps = fopen(argv[1], "r");
  if (fps == NULL) {
    // 打开失败
    perror("fopen()");
    exit(1);
  }

  fpd = fopen(argv[2], "w");
  if (fpd == NULL) {
    // 打开失败
    perror("fopen()");
    exit(1);
  }

  while (fgets(buf, BUFFERSIZE, fps) != NULL) {
    // 如果可以读取的话那么就写到目标文件中去
    fputs(buf, fpd);
  }

  // 先关闭依赖的对象，再关闭被依赖的对象
  fclose(fpd);
  fclose(fps);

  return 0;
};