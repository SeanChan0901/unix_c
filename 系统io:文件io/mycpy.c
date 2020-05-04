#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define BUFFERSIZE 1024

int main(int argc, char *argv[]) {
  int sfd, dfd;             // 文件描述符
  char buffer[BUFFERSIZE];  // buffer
  int len;                  // 读取长度
  int ret;                  // 写的返回结果
  int pos;                  // 当前写到的位置

  // 判断参数
  if (argc < 3) {
    fprintf(stderr, "Usage : %s <pathname> <pathname>", argv[0]);
    exit(1);
  }

  // 打开源文件
  sfd = open(argv[1], O_RDONLY);
  if (sfd < 0) {
    // 打开失败
    perror("open()");
    exit(1);
  }

  // 打开目标文件，没有则创建，有则截断
  dfd = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0600);
  if (dfd < 0) {
    // 打开文件失败
    perror("open()");
    close(sfd);  // 关闭源文件
    exit(1);
  }

  while (1) {
    len = read(sfd, buffer, BUFFERSIZE);
    // 拷贝文件内容
    if (len < 0) {
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
        // 报错
        perror("write()");
        exit(1);  // 允许小范围的内存泄漏
      }
      len -= ret;  // 如果没有写完，那么记录写到哪
      pos = ret;   // 目前写到的位置
    }
  }

  close(dfd);
  close(sfd);
  exit(0);
};