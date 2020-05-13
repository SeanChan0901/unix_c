#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
// 找文件内有多少a

int main(int argc, char *argv[]) {
  int fd;
  struct stat stat_buf;
  char *my_loc;
  int64_t count = 0;
  if (argc < 2) {
    fprintf(stderr, "Usage: argv[0] <filename>");
    exit(1);
  }
  fd = open(argv[1], O_RDONLY);
  if (fd < 0) {
    perror("open()");
    exit(1);
  }
  if (stat(argv[1], &stat_buf) < 0) {
    perror("stat()");
    exit(1);
  }
  my_loc = mmap(my_loc, stat_buf.st_size, PROT_READ, MAP_SHARED, fd, 0);
  if (my_loc == MAP_FAILED) {
    perror("mmap()");
    exit(1);
  }
  close(fd);  // 已经映射好了，其实可以关闭它了。
  for (int i = 0; i < stat_buf.st_size; i++) {
    if (my_loc[i] == 'a') count++;
  }
  printf("%lld\n", count);
  munmap(my_loc, stat_buf.st_size);
}