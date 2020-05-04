#include <glob.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// 分割符
#define DELIMS " \t\n"

struct cmd_st {
  // 可持续拓展
  glob_t glob_res;
};

// 打印提示
static void prompt(void) { printf("mysh-0.1$:"); };

// 解析命令
// strtok & strsep 可以用于分割串
static void parse(char* line, struct cmd_st* res) {
  char* tok = NULL;
  int i = 0;  // 第一次不append

  while (1) {
    tok = strsep(&line, DELIMS);
    if (tok == NULL) break;  // 结束或出错就跳出
    if (tok[0] == '\0') {
      // 如果连续几个分割符
      continue;  // 继续接收
    }
    glob(tok, GLOB_NOCHECK | GLOB_APPEND * i, NULL, &(res->glob_res));
    i = 1;
  }
}

int main() {
  char* linebuf = NULL;
  size_t linebuf_size = 0;
  struct cmd_st cmd;
  pid_t pid;
  while (1) {
    prompt();                                           // 打印提示
    if (getline(&linebuf, &linebuf_size, stdin) < 0) {  //获取命令
      break;
    }

    parse(linebuf, &cmd);  // 解析命令
    if (0) {
      // 内部命令 暂时不写
    } else {
      // 外部命令
      pid = fork();
      if (pid < 0) {
        perror("fork()");
        exit(1);
      }
      if (pid == 0) {
        // child
        execvp(cmd.glob_res.gl_pathv[0], cmd.glob_res.gl_pathv);
        perror("exec()");
        exit(1);
      } else {
        // parent
        wait(NULL);
      }
    }
  }
  globfree(&cmd.glob_res);
  exit(0);
}