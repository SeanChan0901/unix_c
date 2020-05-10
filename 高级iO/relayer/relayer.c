#include "relayer.h"

#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

// 有限状态机的状态
enum { STATE_R = 1, STATE_W, STATE_Ex, STATE_T };

// 有限状态机
struct rel_fsm_st {
  int state;
  int sfd;
  int dfd;
  char *buf[BUFSIZ];
  int len;
  char *err_str;  // 错误信息
  int write_pos;  // 当前位置
  int64_t count;  //
};

// job的状态
struct rel_job_st {
  int fd1;
  int fd2;
  int job_state;
  struct rel_fsm_st fsm21, fsm12;  // 状态机
  int fd1_save, fd2_save;          // 文件描述符状态保存
  int job_pos;                     // 自述下标
};

static struct rel_job_st *rel_job[REL_JOBMAX];            // 任务数组
pthread_mutex_t mut_rel_job = PTHREAD_MUTEX_INITIALIZER;  // 互斥量
pthread_once_t init_once = PTHREAD_ONCE_INIT;  // 动态模块加载的单次初始化
static pthread_t tid_relayer;

// 推状态机
void driver(struct rel_fsm_st *fsm) {
  int ret = 0;
  switch (fsm->state) {
    case STATE_R:
      fsm->len = read(fsm->sfd, fsm->buf, BUFSIZ);  // 读
      if (fsm->len < 0) {
        if (errno == EINTR)
          // 假错，那么还是读态，不用管，直接break
          fsm->state = STATE_R;
        else {
          // 如果是真错,进入异常状态
          fsm->err_str = "read()";
          fsm->state = STATE_Ex;
        }
      } else if (fsm->len == 0)
        fsm->state = STATE_T;
      else {
        fsm->write_pos = 0;  // 准备写之前设置位置为0
        fsm->state = STATE_W;
      }
      break;
    case STATE_W:
      // 写态：写
      ret = write(fsm->dfd, fsm->buf + fsm->write_pos, fsm->len);
      if (ret < 0) {
        //  报错，判断是否假错
        if (errno == EAGAIN)
          fsm->state = STATE_W;
        else {
          fsm->err_str = "write()";
          fsm->state = STATE_Ex;  // 进入错误状态
        }
      } else {
        fsm->len -= ret;    // 剩下多少没写
        fsm->count += ret;  // 通信数据量
        if (fsm->len == 0) {
          // 写入完成进入T
          fsm->state = STATE_T;
        } else {
          // 还没完成则继续
          fsm->write_pos += ret;  // 记录当前的位置
          fsm->state = STATE_W;
        }
      }
      break;
    case STATE_T:
      // 正常结束
      break;
    case STATE_Ex:
      perror(fsm->err_str);
      fsm->state = STATE_T;
      break;
    default:
      abort();  // 发送一个异常，顺便得到一个core文件
      break;
  }
};

// 寻找可用位置
static int get_free_pos_unlock() {
  for (int i = 0; i < REL_JOBMAX; i++) {
    if (rel_job[i] == NULL) return i;
  }
  return -1;
};

// 永远推这些状态机 , 可以优化成多线程
static void *thr_relayer(void *p) {
  while (1) {
    pthread_mutex_lock(&mut_rel_job);  // 上锁
    for (int i = 0; i < REL_JOBMAX; i++) {
      if (rel_job[i] != NULL) {
        if (rel_job[i]->job_state == STATE_RUNNING) {
          // 确保job正在运行
          driver(&(rel_job[i]->fsm12));  // 推状态机
          driver(&(rel_job[i]->fsm21));  // 推状态机
          if ((rel_job[i]->fsm12.state == STATE_T) &&
              (rel_job[i]->fsm21.state == STATE_T))
            rel_job[i]->job_state = STATE_OVER;  // 任务完成
        }
      }
    }
    pthread_mutex_unlock(&mut_rel_job);
  }
  pthread_exit(NULL);
};

// 卸载状态机模块
static void module_unload(void) {
  // 收尸
  pthread_join(tid_relayer, NULL);
  for (int i = 0; i < REL_JOBMAX; i++) {
    free(rel_job[i]);  // 释放空间
  }
};

// 加载一个状态机模块，让他作为一个线程一直跑
static void module_load(void) {
  int err;
  err = pthread_create(&tid_relayer, NULL, thr_relayer, NULL);
  if (err) {
    fprintf(stderr, "pthread_create()%s\n", strerror(err));
    exit(1);
  }
  atexit(module_unload);  // 挂上勾子函数
};

/*
 * return >=0       成功，返回当前任务ID
 *        ==-EINVAL 失败，参数非法
 *        ==-ENOSPC 失败，任务数组满
 *        ==-ENOMEM 失败，malloc失败
 *
 */
