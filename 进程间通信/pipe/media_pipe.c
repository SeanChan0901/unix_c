#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

// 播放一首歌

// 伪代码

int main() {
  int pd[2];
  char buf[BUFSIZ];
  pid_t pid;
  if (pipe(pd) < 0) {
    perror("pipe()");
    exit(1);
  }
  pid = fork();
  if (pid < 0) {
    perror("fork()");
    exit(1);
  }
  if (pid == 0) {
    // child
    close(pd[1]);
    int fd = open("/dev/null", O_RDWR);
    if (fd < 0) {
      perror("open()");
      exit(1);
    }
    dup2(fd, 1);  // 重定向到标准输出
    dup2(fd, 2);  // 重定向到标砖出错
    execl("/usr/local/bin/mpg123", "mpg123", "-", NULL);
    perror("execl()");
    exit(1);
  } else {
    // parent
    close(pd[0]);
    // 父进程从网上收数据往管道中写
    close(pd[1]);
    wait(NULL);
    exit(0);
  }
}