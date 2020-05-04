#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char* argv[]) {
  if (argc < 2) {
    // 判断命令参数
    fprintf(stderr, "Usage : %s\n", argv[1]);
    exit(1);
  }

  int fd;  // 文件描述符

  fd = open(argv[1], O_WRONLY | O_CREAT | O_TRUNC, 0600);
  if (fd < 0) {
    perror("open()");
    exit(1);
  }
  if (lseek(fd, 5LL * 1024LL * 1024LL * 1024LL - 1LL, SEEK_SET) < 0) {
    // 创建一个5G大小的空洞文件,如果创建失败则关闭
    perror("lseek()");
    close(fd);
    exit(1);
  }
  write(fd,"",1);  // 为了产生空间，写入一个空字符
  close(fd);
  exit(0);
};