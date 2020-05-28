#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <syslog.h>
#include <netinet/in.h>
#include "server_conf.h"
#include "medialib.h"
#include "thread_channel.h"
#include "thread_list.h"
#include <proto.h>

struct server_conf_st server_conf = {
    .rcvport = DEFAULT_RECEIVEPORT,
    .multigroup = DEFAULT_MULTIGROUP,
    .media_dir = DEFAULT_MEDIADIR,
    .runmode = RUN_DAEMON,
    .ifname = DEFAULT_IF,
};

int server_sd;
struct sockaddr_in sndaddr;

static void daemon_exit(int s) {}

static int daemonzie(void) {
  pid_t pid;
  int fd;
  pid = fork();
  if (pid < 0) {
    // perror("fork()");
    syslog(LOG_ERR, "fork():%s", sterror(errno));
    return -1;
  }
  if (pid > 0) exit(1);
  if (pid == 0) {
    fd = open("/dev/null", O_RDWR);
    if (fd < 0) {
      // perror("open()");
      syslog(LOG_WARNING, "open():%s", sterror(errno));
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

static int socket_init() {
  server_sd = socket(AF_INET, SOCK_DGRAM, 0);
  if (server_sd < 0) {
    syslog(LOG_ERR, "socket():%s", sterror(errno));
    exit(1);
  }

  struct ip_mreqn mreq;
  inet_pton(AF_INET, server_conf.multigroup, mreq.imr_multiaddr);
  inet_pton(AF_INET, "0.0.0.0", &mreq.imr_address);
  mreq.imr_ifindex = if_nametoindex(server_conf.ifname);
  if (setsockopt(server_sd, IPPROTO_IP, IP_MULTICAST_IF, &mreq, sizeof(mreq)) <
      0) {
    syslog(LOG_ERR, "setsockopt(IP_MULTICAST_IF):%s", sterror(errno));
    exit(1);
  }
  // bind();

  sndaddr.sin_family = AF_INET;
  sndaddr.sin_port = htons(atoi((server_conf.rcvport)));
  inet_pton(AF_INET, "0.0.0.0", sndaddr.sin_addr.s_addr);
  return 0;
}

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

  struct sigaction sa;
  sa.sa_handler = daemon_exit;
  setemptyset(&sa.sa_mask);
  sigaddset(&sa.sa_mask, SIGQUIT);
  sigaddset(&sa.sa_mask, SIGINT);
  sigaddset(&sa.sa_mask, SIGTERM);

  sigaction(SIGTERM, &sa, NULL);
  sigaction(SIGINT, &sa, NULL);
  sigaction(SIGQUIT, &sa, NULL);

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
    // fprintf(stderr, "EINVAL\n");
    syslog(LOG_ERR, "EINVAL : server_conf.runmode");
    exit(1);
  }

  // SOCKET初始化
  socket_init();

  // 获取频道信息
  struct medialib_listentry_st* list;
  int list_size;
  int err;
  err = medialib_getchnlist(&list, &list_size);
  if (err < 0) {
    syslog(LOG_ERR, "");
    exit(1);
  }

  // 创建节目单线程
  err = thread_list_create(list, list_size);
  if (err < 0) {
    syslog();
    exit(1);
  }

  // 创建频道线程
  for (int i = 0; i < list_size; i++) {
    err = thread_channel_create(list + i);
    if (err) {
      fprintf(stderr, "thread_channel_create():%s\n", strerror(err));
      exit(1);
    }
  }

  syslog(LOG_DEBUG, "%d channel thread create", list_size);
  while (1) pause();
  closelog();
  exit(0);
}