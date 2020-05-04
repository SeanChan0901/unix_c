#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define LEFT 30000000
#define RIGHT 30000200

// 可以发现真的快了好多啊！！！！！！！什么情况，这就是四核八线程的力量吗？

int main() {
  int i, j, mark = 1;
  pid_t pid;
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
      // sleep(1000);
      exit(0);  // 子进程记得结束
    }
  }
  // sleep(1000);
  exit(0);
}