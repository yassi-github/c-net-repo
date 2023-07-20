#ifndef _MY_QUTIL_H
#define _MY_QUTIL_H 1

#include <sys/msg.h>

#include "errorutil.h"

// typedef struct msgbuf msgQ;
typedef struct {
  long mtype;
  char mtext[1];
} msgQ;

error q_init(msgQ *q, long type, char *text, int text_size);

// error q_pop(msgQ *q);

// error q_add(msgQ *q);

#endif
