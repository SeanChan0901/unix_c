#ifndef THREAD_LIST__
#define THREAD_LIST__
#include "medialib.h"

// 创建节目单线程
int thread_list_create(struct medialib_listentry_st*, int);

// 销毁节目单线程(join)
int thread_list_destory(void);

#endif