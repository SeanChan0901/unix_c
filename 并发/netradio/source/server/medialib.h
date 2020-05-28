#ifndef MEDIALIB_H__
#define MEDIALIB_H__

#include <sys/types.h>
#include <site_type.h>

struct medialib_listentry_st {
  chnid_t chnid;
  char *desc;
};

// 获取媒体库
int medialib_getchnlist(struct medialib_listentry_st **, int *);

int medialob_freechnlist(struct medialib_listentry_st **);

ssize_t media_readchn(chnid_t, void *, size_t);

#endif