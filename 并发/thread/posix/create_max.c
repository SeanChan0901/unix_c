#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// 10m一个栈的话可以有多少个线程

static void* func(void* p) {
  int n = (int)p;
  fprintf(stdout, "[%d]this is a pthread\n", n);
  pthread_exit(NULL);
}

int main() {
  int err;
  pthread_t tid;
  pthread_attr_t attr;  // 线程的属性
  pthread_attr_init(&attr);
  err = pthread_attr_setstacksize(&attr, 1024 * 1024);  // 栈大小为1m
  if (err) {
    fprintf(stderr, "pthread_attr_setstacksize():%s\n", strerror(err));
    exit(1);
  }
  for (int i = 0;; i++) {
    err = pthread_create(&tid, &attr, func, (void*)i);
    if (err) {
      fprintf(stderr, "pthread_create()failed:%s\n", strerror(err));
      break;  // 注意这里是允许内存泄漏的，只是测试代码，真的写的时候千万不要这么做
    }
  }
  pthread_attr_destroy(&attr);
  exit(0);
}