#include <stdio.h>
#include <syslog.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>

#include "server_conf.h"
#include "thread_channel.h"
#include "medialib.h"
#include <proto.h>

struct thread_channel_entry_st {
  chnid_t chnid;
  pthread_t tid;
};

static int tid_nextpos = 0;
struct thread_channel_entry_st thr_channel[CHANNELNUM];

static void* thr_channel_snder(void* ptr) {
  struct msg_channel_st* sbufp;
  struct medialib_listentry_st* entry = ptr;
  ssize_t len;
  sbufp = malloc(MSG_CHANNEL_MAX);
  if (sbufp == NULL) {
    syslog(LOG_ERR, "malloc():%s\n", strerror(errno));
    exit(1);
  }
  sbufp->chnid = entry->chnid;
  while (1) {
    len = media_readchn(entry->chnid, sbufp->data, MAX_DATA);
    if (sendto(serversd, sbufp, len + sizeof(chnid_t), 0, (void*)&sndaddr,
               sizeof(sndaddr)) < 0) {
      syslog(LOG_ERR, "thread channel [%d] sendto():%s", entry->chnid,
             strerror(errno));
      exit(1);
    }
    sched_yield();
  }
  pthread_exit(NULL);
};

// 创建频道线程
int thread_channel_create(struct medialib_listentry_st* ptr) {
  int err;
  err = pthread_create(&thr_channel[tid_nextpos].tid, NULL, thr_channel_snder,
                       ptr);
  if (err) {
    syslog(LOG_WARNING, "pthread_create():%s", strerror(err));
    return err;
  }
  thr_channel[tid_nextpos].chnid = ptr->chnid;
  tid_nextpos++;
  return 0;
};

// join频道线程
int thread_channel_destory(struct medialib_listentry_st* ptr) {
  for (int i = 0; i < CHANNELNUM; i++) {
    if (thr_channel[i].chnid == ptr->chnid) {
      if (pthread_cancel(thr_channel[i].tid) < 0) {
        syslog(LOG_ERR, "pthread_cancel():the thread of channel %d",
               ptr->chnid);
        return -ESRCH;
      }
      pthread_join(thr_channel[i].tid, NULL);
      thr_channel[i].chnid = -1;
      return 0;
    }
  }
  return -EINVAL;
};

// join所有频道线程
int thread_channel_destoryall() {
  for (int i = 0; i < CHANNELNUM; i++) {
    if (thr_channel[i].chnid > 0) {
      if (pthread_cancel(thr_channel[i].tid) < 0) {
        syslog(LOG_ERR, "pthread_cancel():the thread of channel %d",
               thr_channel[i].chnid);
        return -ESRCH;
      }
      pthread_join(thr_channel[i].tid, NULL);
      thr_channel[i].chnid = -1;
    }
  }
  return 0;
}