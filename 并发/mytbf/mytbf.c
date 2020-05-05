#include "mytbf.h"

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
// 封装成库

static int inited = 0;
typedef void (*sig_t)(int);
static sig_t alrm_handler_save;  // 前一个触发器

// 令牌桶
struct mytbf_st {
  int cps;    // 桶大小
  int burst;  // 桶数上限
  int token;  // 当前桶数
  int pos;    // 自述下标
};
static int min(int a, int b) { return (a > b ? b : a); }

// 令牌桶数组
static struct mytbf_st* job[MYTBF_MAX];  // 用static修饰自动初始化为全0

static void alrm_handler(int s) {
  alarm(1);  // 下一次时钟信号
  for (int i = 0; i < MYTBF_MAX; i++) {
    if (job[i] != NULL) {
      job[i]->token += job[i]->cps;
      if (job[i]->token > job[i]->burst) job[i]->token = job[i]->burst;
    }
  }
}

// 解除时钟信号
static void module_unload(void) {
  signal(SIGALRM, alrm_handler_save);  // 恢复之前的触发器
  alarm(0);                            // 关闭闹钟
  for (int i = 0; i < MYTBF_MAX; i++) {
    free(job[i]);
  }
}

//  初始化时钟
static void module_load(void) {
  alrm_handler_save = signal(SIGALRM, alrm_handler);
  alarm(1);
  atexit(module_unload);  // 钩子函数
}

//  获得可用位置
static int get_free_pos(void) {
  for (int i = 0; i < MYTBF_MAX; i++) {
    if (job[i] == NULL) return i;
  }
  return -1;
};

// 创建一个令牌桶
mytbf_t* mytbf_init(int cps, int brust) {
  struct mytbf_st* me;  // 申请令牌桶
  if (!inited) {
    // 如果没有初始化的话就加入alarm 和 handler
    module_load();
    inited = 1;
  }
  int pos = get_free_pos();  // 给令牌桶找位置
  if (pos < 0) return NULL;  // 没有位置了就创建失败

  me = malloc(sizeof(me));
  if (me == NULL) return NULL;
  me->token = 0;
  me->cps = cps;
  me->burst = brust;
  me->pos = pos;
  job[pos] = me;  // 放进桶数组里面
  return me;
};

// 获取令牌
int mytbf_fetchtoken(mytbf_t* ptr, int size) {
  struct mytbf_st* me = ptr;
  if (size <= 0) return -EINVAL;
  while (me->token <= 0) pause();
  int n = min(size, me->token);
  me->token -= n;
  return n;
}

// 归还令牌
int mytbf_returntoken(mytbf_t* ptr, int size) {
  struct mytbf_st* me = ptr;
  if (size <= 0) return -EINVAL;
  me->token += size;
  if (me->token > me->burst) me->token = me->burst;
  return size;
};

// 销毁令牌桶
int mytbf_destory(mytbf_t* ptr) {
  struct mytbf_st* me = ptr;
  job[me->pos] = NULL;
  free(me);
  return 0;
};