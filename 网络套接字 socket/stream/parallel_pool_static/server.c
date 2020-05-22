#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "proto.h"

// 并发版本，把accept和server——job分开，这样即便server_job需要耗掉时间也不会影响
// 继续accept工作
// 引入静态进程池

#define IPSTRSIZE 40
#define BUFSIZE 1024
#define PROCNUM 4  // 子进程资源

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
static void server_loop(int sd);

int main() {
  int sd;
  pid_t pid;
  struct sockaddr_in laddr;  // 本端地址
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

  for (int i = 0; i < PROCNUM; i++) {
    pid = fork();
    if (pid < 0) {
      perror("fork()");
      exit(1);
    }
    if (pid == 0) {
      // child
      server_loop(sd);  // 循环执行任务
      exit(0);
    }
  }
  for (int i = 0; i < PROCNUM; i++) wait(NULL);
  // 关闭连接
  close(sd);
  exit(0);
}

static void server_loop(int sd) {
  struct sockaddr_in raddr;
  socklen_t raddr_len;
  int newsd;
  char ipstr[IPSTRSIZE];
  // 接受连接
  raddr_len = sizeof(raddr);
  while (1) {
    newsd = accept(sd, (void*)&raddr, &raddr_len);
    if (newsd < 0) {
      perror("accept()");
      exit(1);
    }
    inet_ntop(AF_INET, &raddr.sin_addr, ipstr, IPSTRSIZE);
    printf("[%d]Client:%s:%d\n", getpid(), ipstr, ntohs(raddr.sin_port));
    server_job(newsd);
    close(newsd);  // 父进程不用
    // parent
  }
}
