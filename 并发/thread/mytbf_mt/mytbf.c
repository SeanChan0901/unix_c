#include "mytbf.h"

#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
// 封装成库

// 计时器线程标识号
static pthread_t tid_alrm;
static pthread_once_t init_once = PTHREAD_ONCE_INIT;

// 令牌桶
struct mytbf_st {
  int cps;    // 桶大小
  int burst;  // 桶数上限
  int token;  // 当前桶数
  int pos;    // 自述下标
  pthread_mutex_t mut;
  pthread_cond_t cond;
};
static int min(int a, int b) { return (a > b ? b : a); }

// 令牌桶数组
static struct mytbf_st* job[MYTBF_MAX];  // 用static修饰自动初始化为全0
static pthread_mutex_t mutex_job = PTHREAD_MUTEX_INITIALIZER;  // 互斥量

// 计时器
static void* thr_alrm(void* p) {
  while (1) {
    pthread_mutex_lock(&mutex_job);
    for (int i = 0; i < MYTBF_MAX; i++) {
      if (job[i] != NULL) {
        pthread_mutex_lock(&job[i]->mut);
        job[i]->token += job[i]->cps;
        if ((job[i]->token) > (job[i]->burst)) job[i]->token = job[i]->burst;
        // pthread_cond_signal(&job[i]->cond);
        // 唤醒一个等待，具体是哪个并不清楚，并不一定就是你对应的job的等待
        pthread_cond_broadcast(&job[i]->cond);
        pthread_mutex_unlock(&job[i]->mut);
      }
    }
    pthread_mutex_unlock(&mutex_job);
    sleep(1);
  }
}

//  获得可用位置
static int get_free_pos_unlocked(void) {
  for (int i = 0; i < MYTBF_MAX; i++) {
    if (job[i] == NULL) return i;
  }
  return -1;
};

// 卸载模块
static void module_unload(void) {
  pthread_cancel(tid_alrm);
  pthread_join(tid_alrm, NULL);
  for (int i = 0; i < MYTBF_MAX; i++) {
    if (job[i] != NULL) mytbf_destory(job[i]);
  }
  pthread_mutex_destroy(&mutex_job);
};

// 装载模块
static void module_load(void) {
  int err;
  err = pthread_create(&tid_alrm, NULL, thr_alrm, NULL);
  if (err) {
    fprintf(stderr, "pthread_create():%s", strerror(err));
    exit(1);
  }
  atexit(module_unload);
};

// 创建一个令牌桶
mytbf_t* mytbf_init(int cps, int brust) {
  struct mytbf_st* me;
  int pos;

  pthread_once(&init_once, module_load);

  me = malloc(sizeof(*me));  // 分配内存空间
  if (me == NULL) {
    // 分配失败
    return NULL;
  }
  me->cps = cps;
  me->burst = brust;
  me->token = 0;
  pthread_mutex_init(&me->mut, NULL);
  pthread_cond_init(&me->cond, NULL);
  pthread_mutex_lock(&mutex_job);  // 加锁
  pos = get_free_pos_unlocked();   // 获取可用令牌桶的位置
  if (pos < 0) {
    // 没有可用位置
    // 解锁
    pthread_mutex_unlock(&mutex_job);
    free(me);     // 释放堆上的空间
    return NULL;  // 跳转之前记得解锁
  }
  me->pos = pos;
  job[pos] = me;                     // 放进桶数组里面
  pthread_mutex_unlock(&mutex_job);  // 解锁
  return me;
};

// 获取令牌
int mytbf_fetchtoken(mytbf_t* tbf, int size) {
  struct mytbf_st* me = tbf;
  if (size <= 0) return -EINVAL;  // 如果size不符合要求，返回ERRNO
  pthread_mutex_lock(&me->mut);   // 加锁

  /*  查询法
  while (me->token <= 0) {
    pthread_mutex_unlock(&me->mut);  // 等别人去增加token的值
    sched_yield();
    pthread_mutex_lock(&me->mut);
  }
  */

  // 通知法
  while (me->token <= 0) {
    pthread_cond_wait(&me->cond, &me->mut);  // 解锁等待
  }

  int n = min(me->token, size);
  me->token -= n;
  pthread_mutex_unlock(&me->mut);
  return n;  // 返回取到的令牌数
}

// 归还令牌
int mytbf_returntoken(mytbf_t* tbf, int size) {
  struct mytbf_st* me = tbf;
  if (size <= 0) return -EINVAL;  // 归还数目不对
  pthread_mutex_lock(&me->mut);
  me->token += size;
  if (me->token > me->burst) me->token = me->burst;
  pthread_cond_broadcast(&me->cond);  // 还回去了之后也通知一下
  pthread_mutex_unlock(&me->mut);
  return size;
};

// 销毁一个令牌
int mytbf_destory(mytbf_t* tbf) {
  if (tbf == NULL) return 0;
  struct mytbf_st* me = tbf;
  pthread_mutex_lock(&mutex_job);  // 加锁
  job[me->pos] = NULL;
  pthread_mutex_unlock(&mutex_job);  // 解锁
  pthread_mutex_destroy(&(me->mut));
  pthread_cond_destroy(&me->cond);
  free(me);
  return 0;  // 表示删除成功
}
