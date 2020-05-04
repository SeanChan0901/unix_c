#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define FNAME "/tmp/out"

// 使puts("hello!")不输出到屏幕上
// 提示，重定向输出流！！

// 宏观编程思想，把程序当模块来写

int main() {

  /* 方法一
  int fd;  // 文件描述符
  close(1);  // 关闭标准输出流
  // 文件描述结构体优先占据最小的违背未被使用的文件描述符
  fd = open(FNAME, O_WRONLY | O_CREAT | O_TRUNC, 0600);
  if (fd < 0) {
    perror("open()");
    exit(1);
  }
  puts("hello!");
  exit(0);
  */

  // 考虑原子操作的问题用： dup2
  int fd;  // 文件描述符
  // 文件描述结构体优先占据最小的违背未被使用的文件描述符
  fd = open(FNAME, O_WRONLY | O_CREAT | O_TRUNC, 0600);
  if (fd < 0) {
    perror("open()");
    exit(1);
  }

  // 以下代码不具有原子性，可能会出问题
  // 1：如果fd就是1，那么关闭了自己。
  // 2:如果close1之后另外一个进程，open了一个文件，那么他会被分配到1
  // close(1);
  // dup(fd);

  // 上两个操作原子化！
  // 如果fd和1相同，那么dup不做任何操作
  dup2(fd, 1);

  if (fd != 1) close(fd);

  puts("hello!");
  exit(0);
};