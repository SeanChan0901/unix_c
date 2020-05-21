#ifndef PROTO_H
#define PROTO_H
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#define NAMEMAX 512 - 8 - 8
// 一个UDP包的推荐长度为512个字节，UDP的报头长度为8个字节，math和chinese各占4个字节

// 多播组的组号
#define MTGROUP "224.2.2.2"

#define RCVPORT "1989"
//  建议用1024以上的端口，1024之前的是被保留的
//  宏一定要有单位，没有单位没有意义，所以用了string类型的
//  到时候转成int

// 信息结构体
struct msg_st {
  uint32_t math;
  uint32_t chinese;
  uint8_t name[1];
} __attribute__((packed));  // 告诉gcc不要对齐

// 一个包的容量减去固有的定长的容量就是剩下name的容量

#endif
