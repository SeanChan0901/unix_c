#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <time.h>
#include "proto.h"

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Usage : %s <ip-address>", argv[0]);
    exit(1);
  }
  int sd;
  struct sockaddr_in raddr;
  long long stamp;
  sd = socket(AF_INET, SOCK_STREAM, 0);
  if (sd < 0) {
    perror("socket()");
    exit(1);
  }
  // bind();
  raddr.sin_family = AF_INET;
  raddr.sin_port = htons(atoi(SERVERPORT));  // 网络序和主机序的转换
  inet_pton(AF_INET, argv[1], &raddr.sin_addr);  // 点分式转大整数

  if (connect(sd, (void *)&raddr, sizeof(raddr)) < 0) {  // 请求连接
    perror("connect()");
    exit(1);
  }
  // recv();
  // close();

  FILE *fp;
  // 一切皆文件，可以讲文件描述符转化为流就可以使用标准IO的所有操纵！！
  fp = fdopen(sd, "r+");
  if (fp == NULL) {
    perror("fopen()");
    exit(1);
  }
  if (fscanf(fp, FMT_STAMP, &stamp) < 1) {
    fprintf(stderr, "Bad format!\n");
    exit(1);
  } else {
    fprintf(stdout, "stamp = %lld\n", stamp);
  }
  fclose(fp);
  exit(0);
}