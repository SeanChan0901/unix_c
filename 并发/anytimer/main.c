#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include "anytimer.h"

void f1(void* p) { printf("f1():%s\n", p); }
void f2(void* p) { printf("f2():%s\n", p); }
void f3(void* p) { printf("f3():%s\n", p); }

int main() {
  int job1, job2, job3;
  puts("Begin");
  job1 = at_addjob_repeat(5, f1, "aaa");
  job2 = at_addjob(2, f2, "bbb");
  job3 = at_addjob(7, f3, "ccc");
  while (1) {
    write(1, ".", 1);
    sleep(1);
  }
  puts("End!");
  exit(0);
}