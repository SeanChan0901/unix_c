#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <syslog.h>
#include <string.h>
#include <pthread.h>
#include <proto.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include "thread_list.h"
#include "server_conf.h"

static pthread_t tid_list;
static int nr_lsit_entry;
static struct medialib_listentry_st *list_entry;  // 媒体库信息（包含所有频道）

static void *thread_list(void *p) {
  int size;
  int totalsize;
  ssize_t ret;
  struct msg_list_st *entrylistp;   // 节目单包
  struct msg_listentry_st *entryp;  // 节目单中单个条目的信息

  totalsize = sizeof(chnid_t);
  for (int i = 0; i < nr_lsit_entry; i++) {
    totalsize += (sizeof(struct msg_listentry_st) + strlen(list_entry[i].desc));
  }
  entrylistp = malloc(totalsize);
  if (entrylistp == NULL) {
    syslog(LOG_ERR, "malloc():%s.", strerror(errno));
    exit(1);
  }

  entrylistp->chnid = LISTCHNID;  // 节目单频道号
  entryp = entrylistp->entry;     // 节目单信息

  for (int i = 0; i < nr_lsit_entry; i++) {
    // 将媒体库的信息加载到节目单中去
    size = (sizeof(struct msg_listentry_st) + strlen(list_entry[i].desc));
    entryp->chnid = list_entry[i].chnid;
    entryp->len = htons(size);
    strcpy(entryp->desc, list_entry[i].desc);
    entryp = (void *)(((char *)entryp) + size);
  }

  while (1) {
    ret = sendto(serversd, entrylistp, totalsize, 0, (void *)&sndaddr,
                 sizeof(sndaddr));
    if (ret < 0) {
      syslog(LOG_WARNING, "sendto(serversd,entlistp...):%s", strerror(errno));
    } else {
      syslog(LOG_DEBUG, "sendto(serversd entrylistp...):succeed.");
    }
    sleep(1);
  }
}

// 创建节目单线程
int thread_list_create(struct medialib_listentry_st *listp, int nr_entry) {
  int err;
  list_entry = listp;
  nr_lsit_entry = nr_entry;
  err = pthread_create(&tid_list, NULL, thread_list, NULL);
  if (err) {
    syslog(LOG_ERR, "pthread_create():%s.", strerror(errno));
    return -1;
  }
  return 0;
}

// 销毁节目单线程(join)
int thread_list_destory(void) {
  pthread_cancel(tid_list);
  pthread_join(tid_list, NULL);
  return 0;
}