#include <stdio.h>
#include <stdlib.h>
#include "proto.h"
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>

#define IPSTRSIZE 40

int main() {
  int sd;
  struct sockaddr_in laddr;  // 接收端地址(本地)
  struct sockaddr_in raddr;  // 对端（远端）地址
  socklen_t raddr_len;       // 对端地址的长度
  char ipstr[IPSTRSIZE];
  struct msg_st rbuf;
  sd = socket(AF_INET, SOCK_DGRAM, 0 /*IPPRITO_UDP*/);
  if (sd < 0) {
    perror("socket");
    exit(1);
  }
  laddr.sin_family = AF_INET;             // 协议族
  laddr.sin_port = htons(atoi(RCVPORT));  // 哪个端口是给我用的,
  printf("%d\n", laddr.sin_port);
  // 然后如果是网络传输,本地信息到远端用htons
  inet_pton(AF_INET, "0.0.0.0", &laddr.sin_addr.s_addr);  // 你的ip地址
  if (bind(sd, (void *)&laddr, sizeof(laddr)) < 0) {
    perror("bind()");
    exit(1);
  }
  // 非常重要,一定要填好对端地址长度，告诉它回填的地址有多大，不然第一次传输会地址不对
  raddr_len = sizeof(raddr);
  while (1) {
    recvfrom(sd, &rbuf, sizeof(rbuf), 0, (void *)&raddr, &raddr_len);
    inet_ntop(AF_INET, &raddr.sin_addr, ipstr, IPSTRSIZE);
    printf("---MESSAGE FROM %s:%d---\n", ipstr, ntohs(raddr.sin_port));
    printf("NAME = %s\n", rbuf.name);  // 单字节传输不用转换
    printf("MATH = %d\n", ntohl(rbuf.math));
    printf("CHINESE = %d\n", ntohl(rbuf.chinese));
  }
  close(sd);
  exit(0);
}