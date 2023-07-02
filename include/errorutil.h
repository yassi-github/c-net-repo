#ifndef _MY_ERRORUTIL_H
#define _MY_ERRORUTIL_H 1

#include <stdbool.h>  // bool

#include "utils.h"  // no_return

// error can contain NULL
#define NULL ((void *)0)

typedef const char *error;

// create error.
error error_new(const char *text);

// compare error.
// return true if same else false
bool error_is(error err, error target);

// show error msg and exit
no_return void error_msg(error msg);

#endif
