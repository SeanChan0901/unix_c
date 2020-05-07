#include <fcntl.h>
#include <signal.h>
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

static FILE *fp;

// 信号处理函数
static void daemon_exit(int s) {
  fclose(fp);
  exit(0);
}

// 使其成为守护进程
static int deamonize() {
  pid_t pid;
  int fd;  // 文件描述符
  pid = fork();
  if (pid < 0) {
    // fork失败
    return -1;
  }
  if (pid == 0) {
    // child
    fd = open("/dev/null", O_RDWR);
    if (fd < 0) {
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
  // parent
  exit(0);  // 正常结束
}

int main() {
  int i = 0;
  struct sigaction sa;

  sa.__sigaction_u.__sa_handler = daemon_exit;
  sigemptyset(&sa.sa_mask);  // 结构体的信号屏蔽集
  sigaddset(&sa.sa_mask, SIGQUIT);
  sigaddset(&sa.sa_mask, SIGTERM);
  sa.sa_flags = 0;
  sigaction(SIGINT, &sa, NULL);

  sigemptyset(&sa.sa_mask);  // 结构体的信号屏蔽集
  sigaddset(&sa.sa_mask, SIGINT);
  sigaddset(&sa.sa_mask, SIGTERM);
  sigaction(SIGQUIT, &sa, NULL);

  sigemptyset(&sa.sa_mask);  // 结构体的信号屏蔽集
  sigaddset(&sa.sa_mask, SIGQUIT);
  sigaddset(&sa.sa_mask, SIGINT);
  sigaction(SIGTERM, &sa, NULL);

  /*
   *  简便写法
   * sa.__sigaction_u.__sa_handler = daemon_exit;
   * sigemptyset(&sa.sa_mask);  // 结构体的信号屏蔽集
   * sigaddset(&sa.sa_mask, SIGQUIT);
   * sigaddset(&sa.sa_mask, SIGTERM);
   * sigaddset(&sa.sa_mask, SIGNIT);
   * sa.sa_flags = 0;
   * sigaction(SIGINT, &sa, NULL);
   * sigaction(SIGTERM, &sa, NULL);
   * sigaction(SIGQUIT,&sa,NULL);
   */

  openlog("mydaemon", LOG_PID, LOG_DAEMON);
  if (deamonize()) {  // 守护进程化
    syslog(LOG_ERR, "daemonize() failed!");
    exit(1);
  } else {
    syslog(LOG_INFO, "daemonize() sucesseded!");
  }
  // 将文件描述符重定向

  fp = fopen(FNAME, "w");
  if (fp == NULL) {
    syslog(LOG_ERR, "fopen():%s", strerror(errno));
    exit(1);
  }
  syslog(LOG_INFO, "%s was open.", FNAME);
  while (1) {
    // 每隔一秒钟往文件输入一个数字˝
    fprintf(fp, "%d\n", i);
    fflush(fp);
    syslog(LOG_DEBUG, "%d is printed.", i);
    i++;
    sleep(1);
  }
  closelog();
  exit(0);
}
