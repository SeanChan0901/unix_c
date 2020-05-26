#ifndef CLIENT_H__
#define CLIENT_H__

#define DEFAULT_PLAYERCMD "/usr/local/bin/mpg123 > /dev/null"

// 命令行参数接收
struct client_conf_st {
  char* rcvport;
  char* multigroup;
  char* player_cmd;
};

// 默认参数
extern struct client_conf_st client_conf;

#endif