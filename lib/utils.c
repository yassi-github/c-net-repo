#include "utils.h"

#include <ctype.h>   // memmove
#include <stdio.h>   // perror
#include <stdlib.h>  // exit
#include <string.h>  // strlen,memmove
#include <time.h>    // timespec,nanosleep

void err_msg(char *msg) {
  perror(msg);
  exit(1);
}

int mili_sleep(int mili_sec) {
  int msec = mili_sec % 1000;
  int sec = mili_sec / 1000;
  struct timespec sleep_nanosec = {.tv_sec = (long int)sec,
                                   .tv_nsec = (long int)msec * MSEC};
  return nanosleep(&sleep_nanosec, NULL);
}

void trim(char *string) {
  char *local_string = string;
  int string_length = strlen(local_string);

  // trim leading white space
  while (local_string[0] != '\0' && isspace(local_string[0]))
    ++local_string, --string_length;
  // trim trailing white space
  while (string_length > 0 && isspace(local_string[string_length - 1]))
    local_string[--string_length] = '\0';

  memmove(string, local_string, string_length + 1);
}
