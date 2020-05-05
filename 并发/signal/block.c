#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static void int_handler(int s) { write(1, "!", 1); }

int main() {
  // 5*换行 一行的*号中是忽略掉信号
  sigset_t set, old_set, save_set;
  sigemptyset(&set);  // 集合设为空集
  sigaddset(&set, SIGINT);
  signal(SIGINT, int_handler);
  sigprocmask(SIG_UNBLOCK, &set, &save_set);  // 旧状态的保存方法。
  for (int j = 0; j < 1000; j++) {
    sigprocmask(SIG_BLOCK, &set, &old_set);  // 另种形式的旧状态保存
    for (int i = 0; i < 5; i++) {
      write(1, "*", 1);
      sleep(1);
    }
    write(1, "\n", 1);
    sigprocmask(SIG_SETMASK, &old_set, NULL);  // 恢复
  }
  sigprocmask(SIG_SETMASK, &save_set, NULL);  // 恢复旧状态
  exit(0);
}

// 具体两种方法