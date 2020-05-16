#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

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
    int len;
    close(pd[1]);
    len = read(pd[0], buf, BUFSIZ);
    write(1, buf, len);
    close(pd[0]);
  } else {
    // parent
    close(pd[0]);
    write(pd[1], "Hello!", 6);
    close(pd[1]);
    wait(NULL);
    exit(0);
  }
}