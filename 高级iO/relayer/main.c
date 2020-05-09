#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

#include "relayer.h"

#define TTY1 "/dev/tty11"
#define TTY2 "/dev/tty12"
#define TTY3 "/dev/tty10"
#define TTY4 "/dev/tty9"

// 中继引擎

int main() {
  int fd1 = -1, fd2 = -1, fd3 = -1,
      fd4 = -1;  // 文件描述符
  int job1, job2;
  fd1 = open(TTY1, O_RDWR);
  if (fd1 < 0) {
    perror("open()");
    exit(1);
  }
  fd2 = open(TTY2, O_RDWR | O_NONBLOCK);
  if (fd2 < 0) {
    perror("open()");
    exit(1);
  }
  fd3 = open(TTY1, O_RDWR);
  if (fd3 < 0) {
    perror("open()");
    exit(1);
  }
  fd4 = open(TTY2, O_RDWR | O_NONBLOCK);
  if (fd4 < 0) {
    perror("open()");
    exit(1);
  }
  job1 = rel_addjob(fd1, fd2);
  if (job1) {
    fprintf(stderr, "rel_addjob()%s\n", strerror(job1));
  }
  job2 = rel_addjob(fd3, fd4);
  if (job2) {
    fprintf(stderr, "rel_addjob()%s\n", strerror(job2));
  }
  while (1) {
    pause();
  }
  close(fd1);
  close(fd2);
  close(fd3);
  close(fd4);
  exit(0);
}