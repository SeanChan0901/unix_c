#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
  pid_t pid;
  printf("[%d]Begin\n", getpid());
  fflush(NULL);
  pid = fork();
  if (pid < 0) {
    // 创建进程失败
    perror("fork()");
    exit(1);
  }
  if (pid == 0) {
    // 这个是子进程
    printf("[%d]:Child is working\n", getpid());
  } else {
    // 这个是父进程
    printf("[%d]:Parent is working\n", getpid());
  }
  printf("[%d]End\n", getpid());
  exit(0);
}

// 用命令重定向 > /tmp/out 查看结果，没有fflush和有fflush的区别。