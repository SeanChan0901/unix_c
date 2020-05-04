#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// 白痴加密程序
int main(int argc, char* argv[]) {
  char* input_pass;
  if (argc < 2) {
    fprintf(stderr, "Usage : %s <password>\n", argv[0]);
    exit(1);
  }
  input_pass = getpass("Password:");
  //  Mac的库不是这样实现的。库里面没有getspnam() 很可惜，实现不了。
  exit(0);
}