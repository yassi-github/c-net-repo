#ifndef _MY_UTILS_H
#define _MY_UTILS_H 1

#include <stdio.h>
#include <stdlib.h>

extern void err_msg(char *msg) {
  perror(msg);
  exit(1);
}

#endif
