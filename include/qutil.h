#ifndef _MY_QUTIL_H
#define _MY_QUTIL_H 1

#include "errorutil.h"

// get new q_id
error q_new(int *new_q_id);

error q_pop(int q_id, char *data);

error q_push(int q_id, const char *data);

#endif
