#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include "client.h"
#include <sys/socket.h>
#include <netinet/in.h>
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
    struct ip_mreq mreq;  // IP多播组结构体
    inet_pton(AF_INET, client_conf.multigroup, &mreq.imr_multiaddr);
    mreq.imr_interface = ;
    setsockopt(sd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void*)&mreq, sizeof(mreq));
    bind();

    pipe();
    fork();
    exit(0);
    return 0;
  }
