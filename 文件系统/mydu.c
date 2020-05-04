#include <glob.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define PATHSIZE 1024

// 判断路径是否成环
static bool path_noloop(const char* path) {
  // 看最后一个/后面的内容是否完整的.或..，如果是则构成死循环
  char* pos = NULL;  // 最后一个/的位置
  pos = strrchr(path, '/');
  if (pos == NULL) exit(1);  // 找不到/

  // 找到/那么的看看后面的是否.或者..
  if (strcmp(pos + 1, ".") == 0 || strcmp(pos + 1, "..") == 0) return false;
  return true;
};

// 注意目录文件结束带/非目录文件结束不带/
static int64_t mydu(const char* path) {
  // 判断当前path是目录文件还是非目录文件
  static struct stat statret;  // 优化到静态区中去，因为递归回来不再使用它
  static char nextpath[PATHSIZE];  // 下一级路径的内通,也可以优化到静态区
  glob_t globret;                  // 获取的glob结构体
  int64_t sum;                     // 总的大小
  int i;                           // for循环使用的

  if (lstat(path, &statret) < 0) {  // 当前文件的信息
    // 出现错误
    perror("lstat()");
    exit(1);
  }
  if (!S_ISDIR(statret.st_mode)) {
    // 如果是非目录，那就统计他的大小
    return statret.st_blocks;
  }
  // 如果为目录文件
  // 非隐藏文件全部找出来
  strncpy(nextpath, path, PATHSIZE);  // 将path复制到nextpath
  strncat(nextpath, "/*", PATHSIZE);  // 进入下一级目录
  printf("%s\n", nextpath);
  if ((glob(nextpath, 0, NULL, &globret) != 0) &&
      (glob(nextpath, 0, NULL, &globret) != GLOB_NOMATCH)) {  // 空文件不算错误
    // 如果出现错误
    perror("glob2()");
    exit(1);
  }

  // 把隐藏文件全部找出来
  strncpy(nextpath, path, PATHSIZE);   // 将path复制到nextpath
  strncat(nextpath, "/.*", PATHSIZE);  // 进入下一级目录
  printf("%s\n", nextpath);
  if (glob(nextpath, GLOB_APPEND, NULL, &globret) != 0) {
    // 如果出现错误
    perror("glob2()");
    exit(1);
  }

  // 递归优化  sum = 0;  // 对于每一个目录文件，都单独计算他的总大小
  sum = statret.st_blocks;

  // 递归，把上目录文件再次找他们里面的目录
  for (i = 0; i < globret.gl_pathc; i++) {
    // 如果不构成目录循环，那么再进行递归，由于.和
    // ..指向的是当前目录和父辈目录，所以会形成环
    if (path_noloop(globret.gl_pathv[i])) sum += mydu(globret.gl_pathv[i]);
  }
  // 优化递归点 sum += statret.st_blocks;  // 加上当前文件的目录文件的大小
  globfree(&globret);
  return sum;
};

// 一定会用到递归
// 两种情况 1）目录文件 2）非目录文件
int main(int argc, char* argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Usage : %s <filename>/<pathname>\n", argv[0]);
    exit(1);
  }

  printf("%lld\n", mydu(argv[1]));
  exit(0);
};