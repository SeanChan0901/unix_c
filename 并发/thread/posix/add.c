#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define LINESIZE 1024
#define FNAME "/tmp/out"
#define THRUM 20

static pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;  // 互斥量

// 竞争问题，同时竞争一个文件的写,实际上是一个数据写了20次

static void* thr_add(void* p) {
  FILE* fp;
  char linebuf[LINESIZE];
  fp = fopen(FNAME, "r+");
  if (fp == NULL) {
    perror("fopen()");
    exit(1);
  }
  //  锁加在临界区
  pthread_mutex_lock(&mut);
  fgets(linebuf, LINESIZE, fp);
  fseek(fp, 0, SEEK_SET);
  fprintf(fp, "%d\n", (atoi(linebuf) + 1));
  fclose(fp);
  pthread_mutex_unlock(&mut);
  pthread_exit(NULL);
}

int main() {
  pthread_t tids[THRUM];
  int err;
  for (int i = 0; i < THRUM; i++) {
    err = pthread_create(tids + i, NULL, thr_add, NULL);
    if (err < 0) {
      fprintf(stderr, "pthread_create()%s\n", strerror(err));
      exit(1);
    }
  }

  for (int i = 0; i < THRUM; i++) {
    pthread_join(tids[i], NULL);
  }
  pthread_mutex_destroy(&mut);
  exit(0);
}