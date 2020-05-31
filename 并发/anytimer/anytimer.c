#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <signal.h>

#include "anytimer.h"
enum { STATE_RUNNING = 1, STATE_CANCELED, STATE_OVER };
static int init = 0;  // 是否已经初始化
static struct sigaction alarm_sa_save;
struct at_job_st {
  int job_state;       // 任务状态
  int sec;             // 几秒钟之后进行任务
  int time_remain;     // 递减时间
  at_jobfunc_t* jobp;  // 函数指针
  void* arg;           // 参数
  int repeat;          // 周期性任务
};

struct at_job_st* jobs[JOB_MAX];  // 任务数组

static void alarm_action(int s, siginfo_t* infop, void* unused) {
  if (infop->si_code == SI_USER) {
    return;
  }  // 如果不是内核信号就不做任何处理
  for (int i = 0; i < JOB_MAX; i++) {
    if (jobs[i] != NULL) {
      if (jobs[i]->job_state == STATE_RUNNING) {
        jobs[i]->time_remain--;
        if (jobs[i]->time_remain == 0) {          // 倒计时结束
          jobs[i]->jobp(jobs[i]->arg);            // 执行函数
          if (jobs[i]->repeat == 1) {             // 如果是周期性任务
            jobs[i]->time_remain = jobs[i]->sec;  // 重置时钟周期
          } else                                  // 非周期性任务
            jobs[i]->job_state = STATE_OVER;      // 任务完毕
        }
      }
    }
  }
}

static void module_unload(void) {
  // 卸载模块
  struct itimerval itv;
  itv.it_interval.tv_sec = 0;
  itv.it_interval.tv_usec = 0;
  itv.it_value.tv_usec = 0;
  itv.it_value.tv_sec = 0;
  setitimer(ITIMER_REAL, &itv, NULL);
  if (sigaction(SIGALRM, &alarm_sa_save, NULL) < 0) {
    perror("sigaction()");
    exit(1);
  }
}

static void module_load(void) {
  // 模块加载
  // sigaction设置
  struct sigaction sa;
  sa.sa_sigaction = alarm_action;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_SIGINFO;
  if (sigaction(SIGALRM, &sa, &alarm_sa_save) < 0) {
    perror("sigaction()");
    exit(1);
  }

  struct itimerval itv;
  itv.it_interval.tv_sec = 1;  // 周期
  itv.it_interval.tv_usec = 0;
  itv.it_value.tv_sec = 1;  // 基准
  itv.it_value.tv_usec = 0;
  if (setitimer(ITIMER_REAL, &itv, NULL) < 0) {
    perror("setitimer()");
    exit(1);
  }
  atexit(module_unload);
};

int get_free_pos(void) {
  for (int i = 0; i < JOB_MAX; i++) {
    if (jobs[i] == NULL) return i;
  }
  return -1;
};

int at_waitjob(int id) {
  if (id < 0 || id > (JOB_MAX - 1) || jobs[id] == NULL) return -EINVAL;
  if (jobs[id]->job_state == STATE_OVER) return -EBUSY;
  if (jobs[id]->repeat == 1) return -EBUSY;  // 周期性任务不能回收资源
  while (jobs[id]->job_state == STATE_RUNNING) pause();  // 确保任务完成或者取消
  if (jobs[id]->job_state == STATE_OVER ||
      jobs[id]->job_state == STATE_CANCELED) {
    // 收尸
    free(jobs[id]);
    jobs[id] = NULL;
  }
  return 0;
};

int at_canceljob(int id) {
  if (id < 0 || id > (JOB_MAX - 1) || jobs[id] == NULL) return -EINVAL;
  if (jobs[id]->job_state == STATE_CANCELED) return -ECANCELED;
  if (jobs[id]->job_state == STATE_OVER) return -EBUSY;

  jobs[id]->job_state = STATE_CANCELED;
  return 0;
};

int at_addjob(int sec, at_jobfunc_t* jobp, void* arg) {
  if (sec < 0) return -EINVAL;
  int pos;
  struct at_job_st* me;
  if (init == 0) {
    module_load();
    init = 1;
  }
  pos = get_free_pos();
  if (pos < 0) return -ENOSPC;  // 数组满
  me = malloc(sizeof(*me));
  if (me == NULL) return -ENOMEM;  // 堆空间满

  me->job_state = STATE_RUNNING;
  me->sec = sec;
  me->time_remain = me->sec;
  me->jobp = jobp;
  me->arg = arg;
  me->repeat = 0;
  jobs[pos] = me;
  return pos;
};

int at_addjob_repeat(int sec, at_jobfunc_t* jobp, void* arg) {
  if (sec < 0) return -EINVAL;
  int pos;
  struct at_job_st* me;
  if (init == 0) {
    module_load();
    init = 1;
  }
  pos = get_free_pos();
  if (pos < 0) return -ENOSPC;
  me = malloc(sizeof(*me));
  if (me == NULL) return -ENOMEM;

  me->job_state = STATE_RUNNING;
  me->sec = sec;
  me->time_remain = me->sec;
  me->jobp = jobp;
  me->arg = arg;
  me->repeat = 1;  // 周期性函数
  jobs[pos] = me;
  return pos;
};