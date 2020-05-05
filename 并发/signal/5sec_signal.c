#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// 异步事件，用信号提高精度：提高精度
// gcc 5sec_singal.c -O1 （优化）
// 不要相信内存的数据，要真的到数据存放的地方去娜数据

static volatile int loop = 1;  // 不加volatile优化会有什么结果

static void alarm_handler(int s) { loop = 0; };

int main() {
  int64_t count = 0;
  alarm(5);
  signal(SIGALRM, alarm_handler);
  while (loop) count++;
  exit(0);
}