#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define FNAME "/tmp/out"
#define BUFFERSIZE 1024

int main() {
  // 读从头读，写从尾写
  FILE *fp;
  char buf[BUFFERSIZE];
  int count = 0;  // 文件行数
  time_t stamp;   // 时间戳
  struct tm *tm;
  fp = fopen(FNAME, "a+");
  if (fp == NULL) {
    // 打开失败
    perror("fopen()");
    exit(1);
  }

  // 读行数
  while (fgets(buf, BUFFERSIZE, fp) != NULL) count++;

  //  除了标准终端设备，其他形式默认都是全缓冲形式
  while (1) {
    time(&stamp);
    tm = localtime(&stamp);
    // 用printf的话要自己转化
    fprintf(fp, "%-4d%d-%d-%d %d:%d:%d\n", ++count, tm->tm_year+1900, tm->tm_mon+1,
            tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
    fflush(fp);
    sleep(1);
  }

  fclose(fp);
  exit(0);
}