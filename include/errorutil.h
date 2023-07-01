#ifndef _MY_ERRORUTIL_H
#define _MY_ERRORUTIL_H 1

#include <stdbool.h>  // bool

typedef const char *error;

// create error.
error errors_new(char *text);

// compare error.
// return true if same else false
bool errors_is(error err, error target);

#endif
