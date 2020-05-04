#include <stdio.h>
#include <stdlib.h>

// 接受命令参数
int main(int argc, char *argv[]) {
  FILE *fps, *fpd;  // 拷贝的源文件和目标文件
  int ch;           // 接受返回的字符

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

  while (1) {
    ch = fgetc(fps);
    if (ch == EOF) break;  // 如果读失败了
    fputc(ch, fpd);
  }

  // 先关闭依赖的对象，再关闭被依赖的对象
  fclose(fpd);
  fclose(fps);

  return 0;
};