#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// 多线程并发计算primer
// 利用线程池分发任务
// 通知法

#define LEFT 30000000
#define RIGHT 30000200
#define THRUM 4  // 线程数

static int num = 0;  // 线程池的状态
static pthread_mutex_t mut_num = PTHREAD_MUTEX_INITIALIZER;  // 互斥量
static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;       // 条件变量

struct thr_arg_st {
  int n;
};

static void* thr_primer(void* p) {
  int mark, i;
  while (1) {
    pthread_mutex_lock(&mut_num);  // 读num的时候，上锁
    while (num == 0) {
      // 如果没有任务，解锁等分发
      pthread_cond_wait(&cond, &mut_num);  //  解锁等待发放任务
    }
    if (num == -1) {
      // 如果任务已经结束了
      pthread_mutex_unlock(&mut_num);  // 一定解锁！！！千万记住
      break;  // 一定要注意临界区里面的所有跳转，一定要小心小心再小心
    }

    // 如果线程池有任务，那就抢任务
    i = num;
    num = 0;                         // 清空池
    pthread_cond_broadcast(&cond);   // 发信号通知任务发放线程
    pthread_mutex_unlock(&mut_num);  // 解锁
    for (int j = 2; j < i / 2; j++) {
      mark = 1;
      if (i % j == 0) {
        mark = 0;
        break;
      }
    }
    if (mark) {
      printf("[%d]%d is a primer\n", (int)p, i);
    }
  }
  pthread_exit(p);
}

int main() {
  int i, err;
  pthread_t tids[THRUM];  // 线程数
  for (i = 0; i < THRUM; i++) {
    err = pthread_create(tids + i, NULL, thr_primer, (void*)i);
    if (err) {
      fprintf(stderr, "pthread_create()%s\n", strerror(err));
      exit(1);
    }
  }
  for (int j = LEFT; j <= RIGHT; j++) {
    // 放进池
    pthread_mutex_lock(&mut_num);  // 上锁，分发任务
    while (num != 0) {
      // 任务还在
      pthread_cond_wait(&cond, &mut_num);
    }
    // 任务不在，分发任务
    num = j;
    pthread_cond_signal(&cond);      // 通知发放任务
    pthread_mutex_unlock(&mut_num);  // 解锁，让别人来拿任务
  }
  pthread_mutex_lock(&mut_num);  //  写num的时候记得加锁
  while (num != 0) {
    pthread_cond_wait(&cond, &mut_num);
  }
  num = -1;  // 用于线程退出
  pthread_cond_broadcast(&cond);
  pthread_mutex_unlock(&mut_num);  // 解锁
  for (i = 0; i < THRUM; i++) {
    pthread_join(tids[i], NULL);
  }
  pthread_cond_destroy(&cond);      // 销毁条件变量
  pthread_mutex_destroy(&mut_num);  // 销毁互斥量
  exit(0);
}