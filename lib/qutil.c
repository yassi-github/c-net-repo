#include "qutil.h"

#include <stdio.h>  //snprintf
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <time.h>

#include "errorutil.h"

// internal error messages
struct _error_msg_list {
  const char *error_q_init;
  const char *error_malloc;
};
static const struct _error_msg_list error_msg_list = {
    .error_q_init = "failed to init Q",
    .error_malloc = "failed to malloc",
};

error q_init(msgQ *q, long type, char *text, int text_size) {
  if (snprintf(q->mtext, text_size, "%s", text) < 0) {
    return error_msg_list.error_q_init;
  }
  q->mtype = type;
  return NULL;
}

// error q_pop(msgQ *q) {
//   return NULL;
// }

// error q_add(msgQ *q) {
//   return NULL;
// }
