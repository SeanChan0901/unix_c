// 接收端
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>

#include "proto.h"

int main() {
  key_t key;
  int msgid;
  struct msg_st rbuf;
  key = ftok(KEYPATH, KEYPROJ);
  if (key < 0) {
    perror("ftok()");
    exit(1);
  }
  msgid = msgget(key, IPC_CREAT | 0600);  // 一定要接收方先创建
  if (msgid < 0) {
    perror("msgget()");
    exit(1);
  }
  while (1) {
    // 需要接收的字节数可以用减法来确定
    if (msgrcv(msgid, &rbuf, sizeof(rbuf) - sizeof(long), 0, 0) < 0) {
      perror("msgrcv()");
      exit(1);
    }
    // 接收信息
    printf("NAME = %s\n", rbuf.name);
    printf("MATH = %d\n", rbuf.math);
    printf("CHINESE = %d\n", rbuf.chinese);
  }
  if (msgctl(msgid, IPC_RMID, NULL) < 0) {
    perror("msgctl");
    exit(1);
  }
  exit(0);
}