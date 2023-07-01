#ifndef _MY_ERRORUTIL_H
#define _MY_ERRORUTIL_H 1

#include <stdbool.h>      // bool
#include <stdnoreturn.h>  // noreturn

typedef const char *error;

// create error.
error error_new(const char *text);

// compare error.
// return true if same else false
bool error_is(error err, error target);

// show error msg and exit
noreturn void error_msg(error msg);

#endif
