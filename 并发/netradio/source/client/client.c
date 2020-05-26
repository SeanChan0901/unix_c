#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include "client.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <net/if.h>
#include <proto.h>
/*
 * -M --multigroup指定多播组
 * -P --port指定接收端口
 * -p --player指定播放器
 * -H --help显示帮助
 *
 */

struct client_conf_st client_conf = {
    .rcvport = DEFAULT_RECEIVEPORT,
    .multigroup = DEFAULT_MULTIGROUP,
    .player_cmd = DEFAULT_PLAYERCMD,
};

int main(int argc, char* argv[]) {
  int pd[2];  // 管道
  pid_t pid;
  int index = 0;
  int c;
  int sd;
  struct option argarray[] = {
      {"port", 1, NULL, 'P'}, {"multigroup", 1, NULL, 'M'},
      {"help", 1, NULL, 'H'}, {"player", 1, NULL, 'p'},
      {NULL, 0, NULL, 0},
  };
  while (1) {
    c = getopt_long(argc, argv, "M:P:p:H", argarray, &index);
    if (c == -1) break;
    switch (c) {
      case 'p':
        client_conf.rcvport = optarg;
        break;
      case 'M':
        client_conf.multigroup = optarg;
        break;
      case 'P':
        client_conf.player_cmd = optarg;
        break;
      case 'H':
        printf(
            "usage: %s [--help | -H] [--multiport | -M <multigroup address>] "
            "[--player | p <mpgplayer>] \n [--port | P <receive port>]",
            argv[0]);
        exit(1);
        break;
      default:
        abort();  // 严重错误，直接结束进程并且生成coredown文件
        break;
    }
    sd = socket(AF_INET6, SOCK_DGRAM, 0);
    if (sd < 0) {
      perror("socket()");
      exit(1);
    }

    struct ip_mreqn mreq;  // IP多播组结构体
    inet_pton(AF_INET, client_conf.multigroup, &mreq.imr_multiaddr);
    inet_pton(AF_INET, "0.0.0.0", &mreq.imr_address);
    mreq.imr_ifindex = if_nametoindex("eth0");  // 设备（网卡）索引号
    if (setsockopt(sd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void*)&mreq,
                   sizeof(mreq)) < 0) {
      perror("setsockopt()");
      exit(1);
    }
    int val = 1;
    if (setsockopt(sd, IPPROTO_IP, IP_MULTICAST_LOOP, &val, sizeof(val)) < 0) {
      perror("setcockopt()");
      exit(1);
    }

    struct sockaddr_in laddr;  // 本端地址
    laddr.sin_family = AF_INET;
    laddr.sin_port = htons(atoi(client_conf.rcvport));
    inet_pton(AF_INET, "0.0.0.0", &laddr.sin_addr.s_addr);
    if (bind(sd, (void*)&laddr, sizeof(laddr)) < 0) {
      perror("bind()");
      exit(1);
    }

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
      // child 调用解码器
    }
    // parent 从网络上收包，发送给子进程
    // 收节目单
    // 选择频道
    // 收频道包，发送给子进程

    exit(0);
    return 0;
  }
