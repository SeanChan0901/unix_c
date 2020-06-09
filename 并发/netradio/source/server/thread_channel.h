#ifndef THR_CHANNEL_H__
#define THR_CHANNEL_H__

#include "medialib.h"

// 创建频道线程
int thread_channel_create(struct medialib_listentry_st*);

// join频道线程
int thread_channel_destory(struct medialib_listentry_st*);

// join所有频道线程
int thread_channel_destoryall();
#endif