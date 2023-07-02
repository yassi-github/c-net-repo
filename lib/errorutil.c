// for use XSI-compliant `strerror_r`
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200112L
#endif
#ifdef _GNU_SOURCE
#undef _GNU_SOURCE
#endif

#include "errorutil.h"

#include <errno.h>    // errno
#include <stdbool.h>  // bool
#include <stdio.h>    // perror
#include <stdlib.h>   // exit
#include <string.h>   // strcmp,strerror_r

#include "utils.h"  // no_return

// from <string.h>
// XSI-compliant: (_POSIX_C_SOURCE >= 200112L) && ! _GNU_SOURCE
extern int strerror_r(int errnum, char *buf, size_t buflen);
// internal error messages
struct _error_msg_list {
  const char *error_new_aloc_mem;
  // const char *error_is_strcmp;
};
static const struct _error_msg_list error_msg_list = {
    .error_new_aloc_mem =
        "error_new: failed to allocate memory to output previous error message",
    // .error_is_strcmp = "error_is: failed to strcmp",
};

// concat text and previous errno message.
error error_new(const char *text) {
  const int last_errno = errno;

  // initial size. will scaled up.
  // Note: In glibc-2.7, the longest error message string is 50 characters,
  // but non-English can inclease its size so we must be able to scaling it up.
  int err_str_len = 64;
  char *err_str = (char *)malloc(err_str_len);
  if (err_str == NULL) {
    return error_msg_list.error_new_aloc_mem;
  }
  // scale up allocate size till contain whole error message
  while (strerror_r(last_errno, err_str, err_str_len) == -1 &&
         errno == ERANGE) {
    // inclease size
    err_str_len += 64;
    err_str = (char *)realloc(err_str, err_str_len);
    if (err_str == NULL) {
      return error_msg_list.error_new_aloc_mem;
    }
  }

  // concat text and error string
  // +2 for size of ": "
  const int err_text_len = strlen(text) + 2 + err_str_len;
  char *error_text = (char *)malloc(err_text_len);
  snprintf(error_text, err_text_len, "%s: %s", text, err_str);

  // clean up
  free(err_str);

  return (error)error_text;
}

bool error_is(error err, error target) { return strcmp(err, target) == 0; }

no_return void error_exit(error msg) {
  perror(msg);
  exit(1);
}
