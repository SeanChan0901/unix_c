#include "mysem.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// 线程桶：限定线程资源数

struct mysem_st {
  int value;
  pthread_mutex_t mut;  // 互斥量
  pthread_cond_t cond;  // 条件变量
};

mysem_t* mysem_init(int intval) {
  struct mysem_st* me;
  me = malloc(sizeof(*me));
  if (me == NULL) return NULL;
  pthread_mutex_init(&me->mut, NULL);
  pthread_cond_init(&me->cond, NULL);
  me->value = intval;
  return me;
};

// 归还资源量
int mysem_add(mysem_t* ptr, int n) {
  struct mysem_st* me = ptr;
  pthread_mutex_lock(&me->mut);
  me->value += n;
  pthread_cond_broadcast(&me->cond);  // 归还了之后，唤醒所有等待的线程
  pthread_mutex_unlock(&me->mut);
  return n;
}

// 分发资源量
int mysem_sub(mysem_t* ptr, int n) {
  struct mysem_st* me = ptr;
  pthread_mutex_lock(&me->mut);  // 查看资源量的时候上锁
  while (me->value < n)
    pthread_cond_wait(&me->cond, &me->mut);  // 如果不够的话就等待
  me->value -= n;                            //  取走资源
  pthread_mutex_unlock(&me->mut);            // 解锁
  return n;
}

int mysem_destroy(mysem_t* ptr) {
  struct mysem_st* me = ptr;
  pthread_mutex_destroy(&me->mut);
  pthread_cond_destroy(&me->cond);
  free(me);
  return 0;
}
