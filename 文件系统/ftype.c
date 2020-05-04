#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

static int ftype(const char* fname) {
  struct stat statret;
  if (stat(fname, &statret) < 0) {
    perror("stat()");
    exit(1);
  }
  if (S_ISREG(statret.st_mode))
    return '-';
  else if (S_ISSOCK(statret.st_mode))
    return 's';
  else if (S_ISCHR(statret.st_mode))
    return 'c';
  else if (S_ISLNK(statret.st_mode))
    return 'l';
  else if (S_ISBLK(statret.st_mode))
    return 'b';
  else if (S_ISDIR(statret.st_mode))
    return 'd';
  else if (S_ISFIFO(statret.st_mode))
    return 'p';
  else
    return '?';  // 如果不是以上7种，返回一个问号
};

int main(int argc, char* argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Usage : %s <filename>\n", argv[0]);
    exit(1);
  }
  printf("%c\n", ftype(argv[1]));
  exit(0);
};