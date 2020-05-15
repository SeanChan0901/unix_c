#include "mypipe.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

// 管道本身就是阻塞的

struct mypipe_st {
  int head;  // 头
  int tail;  // 胃
  char data[PIPESIZE];
  int datasize;  // 当前管道的有效字节数
  int count_rd;
  int count_wr;
  pthread_mutex_t mut;  // 独占管道
  pthread_cond_t cond;
};

// 循环队列求下一个位置
static int next(int now) { return (now + 1) % BUFSIZ; }

// 注册身份
int mypipe_register(mypipe_t* ptr, int opmap) {
  if ((opmap & MYPIPE_READER) && (opmap & MYPIPE_WRITER)) {
    struct mypipe_st* me = ptr;
    pthread_mutex_lock(&me->mut);
    if (opmap & MYPIPE_READER) me->count_rd++;
    if (opmap & MYPIPE_WRITER) me->count_wr++;
    pthread_cond_broadcast(&me->cond);  // 如果做了注册操作就广播
    while (me->count_rd <= 0 || me->count_wr <= 0)
      pthread_cond_wait(&me->cond, &me->mut);  // 必须既有读者又有写者才开始
    pthread_mutex_unlock(&me->mut);
    return 0;
  }
  return -1;
};

// 注销身份
int mypipe_unregister(mypipe_t* ptr, int opmap) {
  if ((opmap & MYPIPE_READER) && (opmap & MYPIPE_WRITER)) {
    struct mypipe_st* me = ptr;
    pthread_mutex_lock(&me->mut);
    // 这里有一点小问题的，如果已经为0应该是不允许再取消的
    if ((opmap & MYPIPE_READER) && me->count_rd > 0) me->count_rd--;
    if ((opmap & MYPIPE_WRITER) && me->count_wr > 0) me->count_wr--;
    pthread_cond_broadcast(&me->cond);
    pthread_mutex_unlock(&me->mut);
    return 0;
  }
  return -1;
}

// 读单字节
static int mypipe_readByte_unlock(struct mypipe_st* me, char* datap) {
  if (me->datasize <= 0) return -1;
  *datap = me->data[me->head];  // 从头开始读
  me->head = next(me->head);
  me->datasize--;  // 读走了那么有效字节数减少
  return 0;
};

// 写单字节
static int mypipe_writeByte_unlock(struct mypipe_st* me, const char* datap) {
  if (me->datasize >= BUFSIZ) return -1;
  me->data[me->tail] = *datap;  // 写
  me->tail = next(me->tail);
  me->datasize++;  // 读走了那么有效字节数减少
  return 0;
};

// 初始化管道
/*
 *  RETURN VALUE: 1)NULL 2）
 *
 */
mypipe_t* mypipe_init(void) {
  struct mypipe_st* me;
  me = malloc(sizeof(*me));
  if (me == NULL) return NULL;
  me->head = 0;
  me->tail = 0;
  me->datasize = 0;
  me->count_rd = 0;
  me->count_wr = 0;
  pthread_mutex_init(&me->mut, NULL);
  pthread_cond_init(&me->cond, NULL);
  return me;
};

// 读管道
int mypipe_read(mypipe_t* ptr, void* buf, size_t count) {
  struct mypipe_st* me = ptr;
  int i = 0;
  pthread_mutex_lock(&me->mut);
  while ((me->datasize <= 0) && (me->count_rd > 0))
    pthread_cond_wait(&me->cond, &me->mut);
  if ((me->datasize <= 0) && (me->count_rd < 0)) {
    // 如果管道没内容且目前没有写者那么就不要等下去了直接返回0
    pthread_mutex_unlock(&me->mut);
    return 0;
  }
  for (i = 0; i < count; i++)
    if (mypipe_readByte_unlock(me, buf + i) != 0) break;
  pthread_cond_broadcast(&me->cond);
  pthread_mutex_unlock(&me->mut);
  return i;
};

// 写管道
int mypipe_write(mypipe_t* ptr, const void* buf, size_t count) {
  struct mypipe_st* me = ptr;
  int i = 0;
  pthread_mutex_lock(&me->mut);
  while (me->datasize >= BUFSIZ && (me->count_rd >= 0))
    // 管道满
    pthread_cond_wait(&me->cond, &me->mut);
  if (me->datasize >= BUFSIZ && me->count_rd <= 0) {
    // 如果管道满而且没有读者那么也可以直接返回了，没有必要等待了
    pthread_mutex_lock(&me->mut);
    return 0;
  }
  for (i = 0; i < count; i++)
    if (mypipe_writeByte_unlock(me, buf + i) != 0) break;
  pthread_cond_broadcast(&me->cond);
  pthread_mutex_unlock(&me->mut);
  return i;
};

// 销毁管道
int mypipe_destroy(mypipe_t* ptr) {
  struct mypipe_st* me = ptr;
  pthread_cond_destroy(&me->cond);
  pthread_mutex_destroy(&me->mut);
  free(ptr);
  return 0;
}