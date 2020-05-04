#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
  // 用于接受输入内容的缓冲字符串块
  char buf[1024];
  int year = 2020, month = 4, date = 22;

  // atoi将字符转化为整数，遇到非数字类型的字符自动中断.

  // 输出到标准输出流中
  sprintf(buf, "%d-%d-%d", year, month, date);
  puts(buf);
  exit(0);
};