#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "mysem.h"
#define LEFT 30000000
#define RIGHT 30000200
#define THRUM (RIGHT - LEFT + 1)
#define N 4

static mysem_t *sem;  // 令牌桶

// 线程函数
static void *thr_primer(void *p) {
  int n = (int)p;
  int mark = 1;
  for (int i = 2; i < n / 2; i++) {
    mark = 1;
    if (n % i == 0) {
      mark = 0;
      break;
    }
  }
  if (mark != 0) {
    fprintf(stdout, "[%d]%d is a primer\n", (n - LEFT), n);
    fflush(stdout);
  }
  mysem_add(sem, 1);  // 归还
  pthread_exit(NULL);
}

int main() {
  int err;
  pthread_t tid[THRUM];  //  总线程数目
  sem = mysem_init(N);   // 初始化令牌桶
  if (sem == NULL) {
    fprintf(stderr, "mysem_init() failed\n");
    exit(1);
  }
  for (int i = LEFT; i <= RIGHT; i++) {
    // 从桶内拿出令牌，分发（最多N个令牌）
    mysem_sub(sem, 1);
    err = pthread_create(tid + (i - LEFT), NULL, thr_primer, (void *)i);
    if (err) {
      fprintf(stderr, "pthread_create():%s", strerror(err));
      exit(1);
    }
  }
  for (int i = 0; i < THRUM; i++) {
    pthread_join(tid[i], NULL);
  }
  mysem_destroy(sem);
  exit(0);
}