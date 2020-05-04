#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/*
 * date +%s
 */
int main() {
  puts("Begin!");
  fflush(NULL);
  execl("/bin/date", "date", "+%s", NULL);
  perror("execl()");
  exit(1);  // 如果能够继续进行下去，那么肯定出错了。
  puts("End!");
  exit(0);
}