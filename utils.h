#include <stdio.h>
#include <stdlib.h>

extern void err_msg(char *msg) {
  perror(msg);
  exit(1);
}
