#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include "relayer.h"

/*
 * return >=0       成功，返回当前任务ID
 *        ==-EINVAL 失败，参数非法
 *        ==-ENOSPC 失败，任务数组满
 *        ==-ENOMEM 失败，malloc失败
 *
 */
int rel_addjob(int fd1, int fd2);

/*
 * return >=0       成功，指定任务成功取消
 *        ==-EINVAL 失败，参数非法
 *        ==-EBUSY  失败，指定任务无法重复取消
 */
int rel_canceljob(int jobID);

/*
 * 任务收尸
 * return == 0      成功，指定任务已终止并返回状态
 * return ==-EINVAL 失败，参数非法
 *
 */
int rel_waitjob(int id, struct rel_stat_st*);

/*
 * 任务收尸
 * return == 0      成功，指定任务状态已返回
 * return ==-EINVAL 失败，参数非法
 *
 */
int rel_statjob(int id, struct rel_stat_st*);