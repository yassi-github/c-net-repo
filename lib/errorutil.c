#include "errorutil.h"

#include <string.h>  // strcmp

#include <stdbool.h>  // bool

error errors_new(char *text) { return (error)text; }

bool errors_is(error err, error target) { return strcmp(err, target) == 0; }
