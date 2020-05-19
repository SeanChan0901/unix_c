#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <errno.h>

#define LINESIZE 1024
#define FNAME "/tmp/out"
#define PROCNUM 20

// 多进程并发版本

static pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;  // 互斥量
static int semid;

// 竞争问题，同时竞争一个文件的写,实际上是一个数据写了20次

static void P() {
  struct sembuf op;
  op.sem_num = 0;
  op.sem_op = -1;
  op.sem_flg = 0;
  while (semop(semid, &op, 1) < 0) {
    if (errno != EINTR || errno != EAGAIN) {
      perror("semop()");
      exit(1);
    }
  }
}

static void V() {
  struct sembuf op;
  op.sem_num = 0;
  op.sem_op = 1;
  op.sem_flg = 0;
  while (semop(semid, &op, 1) < 0) {
    if (errno != EINTR || errno != EAGAIN) {
      perror("semop()");
      exit(1);
    }
  }
}

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
  P();
  fgets(linebuf, LINESIZE, fp);
  fseek(fp, 0, SEEK_SET);
  fprintf(fp, "%d\n",
          (atoi(linebuf) + 1));  // 从位置0开始写会依次覆盖掉后面的内容
  fflush(fp);
  V();
  fclose(fp);  // 防止意外解锁
  return NULL;
}

int main() {
  pid_t pid;
  semid = semget(IPC_PRIVATE, 1, 0600);
  if (semid < 0) {
    perror("semget()");
    exit(1);
  }
  semctl(semid, 0, SETVAL, 1);
  if (semid < 0) {
    perror("semctl()");
    exit(1);
  }
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
  semctl(semid, 0, IPC_RMID);
  exit(0);
}