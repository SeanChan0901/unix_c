#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>
#include <unistd.h>

#include "mytbf_sa.h"

// 流量控制 ： 令牌桶
// 任何类型的影音或者音频播放器都会做流控制

#define CPS 10           // 一秒钟10个字符
#define BUFFERSIZE 1024  // BUFSIZE
#define BURST 100        // 桶数上限

int main(int argc, char *argv[]) {
  int sfd, dfd = 1;         // 文件描述符
  char buffer[BUFFERSIZE];  // buffer
  int len;                  // 读取长度
  int ret;                  // 写的返回结果
  int pos;                  // 当前写到的位置
  mytbf_t *tbf;             // 桶令牌
  int size;                 // 令牌桶中的令牌个数

  // 判断参数
  if (argc < 2) {
    fprintf(stderr, "Usage : %s <pathname>\n", argv[0]);
    exit(1);
  }
  tbf = mytbf_init(CPS, BURST);  // 创建一个令牌桶
  if (tbf == NULL) {
    // 如果创建令牌失败
    fprintf(stderr, "mytbf_init() failed!\n");
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
    size = mytbf_fetchtoken(tbf, BUFFERSIZE);  // 获取令牌
    if (size < 0) {
      fprintf(stderr, "mytbf_fetchtoken()%s",
              strerror(-size));  // 利用errorno报错
      exit(1);
    }
    // 拷贝文件内容
    // read是阻塞系统调用，如果没有返回信息会等在那。
    while ((len = read(sfd, buffer, size)) < 0) {
      if (errno == EINTR) continue;  // 如果被信号打断则继续再读一遍
      // 报错
      perror("read()");
      break;
    }

    // 读到文件尾了
    if (len == 0) break;

    if (size - len > 0) {
      // 如果令牌还没有用完就归还令牌到桶中
      mytbf_returntoken(tbf, size - len);
    }

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
  mytbf_destory(tbf);  // 销毁令牌桶
  exit(0);
};