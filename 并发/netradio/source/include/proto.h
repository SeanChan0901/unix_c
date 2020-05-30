#ifndef PROTO_H__
#define PROTO_H__

#include "site_type.h"

#define DEFAULT_MULTIGROUP "224.2.2.2"        // 多播组IP地址
#define DEFAULT_RECEIVEPORT "1989"            // 端口号
#define CHANNELNUM 100                        // 频道数
#define LISTCHNID 0                           // 0号频道发送节目单
#define MINCHNID 1                            // 最小频道号
#define MAXCHNID (MINCHNID + CHANNELNUM - 1)  // 最大频道号

#define MSG_CHANNEL_MAX (65536 - 20 - 8)              // channel包最大长度
#define MAX_DATA (MSG_CHANNEL_MAX - sizeof(chnid_t))  // data最大长度

#define MSG_LIST_MAX (65536 - 20 - 8)               // list包最大长度
#define MAX_ENTRY (MSG_LIST_MAX - sizeof(chnid_t))  // entry最大长度

struct msg_channel_st {
  chnid_t chnid;
  uint8_t data[1];
} __attribute__((packed));

// 条目
struct msg_listentry_st {
  chnid_t chnid;
  uint16_t len;  // 当前包长
  uint8_t desc[1];
} __attribute__((packed));

struct msg_list_st {
  chnid_t chnid;  // must be LISTCHNID
  struct msg_listentry_st entry[1];
} __attribute__((packed));

#endif