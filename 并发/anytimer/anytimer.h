#ifndef _ANYTIMER_H
#define _ANYTIMER_H

#define JOB_MAX 1024

typedef void at_jobfunc_t(void*);

int at_addjob(int sec, at_jobfunc_t* jobp, void* arg);
/*
 * return >= 0 成功，返回任务ID
 * return == -EINVAL 失败，参数非法
 * return == -ENOSPC 失败，数组满
 * return == -ENOMEN 失败，内存空间不足
 */

int at_addjob_repeat(int sec, at_jobfunc_t* jobp, void* arg);
/*
 * 周期性任务
 * return >= 0 成功，返回任务ID
 * return == -EINVAL 失败，参数非法
 * return == -ENOSPC 失败，数组满
 * return == -ENOMEN 失败，内存空间不足
 */

int at_canceljob(int id);
/*
 * return  == 0 成功，指定任务取消
 * return == -EINVAL 失败，参数非法
 * return == -EBUSY 失败，指定任务已完成
 * return == -ECANCELED 失败，指定任务重复取消
 */

int at_waitjob(int id);
/*
 * return == 0 成功，指定任务释放
 * return == -EINVAL 失败，参数非法
 * return == -EBUSY 失败，指定任务为周期性任素
 */

#endif