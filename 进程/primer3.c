#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define LEFT 30000000
#define RIGHT 30000200
#define N 3

// 交叉分配法
int main() {
  int i, j, mark, n = 1;
  pid_t pid;
  for (n = 0; n < N; n++) {
    pid = fork();
    if (pid < 0) {
      perror("fork()");
      for (int z = 0; z < n - 1; z++)
        wait(NULL);  // 如果第n个进程创建失败，将之前的n-1个进程回收
      exit(1);
    }
    if (pid == 0) {
      // child
      for (i = LEFT + n; i <= RIGHT; i += N) {
        mark = 1;
        for (j = 2; j < (i / 2); j++) {
          if (i % j == 0) {
            mark = 0;  //  不是primer
            break;
          }
        }
        if (mark) printf("[%d]:%d is a primier\n", n, i);
      }
      exit(0);  // 子进程结束
    }
  }
  for (n = 0; n < N; n++) {
    // 回收子进程
    wait(NULL);
  }
  exit(0);  // 父进程结束
}