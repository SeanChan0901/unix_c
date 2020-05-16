#ifndef _MYPIPE_H
#define _MYPIPE_H

#include <stdlib.h>
// 管道大小 （线程管道）
#define PIPESIZE 1024
#define MYPIPE_READER 0x00000001UL  // 位图
#define MYPIPE_WRITER 0x00000002UL  // 位图

typedef void mypipe_t;

// 注册身份
int mypipe_register(mypipe_t*, int opmap);

// 注销身份
int mypipe_unregister(mypipe_t*, int opmap);

// 初始化管道
mypipe_t* mypipe_init(void);

// 读管道
int mypipe_read(mypipe_t*, void* buf, size_t size);

// 写管道
int mypipe_write(mypipe_t*, const void* buf, size_t size);

// 销毁管道
int mypipe_destroy(mypipe_t*);
#endif