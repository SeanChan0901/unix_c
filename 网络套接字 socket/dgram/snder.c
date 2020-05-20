#include <stdlib.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>

#include "proto.h"

int main(int argc, char *argv[]) {
  if (argc < 3) {
    fprintf(stderr, "Usage :%s <ip-address> <name>", argv[0]);
    exit(1);
  }
  if (strlen(argv[2]) > NAMEMAX) {
    fprintf(stderr, "%s : name is too long\n", argv[2]);
    exit(1);
  }
  int sd;
  struct msg_st *sbuf_ptr;
  struct sockaddr_in raddr;

  // ???????????
  int size = sizeof(struct msg_st) - 1 + strlen(argv[2]);
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
  // bind()

  // sbuf
  strcpy(sbuf_ptr->name, argv[2]);
  sbuf_ptr->math = htonl(rand() % 100);
  sbuf_ptr->chinese = htonl(rand() % 100);

  // raddr
  raddr.sin_family = AF_INET;
  raddr.sin_port = htons(atoi(RCVPORT));
  inet_pton(AF_INET, argv[1], &raddr.sin_addr);
  if (sendto(sd, sbuf_ptr, size, 0, (void *)&raddr, sizeof(raddr)) < 0) {
    perror("sendto()");
    exit(1);
  }
  puts("ok!\n");
  close(sd);
  exit(0);
}