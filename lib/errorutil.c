#include "errorutil.h"

#include <stdbool.h>  // bool
#include <stdio.h>    // perror
#include <stdlib.h>   // exit
#include <string.h>   // strcmp

#include "utils.h"  // no_return

error error_new(const char *text) { return (error)text; }

bool error_is(error err, error target) { return strcmp(err, target) == 0; }

no_return void error_msg(error msg) {
  perror(msg);
  exit(1);
}