int rel_addjob(int fd1, int fd2) {
  struct rel_job_st *me;
  pthread_once(&init_once, module_load);  // 动态模块的单次初始化
  me = malloc(sizeof(*me));
  if (me == NULL) return -ENOMEM;
  me->fd1 = fd1;
  me->fd2 = fd2;
  me->job_state = STATE_RUNNING;

  // 保证非阻塞的形式打开
  me->fd1_save = fcntl(me->fd1, F_GETFL);
  fcntl(me->fd1, F_SETFL, me->fd1_save | O_NONBLOCK);  // 设置成为非阻塞
  me->fd2_save = fcntl(me->fd2, F_GETFL);
  fcntl(me->fd2_save, F_SETFL, me->fd2_save | O_NONBLOCK);

  // 设置状态机 1
  me->fsm12.sfd = fd1;
  me->fsm12.dfd = fd2;
  me->fsm12.state = STATE_R;
  me->fsm12.count = 0;
  // 设置状态机 2
  me->fsm21.sfd = fd2;
  me->fsm21.dfd = fd1;
  me->fsm21.state = STATE_R;
  me->fsm21.count = 0;

  pthread_mutex_lock(&mut_rel_job);
  me->job_pos = get_free_pos_unlock();
  if (me->job_pos < 0) {
    pthread_mutex_unlock(&mut_rel_job);     // 释放锁
    fcntl(me->fd1, F_SETFL, me->fd1_save);  // 还原文件状态
    fcntl(me->fd2, F_SETFL, me->fd2_save);  // 还原文件状态
    free(me);                               // 释放空间
    return -ENOSPC;                         // 查找job空间失败
  }
  rel_job[me->job_pos] = me;  // 放入job数组中
  pthread_mutex_unlock(&mut_rel_job);
  return me->job_pos;  // 成功，返回作业ID
};

/*
 * return >=0       成功，指定任务成功取消
 *        ==-EINVAL 失败，参数非法
 *        ==-EBUSY  失败，指定任务无法重复取消
 */
int rel_canceljob(int id) {
  pthread_mutex_lock(&mut_rel_job);
  if (id > REL_JOBMAX || id < 0) pthread_mutex_unlock(&mut_rel_job);
  return -EINVAL;  // 参数不合法
  if (rel_job[id]->job_state == STATE_CANCELED ||
      rel_job[id]->job_state == STATE_OVER) {
    pthread_mutex_unlock(&mut_rel_job);
    return -EBUSY;  // 任务不存在或者任务已取消或者任务已结束
  }
  if (rel_job[id] == NULL) {
    pthread_mutex_unlock(&mut_rel_job);
    return -EFAULT;
  }
  rel_job[id]->job_state = STATE_CANCELED;
  pthread_mutex_unlock(&mut_rel_job);
  return id;
};

/*
 * 任务收尸
 * return == 0      成功，指定任务已终止并返回状态
 * return ==-EINVAL 失败，参数非法
 *
 */
int rel_waitjob(int id, struct rel_stat_st *jobstat) {
  pthread_mutex_lock(&mut_rel_job);
  if (id > REL_JOBMAX || id < 0) {
    pthread_mutex_unlock(&mut_rel_job);
    return -EINVAL;
  }
  if (rel_job[id] == NULL) {
    pthread_mutex_unlock(&mut_rel_job);
    return -EFAULT;
  }
  jobstat->count12 = rel_job[id]->fsm12.count;
  jobstat->count21 = rel_job[id]->fsm21.count;
  jobstat->fd1 = rel_job[id]->fd1;
  jobstat->fd2 = rel_job[id]->fd2;
  pthread_mutex_unlock(&mut_rel_job);
  return 0;
}
/*
 * 任务状态
 * return == 0      成功，指定任务状态已返回
 * return ==-EINVAL 失败，参数非法
 *
 */
int rel_statjob(int id, struct rel_stat_st *jobstat) {
  pthread_mutex_lock(&mut_rel_job);
  if (id > REL_JOBMAX || id < 0) {
    pthread_mutex_unlock(&mut_rel_job);
    return -EINVAL;
  }
  if (rel_job[id] == NULL) {
    pthread_mutex_unlock(&mut_rel_job);
    return -EFAULT;
  }
  jobstat->fd1 = rel_job[id]->fd1;
  jobstat->fd2 = rel_job[id]->fd2;
  jobstat->state = rel_job[id]->job_state;
  jobstat->count12 = rel_job[id]->fsm12.count;
  jobstat->count21 = rel_job[id]->fsm21.count;
  pthread_mutex_unlock(&mut_rel_job);
  return 0;
}