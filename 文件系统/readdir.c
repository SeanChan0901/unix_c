#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>

#define PAT "/etc"

// /etc下有多少文件
int main() {
  DIR* dp;  // 打开的文件目录流
  struct dirent* cur;  // 用于获取当前readdir返回的包含文件目录信息的结构体

  dp = opendir(PAT);
  if (dp == NULL) {
    perror("opendir()");
    exit(1);
  }

  while ((cur = readdir(dp)) != NULL) {
    puts(cur->d_name);
  }

  closedir(dp);
  exit(0);
};