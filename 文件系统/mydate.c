#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

/*
 * -y:year
 * -m:month
 * -d:day
 * -H:hour
 * -M:minute
 * -S:second
 */

#define TIMESTRSIZE 1024
#define FMTSTRSIZE 1024

int main(int argc, char *argv[]) {
  time_t stamp;   // 时间戳
  struct tm *tm;  // 时间结构体
  char timestr[TIMESTRSIZE];
  int c;                    // 选项
  char fmtstr[FMTSTRSIZE];  // 存储选项
  FILE *fp = stdout;
  fmtstr[0] = '\0';

  if ((stamp = time(NULL)) == -1) {
    perror("time()");
    exit(1);
  }
  if ((tm = localtime(&stamp)) == NULL) {
    // 判断是否获取结构体成功
    perror("location()");
    exit(1);
  }

  while (1) {
    // 分析option,不包含-。getopt的选项可以带一个参数
    c = getopt(argc, argv, "H:MSy:md");
    if (c == -1) {
      printf(
          "Usage : %s <-option> <-option> ... <non-option> <non_option> ...\n",
          argv[0]);
      break;  // 如果参数选尽了，或者用法出错，那就break
    }

    switch (c) {
      case 'H':
        if (strcmp(optarg, "12") == 0)  // 确定时制
          strncat(fmtstr, "%I(%P) ", FMTSTRSIZE);
        else if (strcmp(optarg, "24") == 0)
          strncat(fmtstr, "%H ", FMTSTRSIZE);
        else
          fprintf(stderr, "Invalid argument of H");
        break;
      case 'M':
        strncat(fmtstr, "%M ", FMTSTRSIZE);  // 选项参数存到一个串里面
        break;
      case 'S':
        strncat(fmtstr, "%S ", FMTSTRSIZE);  // 选项参数存到一个串里面
        break;
      case 'y':
        if (strcmp(optarg, "2") == 0)  // 确定年制
          strncat(fmtstr, "%y ", FMTSTRSIZE);
        else if (strcmp(optarg, "4") == 0)
          strncat(fmtstr, "%Y ", FMTSTRSIZE);
        else
          fprintf(stderr, "Invalid argument of H");
        break;
      case 'm':
        strncat(fmtstr, "%m ", FMTSTRSIZE);  // 选项参数存到一个串里面
        break;
      case 'd':
        strncat(fmtstr, "%d ", FMTSTRSIZE);  // 选项参数存到一个串里面
        break;
      default:
        printf(
            "Usage : %s <-option> <-option> ... <non-option> <non_option> "
            "...\n",
            argv[0]);
        break;
    }
  }

  // 选项参数处理完毕，处理非选项参数
  // 非选项传参
  while (optind < argc) {
    fp = fopen(argv[optind], "w");
    if (fp == NULL) {
      perror("fopen()");
      fp = stdout;  // 如果失败就让他往终端上输出。
    }
    ++optind;
  }

  strncat(fmtstr, "\n", FMTSTRSIZE);

  // 用strftime的话已经帮你换算好了(不用+1900)
  strftime(timestr, TIMESTRSIZE, fmtstr, tm);
  fputs(timestr, fp);
  fflush(fp);
  if (fp != stdout) fclose(fp);
  exit(0);
}