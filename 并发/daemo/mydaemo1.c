#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>
#include <sys/stat.h>
#include <syslog.h>
#include <unistd.h>

#define FNAME "/tmp/out"

/*
 * 守护进程
 *
 *
 *
 */


// 使其成为守护进程
static int deamonize() {
  pid_t pid;
  int fd;  // 文件描述符
  pid = fork();
  if (pid < 0) {
    // fork失败
    perror("fork()");
    return -1;
  }
  if (pid > 0) {
    // parent
    exit(0);  // 正常结束
  }
  // child
  fd = open("/dev/null", O_RDWR);
  if (fd < 0) {
    perror("open()");
    return -1;  // 打开文件失败
  }
  dup2(fd, 0);  // 脱离控制终端 （重定向到stdin的位置）
  dup2(fd, 1);  // 脱离控制终端 （重定向到stdout的位置）
  dup2(fd, 2);  // 脱离控制终端 （重定向到stderr的位置）
  if (fd > 2) close(fd);  // 重定向失败，就关闭这个文件
  setsid();               // 做成守护进程
  chdir("/");             // 把当前的工作路径设置为根
  umask(0);               // 如果不再生成文件，那么把umask关了
  return 0;
}

int main() {
  FILE *fp;
  int i = 0;
  // 守护进程化
  if (deamonize()) {
    exit(1);
  }
  // 将文件描述符重定向
  fp = fopen(FNAME, "w");
  if (fp == NULL) {
    perror("fopen()");
    exit(1);
  }
  while (1) {
    // 每隔一秒钟往文件输入一个数字˝
    fprintf(fp, "%d\n", i);
    fflush(fp);
    i++;
    sleep(1);
  }
  exit(0);
}
