#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/*
 * 使用 ./mydu uid + cmd 来实行权限的下放
 *
 */

int main(int argc, char *argv[]) {
  pid_t pid;

  if (argc < 3) {
    fprintf(stderr, "Usage: %s <cmd> <cmd option> <cmd optinon>...\n", argv[0]);
    exit(1);
  }
  pid = fork();
  if (pid < 0) {
    // 出错
    perror("fork()");
    exit(1);
  }
  if (pid == 0) {
    // child
    // 将uid转换成一个整形数,（很明显这是不可能成功的因为不是谁都能把自己设置成root的） 
    setuid(atoi(argv[1]));
    execvp(argv[2], argv + 2);
    perror("execvp()");
    exit(1);
  }
  // father
  wait(NULL);
  exit(0);
}