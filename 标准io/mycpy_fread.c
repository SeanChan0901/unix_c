#include <stdio.h>
#include <stdlib.h>

#define BUFFERNUM 1024

// 接受命令参数
int main(int argc, char *argv[]) {
  FILE *fps, *fpd;      // 拷贝的源文件和目标文件
  char buf[BUFFERNUM];  // 用于接受读取的字符串
  int n;                // 成功读到的size_t
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

  while ((n = fread(buf, 1, BUFFERNUM, fps)) > 0) {
    // 成功读取的对象个数大于0
    fwrite(buf, 1, n, fpd);
  }

  // 先关闭依赖的对象，再关闭被依赖的对象
  fclose(fpd);
  fclose(fps);

  return 0;
};