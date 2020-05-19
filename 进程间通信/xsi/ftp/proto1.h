// 实现ftp传输（用linux）
#ifndef PROTO1_H
#define PROTO1_H

#define PATHMAX 1024
#define DATAMAX 1024

// 包的类型
enum { MSG_PATH = 1, MSG_DATA, MSG_EOT };

//  路径包
typedef struct msg_path_st {
  long mtpe;           // must be MSG_PATH
  char path[PATHMAX];  // ASCIIz带尾0的串
} msg_path_t;

// 数据包
typedef struct msg_data_st {
  long mtype;  // must be MSG_DATA
  char data[DATAMAX];
  int datalen;
} msg_data_t;

// 结束符号包
typedef struct msg_eot_st {
  long mtype;  // must be MSG_EOT
} msg_eot_t;

// 需要收到才能知道是什么类型的data,用这个来封装
union msg_s2c_un {
  long mtype;
  msg_data_t datamsg;
  msg_eot_st eotmsg;
};

#endif