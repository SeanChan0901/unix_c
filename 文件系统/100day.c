#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define TIMESTRSIZE 1024

int main() {
  time_t stamp;   // 时间戳
  struct tm *tm;  // 时间结构体
  char timestr[TIMESTRSIZE];
  if ((stamp = time(NULL)) == -1) {
    perror("time()");
    exit(1);
  }
  if ((tm = localtime(&stamp)) == NULL) {
    // 判断是否获取结构体成功
    perror("location()");
    exit(1);
  }
  // 用strftime的话已经帮你换算好了(不用+1900)
  strftime(timestr, TIMESTRSIZE, "Now:%Y-%m-%d", tm);
  puts(timestr);

  tm->tm_mday += 100;
  (void)mktime(tm);  // 不用他的正常用法。
  strftime(timestr, TIMESTRSIZE, "100 days later:%Y-%m-%d", tm);
  puts(timestr);
  // mktime 可以调整struct_tm使其合法
  exit(0);
}