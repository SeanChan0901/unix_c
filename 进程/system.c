#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
  system("date +%s > /tmp/out");
  exit(0);
}