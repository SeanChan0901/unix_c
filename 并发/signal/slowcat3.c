#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/errno.h>
#include <sys/time.h>
#include <unistd.h>

// 流量控制 : 漏桶
// 任何类型的影音或者音频播放器都会做流控制
// 用setitimer来实现

#define CPS 10  // 一秒钟10个字符
#define BUFFERSIZE CPS

static volatile int loop = 1;  // 不加volatile优化会有什么结果

static void alrm_handler(int s) {
  // alarm(1);  // 下一次时钟信号
  loop = 0;
};

int main(int argc, char *argv[]) {
  int sfd, dfd = 1;         // 文件描述符
  char buffer[BUFFERSIZE];  // buffer
  int len;                  // 读取长度
  int ret;                  // 写的返回结果
  int pos;                  // 当前写到的位置
  struct itimerval itv;     // setitimer

  // 判断参数
  if (argc < 2) {
    fprintf(stderr, "Usage : %s <pathname>\n", argv[0]);
    exit(1);
  }
  signal(SIGALRM, alrm_handler);
  // alarm(1);
  itv.it_interval.tv_sec = 1;
  itv.it_interval.tv_usec = 0;
  itv.it_value.tv_sec = 1;
  itv.it_value.tv_usec = 0;
  if (setitimer(ITIMER_REAL, &itv, NULL) < 0) {
    perror("setitimer()");
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
    while (loop) pause();  // 等待信号
    loop = 1;              // 重置循环

    // 拷贝文件内容
    while ((len = read(sfd, buffer, BUFFERSIZE)) < 0) {
      if (errno == EINTR) continue;  // 如果被信号打断则继续再读一遍
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