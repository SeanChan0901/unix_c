// 通信协议
// mac没有msg通信，在linux上重新写遍
#ifndef _PROTO_H
#define _PROTO_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define KEYPATH "/etc/services"
#define KEYPROJ 'g'  // 表示一个整形（一个ASCII码）
#define NAMESIZE 32

struct msg_st {
  // 数据传输格式
  long mtype;
  char name[NAMESIZE];
  int math;
  int chinese;
};

#endif
