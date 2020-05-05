#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

// 5秒输出 误差版
// time耗时间

int main() {
  time_t end;
  int64_t count = 0;
  end = time(NULL) + 5;
  while (time(NULL) <= end) {
    count++;
  }
  printf("%lld\n", count);
  exit(0);
}