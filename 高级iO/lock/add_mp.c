#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define LINESIZE 1024
#define FNAME "/tmp/out"
#define PROCNUM 20

// 多进程并发版本

static pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;  // 互斥量

// 竞争问题，同时竞争一个文件的写,实际上是一个数据写了20次

static void* func_add(void* p) {
  FILE* fp;
  int fd;
  char linebuf[LINESIZE];
  fp = fopen(FNAME, "r+");
  if (fp == NULL) {
    perror("fopen()");
    exit(1);
  }
  fd = fileno(fp);
  if (fd < 0) {
    perror("fileno()");
    exit(1);
  }
  //  锁加在临界区
  lockf(fd, F_LOCK, 0);
  fgets(linebuf, LINESIZE, fp);
  fseek(fp, 0, SEEK_SET);
  fprintf(fp, "%d\n", (atoi(linebuf) + 1));
  fflush(fp);
  lockf(fd, F_ULOCK, 0);
  fclose(fp);  // 防止意外解锁
  return NULL;
}

int main() {
  pid_t pid;
  for (int i = 0; i < PROCNUM; i++) {
    pid = fork();
    if (pid < 0) {
      perror("fork()");
      exit(1);
    }
    if (pid == 0) {
      // child
      func_add(NULL);
      exit(0);
    } 
  }
  // parent
  for (int i = 0; i < PROCNUM; i++) wait(NULL);
  exit(0);
}