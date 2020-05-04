#include <stdio.h>
#include <stdlib.h>

// 环境变量
extern char **environ;

int main() {
  for (int i = 0; environ[i] != NULL; i++) {
    puts(environ[i]);
  }
  exit(0);
}