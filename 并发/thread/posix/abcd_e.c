#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define THRUM 4

static pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;  // 互斥量初始化
static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
static int n = 0;
static int next(int i) { return ((i + 1) > 3 ? 0 : i + 1); };

static void *thr_func(void *p) {
  int num = (int)p;
  int c = 'a' + num;
  while (1) {
    pthread_mutex_lock(&mut);  // 上锁
    while (num != n)
      pthread_cond_wait(&cond, &mut);  // 不是我自己的话就解锁等待
    write(1, &c, 1);                   // 是我自己的话就写
    n = next(num);
    pthread_cond_broadcast(&cond);  // 唤醒
    pthread_mutex_unlock(&mut);
  }
  pthread_exit(NULL);
}

int main() {
  int err;
  pthread_t tids[THRUM];
  for (int i = 0; i < THRUM; i++) {
    err = pthread_create(tids + i, NULL, thr_func, (void *)i);
    if (err) {
      fprintf(stderr, "pthread_create():%s\n", strerror(err));
      exit(1);
    }
  }
  alarm(5);
  for (int i = 0; i < THRUM; i++) {
    pthread_join(tids[i], NULL);
  }
  pthread_mutex_destroy(&mut);
  pthread_cond_destroy(&cond);
  exit(0);
}