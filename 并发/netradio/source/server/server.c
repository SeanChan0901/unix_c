#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "server_conf.h"
#include <proto.h>

struct server_conf_st server_conf = {
    .rcvport = DEFAULT_RECEIVEPORT,
    .multigroup = DEFAULT_MULTIGROUP,
    .media_dir = DEFAULT_MEDIADIR,
    .runmode = RUN_DAEMON,
    .ifname = DEFAULT_IF,
};

static int daemonzie(void) {
  pid_t pid;
  int fd;
  pid = fork();
  if (pid < 0) {
    // perror("fork()");
    syslog(LOG_ERR);
    return -1;
  }
  if (pid > 0) exit(1);
  if (pid == 0) {
    fd = open("/dev/null", O_RDWR);
    if (fd < 0) {
      // perror("open()");
      syslog(LOG_ERR);
      return -2;
    }
    dup2(fd, 0);
    dup2(fd, 1);
    dup2(fd, 2);
    if (fd > 2) close(fd);
  }
  setsid();
  chdir("/");  // 改变工作路径
  umask(0);    // 关闭umask
  return 0;
};

/*
 * -M 指定多播组
 * -P 指定接收端口
 * -F 指定守护进程前台运行
 * -H 帮助信息
 * -D 指定媒体库位置
 * -I 指定网络设备
 */

int main(int argc, char* argv[]) {
  // 命令行分析
  int c;
  openlog("netradio", LOG_PID | LOG_PERROR, LOG_DAEMON);
  while (1) {
    c = getopt(argc, argv, "M:P:FD:I:H");
    if (c < 0) break;
    switch (c) {
      case 'M':
        server_conf.multigroup = optarg;
        break;
      case 'P':
        server_conf.rcvport = optarg;
        break;
      case 'F':
        server_conf.runmode = RUN_FORGROUND;
        break;
      case 'D':
        server_conf.media_dir = optarg;
        break;
      case 'I':
        server_conf.ifname = optarg;
        break;
      case 'H':
        printf(
            "usage: %s [--help | -H] [--multiport | -M <multigroup address>] "
            "[--dir | D <media dir>] [--port | P <receive port>] [--forground "
            "| -F <runmode = forground>] [-I | --interface <interface dev>]\n",
            argv[0]);
        break;
      default:
        abort();
        break;
    }
  }

  // 守护进程的实现
  if (server_conf.runmode == RUN_DAEMON) {
    if (daemonzie() != 0) {
      exit(1);
    }
  } else if (server_conf.runmode == RUN_FORGROUND) {
  } else {
    fprintf(stderr, "EINVAL\n");
    exit(1);
  }
  // SOCKET初始化
  // 获取频道信息
  // 创建节目单线程
  // 创建频道线程
  while (1) pause();
  closelog();
  exit(0);
}