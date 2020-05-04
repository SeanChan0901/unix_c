#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define LEFT 30000000
#define RIGHT 30000200

int main() {
  int i, j, mark = 1;
  pid_t pid;
  int st;
  for (i = LEFT; i <= RIGHT; i++) {  // father
    pid = fork();
    if (pid < 0) {
      perror("fork()");
      exit(1);
    }
    if (pid == 0) {
      // child
      mark = 1;
      for (j = 2; j < (i / 2); j++) {
        if (i % j == 0) {
          mark = 0;
        }
      }
      if (mark) printf("%d is a primier\n", i);
      exit(0);  // 子进程记得结束
    }
  }
  for (i = LEFT; i <= RIGHT; i++) {
    // wait(&st);
    wait(NULL);  // 不保存状态，回收子进程资源
  }
  exit(0);
}