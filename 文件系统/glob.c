#include <glob.h>
#include <stdio.h>
#include <stdlib.h>

#define PAT "/etc/*.conf"

int main() {
  int err;  // 接受glob返回信息
  glob_t globret;
  err = glob(PAT, 0, NULL, &globret);
  if (err) {
    // err == 0 为成功，其余为失败
    fprintf(stderr, "Error code = %d\n", err);
    exit(1);
  }
  for (int i = 0; i < globret.gl_pathc; i++) {
    puts(globret.gl_pathv[i]);
  }

  globfree(&globret);
  exit(0);
};