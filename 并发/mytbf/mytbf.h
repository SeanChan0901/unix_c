#ifndef _MYTBF_H
#define _MYTBF_H

#define MYTBF_MAX 1024  // 最多可以有1024个令牌桶

typedef void mytbf_t;  // void*赋给任何类型的指针都没有问题

mytbf_t* mytbf_init(int cps, int brust);  // 创建一个令牌桶

int mytbf_fetchtoken(mytbf_t*, int);  // 获取令牌

int mytbf_returntoken(mytbf_t*, int);  // 归还令牌

int mytbf_destory(mytbf_t*);  // 销毁一个令牌桶

#endif