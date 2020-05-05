#ifndef _ANYTIMER_H
#define _ANYTIMER_H

#define JOB_MAX 1024

typedef void at_jobfunc_t(void*);

// 看视频提示完成anytimer
int at_addjob(int sec, at_jobfunc_t* jobp, void* arg);

#endif