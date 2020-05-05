#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/errno.h>
#include <unistd.h>

#define BUFFERSIZE 1024

// mycat

int main(int argc, char *argv[]) {
  int sfd, dfd = 1;         // 文件描述符
  char buffer[BUFFERSIZE];  // buffer
  int len;                  // 读取长度
  int ret;                  // 写的返回结果
  int pos;                  // 当前写到的位置

  // 判断参数
  if (argc < 2) {
    fprintf(stderr, "Usage : %s <pathname>", argv[0]);
    exit(1);
  }

  do {
    // 打开源文件
    sfd = open(argv[1], O_RDONLY);
    if (sfd < 0) {
      // 打开失败
      if (errno != EINTR) {
        // 如果不是被信号打断的,如果不是被信号打断的，那么就是真的出错了，退出
        perror("open()");
        exit(1);
      }
    }
  } while (sfd < 0);  // 不是被信号打断的，那么再重新打开一次

  while (1) {
    len = read(sfd, buffer, BUFFERSIZE);
    // 拷贝文件内容
    if (len < 0) {
      if (errno == EINTR) continue;  // 如果被信号打断则继续
      // 报错
      perror("read()");
      break;
    }

    // 读到文件尾了
    if (len == 0) break;

    int pos = 0;
    while (len > 0) {
      // 如果还没有把读到的写完
      // 写
      ret = write(dfd, buffer + pos, len);
      if (ret < 0) {
        if (errno == EINTR) continue;
        // 报错
        perror("write()");
        exit(1);  // 允许小范围的内存泄漏
      }
      len -= ret;  // 如果没有写完，那么记录写到哪
      pos = ret;   // 目前写到的位置
    }
  }
  close(sfd);
  exit(0);
};