#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <time.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <errno.h>
#include "proto.h"

// 并发版本，把accept和server——job分开，这样即便server_job需要耗掉时间也不会影响
// 继续accept工作
// 引入动态进程池

#define MINSPARESERVER 5   // 空闲子进程的数量不超过五个
#define MAXSPARESERVER 10  // 空闲的子进程的数量不高于10个
#define MAXCLIENTS 20
#define IPSTRSIZE 40
#define SIG_NOTIFY SIGUSR2  // 自定义信号行为

enum {
  STATE_IDEL = 0,
  STATE_BUSY,
  // int reuse, 用的次数超过某个上限就杀掉，重新创建一个进程（防止bug扩散）
};

struct server_st {
  pid_t pid;
  int state;
};

static struct server_st *serverpool;
static int idle_count = 0;
static int busy_count = 0;
static int sd;  // socket id

static void usr2_handler(int s) { return; }

static int scan_pool(void) {
  int busy = 0;
  int idle = 0;
  for (int i = 0; i < MAXCLIENTS; i++) {
    if (serverpool[i].pid == -1) continue;
    if (kill(serverpool[i].pid, 0)) {
      // 检测进程是否存在
      // 有可能进程已经消亡，更新进程状态
      serverpool[i].pid = -1;
      continue;
    }
    if (serverpool[i].state == STATE_IDEL) {
      idle++;
    } else if (serverpool[i].state == STATE_BUSY)
      busy++;
    else {
      // 状态为止，发生大错误
      fprintf(stderr, "Unknow state.\n");
      // _exit(1);  // 不要刷新流直接退出
      abort();  // 杀掉当前进程，获得一个coredown文件
    }
  }
  // 更新进程池
  idle_count = idle;
  busy_count = busy;
  return 0;
}

static void server_job(int pos) {
  pid_t ppid;
  struct sockaddr_in raddr;  // 远端地址
  socklen_t raddr_len;       // 地址长度
  int client_sd;             // 接受连接的sd号
  time_t stamp;              // 时间戳
  int len;                   // 接收的字节长度
  char ipstr[IPSTRSIZE];     // 点分式ip
  char linebuf[BUFSIZ];
  ppid = getppid();
  raddr_len = sizeof(raddr);
  while (1) {
    kill(ppid, SIG_NOTIFY);  // 给父进程发信号确认我的状态
    client_sd = accept(sd, (void *)&raddr, &raddr_len);  // 等待接收连接
    if (client_sd < 0) {
      if (errno != EINTR && errno != EAGAIN) {
        perror("accept()");
        exit(1);
      }
    }
    serverpool[pos].state = STATE_BUSY;  // 接收连接，状态繁忙
    kill(ppid, SIG_NOTIFY);              // 通知父进程查看
    inet_ntop(AF_INET, &raddr.sin_addr, ipstr, IPSTRSIZE);
    // printf("[%d]client:%s:%d\n", getpid(), ipstr, ntohs(raddr.sin_port));
    stamp = time(NULL);
    len = snprintf(linebuf, BUFSIZ, FMT_STAMP, (long long)stamp);
    send(client_sd, linebuf, len, 0);
    close(client_sd);
    serverpool[pos].state = STATE_IDEL;  // 完成工作，状态空闲
  }
}

static int add_1_server(void) {
  pid_t pid;  // 子进程
  int slot;
  if (idle_count + busy_count >= MAXCLIENTS) return -1;  // 资源不足
  for (slot = 0; slot < MAXCLIENTS; slot++) {
    if (serverpool[slot].pid == -1) {
      break;  // 没有进程在这个位置
    }
  }
  serverpool[slot].state = STATE_IDEL;
  pid = fork();  // 初始化进程
  if (pid < 0) {
    perror("fork");
    exit(1);
  }
  if (pid == 0) {
    // child
    server_job(slot);  // slot号进程开始工作
    exit(1);
  }
  // parent
  serverpool[slot].pid = pid;  // 将进程号记录到进程池中
  idle_count++;                // 空闲进程数增加
  return 0;
};

static int del_1_server(void) {
  if (idle_count == 0) return -1;
  for (int i = 0; i < 20; i++) {
    if (serverpool[i].pid != -1 && serverpool[i].state == STATE_IDEL) {
      // 如果进程空闲,杀掉进程
      kill(serverpool[i].pid, SIGTERM);
      serverpool[i].pid = -1;
      idle_count--;
      break;  // 成功杀掉一个进程
    }
  }
  return 0;
};

int main() {
  struct sockaddr_in laddr;

  sigset_t set, oset;
  // 信号行为定义
  struct sigaction sa, osa;
  sa.sa_handler = SIG_IGN;  // 忽略SIGCHILD这个信号，让子进程自己释放资源
  sigemptyset(&sa.sa_mask);    // mask设置成空集
  sa.sa_flags = SA_NOCLDWAIT;  // 不让子进程变成僵尸状态
  sigaction(SIGCHLD, &sa, &osa);

  // 用于驱动的信号
  sa.sa_handler = usr2_handler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  sigaction(SIG_NOTIFY, &sa, &osa);

  sigemptyset(&set);
  sigaddset(&set, SIG_NOTIFY);
  sigprocmask(SIG_BLOCK, &set, &oset);
  serverpool = mmap(NULL, sizeof(struct server_st) * 20, PROT_READ | PROT_WRITE,
                    MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  if (serverpool == MAP_FAILED) {
    perror("mmap()");
    exit(1);
  }
  for (int i = 0; i < MAXCLIENTS; i++) serverpool[i].pid = -1;  // 初始化进程池
  // socket
  sd = socket(AF_INET, SOCK_STREAM, 0);
  if (sd < 0) {
    perror("socket()");
    exit(1);
  }

  int val = 1;
  // 使其断开的时候不需要等两分钟
  if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)) < 0) {
    perror("setsockopt()");
    exit(1);
  }

  laddr.sin_family = AF_INET;
  laddr.sin_port = htons(atoi(SERVERPORT));
  inet_pton(AF_INET, "0.0.0.0", &laddr.sin_addr);
  if (bind(sd, (void *)&laddr, sizeof(laddr)) < 0) {
    perror("bind()");
    exit(1);
  }

  if (listen(sd, 100) < 0) {
    perror("listen()");
    exit(1);
  }

  // 进程初始化
  for (int i = 0; i < MINSPARESERVER; i++) {
    add_1_server();  // 初始化进程并加入进程池
  }

  // 信号驱动程序
  while (1) {
    sigsuspend(&oset);  // 解除屏蔽字set进入睡眠
    scan_pool();        // 被信号唤醒，扫描进程池，检查状态

    // control the pool
    if (idle_count > MAXSPARESERVER) {
      // 空闲过多，杀掉（空闲）进程
      for (int i = 0; i < (idle_count - MAXSPARESERVER); i++) {
        del_1_server();  // 删除一个进程
      }
    } else if (idle_count < MINSPARESERVER) {
      // 空闲过少
      for (int i = 0; i < (MINSPARESERVER - idle_count); i++) {
        add_1_server();  // 增加一个（空闲）进程
      }
    }
    // print the pool
    for (int i = 0; i < MAXCLIENTS; i++) {
      if (serverpool[i].pid == -1) {
        putchar(' ');
      } else if (serverpool[i].state == STATE_IDEL) {
        putchar('.');
      } else {
        putchar('x');
      }
    }
    putchar('\n');
  }
  sigprocmask(SIG_SETMASK, &oset, NULL);
  exit(0);
}