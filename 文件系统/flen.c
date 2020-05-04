#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

static off_t flen(const char *fanme) {
  struct stat statres;  // 文件属性结构体
  if (stat(fanme, &statres) < 0) {
    // 判断是否出错
    perror("stat()");
    exit(1);
  }
  return statres.st_size;
};

int main(int argc, char *argv[]) {
  if (argc < 2) {
    // 判断命令参数
    fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
    exit(1);
  }
  printf("%lld\n", flen(argv[1]));
  exit(0);
};
