#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// 多线程并发计算primer

#define LEFT 30000000
#define RIGHT 30000200
#define THRUM (RIGHT - LEFT + 1)

struct thr_arg_st {  // 用结构体的方法来避免竞争
  int n;
};

static void* thr_primer(void* p) {
  int mark, i = ((struct thr_arg_st*)p)->n;
  for (int j = 2; j < i / 2; j++) {
    mark = 1;
    if (i % j == 0) {
      mark = 0;
      break;
    }
  }
  if (mark) {
    printf("%d is a primer\n", i);
  }
  pthread_exit(p);  // 把指针返回用于销毁
}

int main() {
  int i, j, mark, err, thread_index = 0;
  struct thr_arg_st* p;
  void* ptfree;
  int num[THRUM];
  pthread_t tids[THRUM];
  for (i = LEFT; i <= RIGHT; i++) {
    p = malloc(sizeof(*p));
    if (p == NULL) {
      perror("malloc()");
      exit(1);
    }
    p->n = i;
    err = pthread_create(tids + thread_index, NULL, thr_primer, p);
    if (err < 0) {
      for (int count = 0; count < i - 1; count++) {
        pthread_join(tids[count], NULL);
      }
      fprintf(stderr, "pthread_create():%s\n", strerror(err));
      exit(1);
    }
    thread_index++;
  }
  for (i = 0; i < THRUM; i++) {
    pthread_join(tids[i], &ptfree);  // 收尸的时候接收它传回来的参数并且释放
    free(ptfree);
  }
  exit(0);
}