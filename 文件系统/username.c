#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <uuid/uuid.h>

int main(int argc, char *argv[]) {
  struct passwd *pwdline;
  if (argc < 2) {
    fprintf(stderr, "Usage : %s <filename>\n", argv[0]);
    exit(1);
  }
  if ((pwdline = getpwuid(atoi(argv[1]))) == NULL) {
    perror("getpwuid)");
    exit(1);
  }

  // 0号用户是root
  puts(pwdline->pw_name);
  exit(0);
}