#include <stdio.h>
#include <stdlib.h>
#include <glob.h>
#include <syslog.h>
#include "medialib.h"
#include "mytbf.h"
#include "server_conf.h"
#include <proto.h>

#define PATHSIZE 1024

struct channel_context_st {
  chnid_t chnid;
  char *desc;
  glob_t mp3golb;
  int pos;
  int fd;
  off_t offset;
  mytbf_t *tbf;
};

static struct channel_context_st channel[MAXCHNID + 1];

int medialib_getchnlist(struct medialib_listentry_st **result, int *resnum) {
  char *path[PATHSIZE];
  glob_t globres;  // 解析结果
  int num = 0;     // 解析出来的目录
  struct medialib_listentry_st *ptr;
  struct channel_context_st *res;
  for (int i = MINCHNID; i <= MAXCHNID; i++) {
    channel[i].chnid = -1;  // 未启用
  }
  snprintf(path, PATHSIZE, "%s/*", server_conf.media_dir);
  if (glob(path, 0, NULL, &globres)) {
    return -1;
  }
  ptr = malloc(sizeof(struct medialib_listentry_st) * globres.gl_pathc);
  if (ptr == NULL) {
    syslog(LOG_ERR, "malloc() error.");
    exit(1);
  }
  for (int i = 0; i < globres.gl_pathc; i++) {
    // globres.gl_pathv[i] -> "/var/media/..."
    path2entry(globres.gl_pathv[i]);
    num++;
  }
  *result = ;
  *resnum = num;
}

int medialob_freechnlist(struct medialib_listentry_st **);

ssize_t media_readchn(chnid_t, void *, size_t);