#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include "client.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <errno.h>
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

static ssize_t writen(int fd, const char* buf, size_t buf_len) {
  int pos = 0;
  int ret;
  while (buf_len > 0) {
    ret = write(fd, buf + pos, buf_len);
    if (ret < 0) {
      if (errno == EINTR) continue;
      perror("write()");
      return -1;
    }
    buf_len -= ret;
    pos += ret;
  }
  return buf_len;
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
      close(sd);
      close(pd[1]);
      dup2(pd[0], 0);  // 重定向到标准输入
      if (pd[0] > 0) {
        close(pd[0]);  // 如果他本身不是标准输入，把它关了
      }
      execl("/bin/sh", "sh", "-c", client_conf.player_cmd,
            NULL);  // 直接用shell来命令
      perror("execl()");
      exit(1);
    }

    // parent 从网络上收包，发送给子进程

    // 收节目单
    struct msg_list_st* msg_list;
    struct sockaddr_in server_addr;
    socklen_t ser_len;
    int chosenchnid;
    int len;
    msg_list = malloc(MSG_LIST_MAX);
    if (msg_list == NULL) {
      perror("malloc()");
      exit(1);
    }
    while (1) {
      len = recvfrom(sd, msg_list, MSG_LIST_MAX, 0, (void*)&server_addr,
                     &ser_len);
      if (len < sizeof(struct msg_list_st)) {
        // 包过小
        fprintf(stderr, "message is too small.\n");
        continue;
      }
      if (msg_list->chnid != LISTCHNID) {
        // id不正确
        fprintf(stderr, "chnid is not match");
      }
      break;
    }
    // 打印节目单并且选择频道
    struct msg_listentry_st* pos;
    for (pos = msg_list->entry; (char*)pos < ((char*)msg_list + len);
         pos = (void*)(((char*)pos) +
                       ntohs(pos->len))) {  // 单字节往后移动强转为char*
      printf("channel %d: %s\n", pos->chnid, pos->desc);
    }
    free(msg_list);  // 节目单信息已经输出完毕，可以释放了
    while (1) {
      int ret = 0;
      ret = scanf("%d", &chosenchnid);  // 千万要慎重在循环体中使用scanf
      if (ret != 1) exit(1);            // 如果输入错误直接退出
    }

    // 收频道包，发送给子进程
    struct msg_channel_st* msg_channel;
    msg_channel = malloc(MSG_CHANNEL_MAX);
    if (msg_channel == NULL) {
      perror("malloc()");
      exit(1);
    }

    struct sockaddr_in raddr;
    socklen_t raddr_len;
    while (1) {
      len = recvfrom(sd, msg_channel, MSG_CHANNEL_MAX, 0, (void*)&raddr,
                     &raddr_len);
      if (raddr.sin_addr.s_addr != server_addr.sin_addr.s_addr ||
          raddr.sin_port != server_addr.sin_port) {
        // 如果地址都不对（与节目单地址不对）
        fprintf(stderr, "Ignore : address not match.\n");
        continue;
      }
      if (len < sizeof(struct msg_channel_st)) {
        // 包长度过小
        fprintf(stderr, "Ignore : message too small.\n");
        continue;
      }
      if (msg_channel->chnid == chosenchnid) {
        // 判断是否接收到正确的频道
        fprintf(stdout, "accept message :%d received.\n", msg_channel->chnid);
        // 写到管道当中去
        if (writen(pd[1], msg_channel->data, len - sizeof(chnid_t)) < 0) {
          exit(1);
        }
      }
    }
    free(msg_channel);
    close(sd);
    exit(0);
  }
}