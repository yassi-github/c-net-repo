#include "utils.h"

#include <ctype.h>   // memmove
#include <string.h>  // strlen,memmove
#include <time.h>    // timespec,nanosleep

#include "errorutil.h"

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

// split string.
error split(char *src, const char *delim, char **dest, size_t dest_len,
            size_t *split_count) {
  size_t split_counter = 0;

  char *word = strtok(src, delim);
  while (word != NULL) {
    split_counter++;

    if (split_counter > dest_len) {
      return (error) "split: too many words to dest";
    }

    dest[split_counter - 1] = word;

    word = strtok(NULL, delim);
  }

  *split_count = split_counter;
  return NULL;
}
