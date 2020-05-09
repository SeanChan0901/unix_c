/*
 *中继模型
 */
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

#define TTY1 "/dev/tty11"
#define TTY2 "/dev/tty12"
#define BUF_SIZE 1024  // 缓冲区大小

// 状态机状态
enum {
  STATE_R = 1,   // 读态
  STATE_W = 2,   // 写
  STATE_Ex = 3,  // 异常态
  STATE_T = 4    // 结束态
};

// 有限状态机
struct fsm_st {
  int state;  // 当前状态
  int sfd;
  int dfd;
  int rlen;            // 读取长度
  int pos;             // 写入的当前位置
  char buf[BUF_SIZE];  // 缓冲区
  char* err_str;       // 出错原因
};

// 推状态机
void fsm_driver(struct fsm_st* fsm) {
  int ret;  // 写入结果
  switch (fsm->state) {
    case STATE_R:
      fsm->rlen = read(fsm->sfd, fsm->buf, BUF_SIZE);
      if (fsm->rlen < 0) {
        // 报错，进入Ex态
        if (errno == EINTR)
          // 假错，那么还是读态，不用管，直接break
          fsm->state = STATE_R;
        else {
          // 如果是真错,进入异常状态
          fsm->err_str = "read()";
          fsm->state = STATE_Ex;
        }
      } else if (fsm->rlen == 0)
        // 正常结束，进入T态
        fsm->state = STATE_T;
      else {           // 进入写状态
        fsm->pos = 0;  // 即将进入写态的几乎pos使其成为0
        fsm->state = STATE_W;
      }
      break;
    case STATE_W:
      // 写态：写
      ret = write(fsm->dfd, fsm->buf + fsm->pos, fsm->rlen);
      if (ret < 0) {
        //  报错，判断是否假错
        if (errno == EAGAIN)
          fsm->state = STATE_W;
        else {
          fsm->err_str = "write()";
          fsm->state = STATE_Ex;  // 进入错误状态
        }
      } else {
        fsm->rlen -= ret;  // 剩下多少没写
        if (fsm->rlen == 0)
          // 写入完成进入T
          fsm->state = STATE_T;
        else {
          // 还没完成则继续
          fsm->pos += ret;  // 记录当前的位置
          fsm->state = STATE_W;
        }
      }
      break;
    case STATE_Ex:
      perror(fsm->err_str);  // 报错
      fsm->state = STATE_T;
      break;
    case STATE_T:
      //  正常结束
      // do something
      break;
    default:
      abort();  // 发送一个异常，顺便得到一个core文件
      break;
  }
}

// 中继原理
static void relay(int fd1, int fd2) {
  int fd1_save, fd2_save;      // 文件1的特征
  struct fsm_st fsm12, fsm21;  // 状态机1和2
  fd1_save = fcntl(fd1, F_GETFL);
  fcntl(fd1, F_SETFL, fd1_save | O_NONBLOCK);  // 确保是非阻塞打开
  fd2_save = fcntl(fd2, F_GETFL);
  fcntl(fd2, F_SETFL, fd2_save | O_NONBLOCK);  // 确保是非阻塞打开

  fsm12.state = STATE_R;
  fsm12.sfd = 1;
  fsm12.dfd = 2;
  fsm21.state = STATE_R;
  fsm21.sfd = 2;
  fsm21.dfd = 1;

  while (fsm12.state != STATE_T || fsm21.state != STATE_T) {
    // 任意一个状态机没有到达终态
    fsm_driver(&fsm12);  // 推状态机12
    fsm_driver(&fsm21);  // 推状态机21
  }

  // 恢复之前的用户状态
  fcntl(fd1, F_SETFL, fd1_save);
  fcntl(fd2, F_SETFL, fd2_save);
}

int main() {
  int fd1 = -1, fd2 = -1;  // 文件描述符
  fd1 = open(TTY1, O_RDWR);
  if (fd1 < 0) {
    perror("open()");
    exit(1);
  }
  write(fd1, "tty1\n", 5);
  fd2 = open(TTY2, O_RDWR | O_NONBLOCK);
  if (fd2 < 0) {
    perror("open()");
    exit(1);
  }
  write(fd2, "tty2\n", 5);
  relay(fd1, fd2);  //  中继点操作
  close(fd1);
  close(fd2);
  exit(0);
}

// 中继模型