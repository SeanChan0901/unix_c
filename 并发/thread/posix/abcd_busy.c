#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define THRUM 4

static pthread_mutex_t mut[THRUM];

static int next(int i) { return ((i + 1) > 3 ? 0 : i + 1); };

static void *thr_func(void *p) {
  int n = (int)p;
  int c = 'a' + n;
  while (1) {
    pthread_mutex_lock(mut + n);  // 给自己上锁
    write(1, &c, 1);
    pthread_mutex_unlock(mut + next(n));  // 给下一个解锁
  }
  pthread_exit(NULL);
}

int main() {
  int err;
  pthread_t tids[THRUM];
  for (int i = 0; i < THRUM; i++) {
    pthread_mutex_init(mut + i, NULL);  // 初始化互斥量
    pthread_mutex_lock(mut + i);        // 上锁
    err = pthread_create(tids + i, NULL, thr_func, (void *)i);
    if (err) {
      fprintf(stderr, "pthread_create():%s\n", strerror(err));
      exit(1);
    }
  }
  pthread_mutex_unlock(mut + 0);  // 解锁
  alarm(5);
  for (int i = 0; i < THRUM; i++) {
    pthread_join(tids[i], NULL);
  }
  exit(0);
}