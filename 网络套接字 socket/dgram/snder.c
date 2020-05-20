#include <stdlib.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>

#include "proto.h"

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Usage :%s <ip-address>", argv[0]);
    exit(1);
  }
  int sd;
  struct msg_st sbuf;
  struct sockaddr_in raddr;
  sd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sd < 0) {
    perror("socker()");
    exit(1);
  }
  // bind()

  // sbuf
  strcpy(sbuf.name, "Alan");
  sbuf.math = htonl(rand() % 100);
  sbuf.chinese = htonl(rand() % 100);

  // raddr
  raddr.sin_family = AF_INET;
  raddr.sin_port = htons(atoi(RCVPORT));
  inet_pton(AF_INET, argv[1], &raddr.sin_addr);
  if (sendto(sd, &sbuf, sizeof(sbuf), 0, (void *)&raddr, sizeof(raddr)) < 0) {
    perror("sendto()");
    exit(1);
  }
  puts("ok！");
  close(sd);
  exit(0);
}