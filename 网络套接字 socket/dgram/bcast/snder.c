#include <stdlib.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>

#include "proto.h"

// 广播

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Usage :%s <name>", argv[0]);
    exit(1);
  }
  if (strlen(argv[1]) > NAMEMAX) {
    fprintf(stderr, "%s : name is too long\n", argv[1]);
    exit(1);
  }
  int sd;
  struct msg_st *sbuf_ptr;
  struct sockaddr_in raddr;

  // 数据块占的字节数
  int size = sizeof(struct msg_st) - 1 + strlen(argv[1]);
  sbuf_ptr = malloc(size);
  if (sbuf_ptr == NULL) {
    perror("malloc()");
    exit(1);
  }

  sd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sd < 0) {
    perror("socker()");
    exit(1);
  }

  // 开启广播
  // 属性设置
  int val = 1;  // 为真
  if (setsockopt(sd, SOL_SOCKET, SO_BROADCAST, &val, sizeof(val)) < 0) {
    perror("setsockopt()");
    exit(1);
  }
  // bind()

  // sbuf
  strcpy(sbuf_ptr->name, argv[1]);
  sbuf_ptr->math = htonl(rand() % 100);
  sbuf_ptr->chinese = htonl(rand() % 100);

  // raddr
  raddr.sin_family = AF_INET;
  raddr.sin_port = htons(atoi(RCVPORT));
  inet_pton(AF_INET, "255.255.255.255", &raddr.sin_addr);  // 广播
  if (sendto(sd, sbuf_ptr, size, 0, (void *)&raddr, sizeof(raddr)) < 0) {
    perror("sendto()");
    exit(1);
  }
  puts("ok!\n");
  close(sd);
  exit(0);
}