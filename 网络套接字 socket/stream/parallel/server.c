#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <time.h>
#include "proto.h"

// 并发版本，把accept和server——job分开，这样即便server_job需要耗掉时间也不会影响
// 继续accept工作

#define IPSTRSIZE 40
#define BUFSIZE 1024

// 发送一个大时间戳
static void server_job(int sd) {
  // time函数返回一个time_t类型的整形数，不知道多长的情况下要注意
  char buf[BUFSIZE];
  long long len;
  len = sprintf(buf, FMT_STAMP, (long long)time(NULL));
  if (send(sd, buf, len, 0) < 0) {
    perror("send()");
    exit(1);
  }
}

int main() {
  int sd;
  int newsd;
  pid_t pid;
  char ipstr[IPSTRSIZE];
  socklen_t raddr_len;
  struct sockaddr_in laddr;  // 本端地址
  struct sockaddr_in raddr;  // 对端地址
  sd = socket(AF_INET, SOCK_STREAM, 0 /*IPPROTO_TCP,IPPROTO_SCTP*/);
  if (sd < 0) {
    perror("socket()");
    exit(1);
  }
  int val = 1;
  if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)) < 0) {
    perror("setsockopt()");
    exit(1);
  }

  // 本端地址的设置
  laddr.sin_family = AF_INET;
  laddr.sin_port = htons(atoi(SERVERPORT));
  inet_pton(AF_INET, "0.0.0.0", &laddr.sin_addr);
  if (bind(sd, (void*)&laddr, sizeof(laddr)) < 0) {
    perror("bind()");
    exit(1);
  }

  // 监听
  if (listen(sd, 200) < 0) {
    perror("listen()");
    exit(1);
  }

  // 接受连接
  raddr_len = sizeof(raddr);
  while (1) {
    newsd = accept(sd, (void*)&raddr, &raddr_len);
    if (newsd < 0) {
      perror("accept()");
      exit(1);
    }
    pid = fork();
    if (pid < 0) {
      perror("fork()");
      exit(1);
    }
    if (pid == 0) {
      // child
      close(sd);  // 子进程不用，记得把不用的资源关掉
      inet_ntop(AF_INET, &raddr.sin_addr, ipstr, IPSTRSIZE);
      printf("Client:%s:%d\n", ipstr, ntohs(raddr.sin_port));
      // 接受消息
      server_job(newsd);
      close(newsd);
      exit(0);
    }
    close(newsd);  // 父进程不用
    // parent
  }
  // 关闭连接
  close(sd);
  exit(0);
}