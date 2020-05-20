#ifndef PROTO_H
#define PROTO_H
#include <stdio.h>
#include <stdlib.h>
#define NAMESIZE 11
#define RCVPORT "1989"
//  建议用1024以上的端口，1024之前的是被保留的
//  宏一定要有单位，没有单位没有意义，所以用了string类型的
//  到时候转成int

// 信息结构体
struct msg_st {
  uint8_t name[NAMESIZE];
  uint32_t math;
  uint32_t chinese;
} __attribute__((packed));  // 告诉gcc不要对齐

//

#endif