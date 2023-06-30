#include <string.h> // strcmp

#include "utils.h"  // bool

typedef char *error;

error errors_new(char *text) { return text; }

bool errors_is(error err, error target) { return strcmp(err, target) == 0; }
