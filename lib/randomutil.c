#define _DEFAULT_SOURCE
#include "randomutil.h"

#include <stdlib.h>
#include <time.h>
#include <unistd.h>  // getpid

#include "errorutil.h"

// internal error messages
struct _error_msg_list {
  const char *error_timespec;
  const char *error_random_r;
};
static const struct _error_msg_list error_msg_list = {
    .error_timespec = "failed to get timespec",
    .error_random_r = "failed to random",
};

static char statebuf[64];

error init_seed(unsigned int *retval) {
  struct timespec ts;
  if (timespec_get(&ts, TIME_UTC) == 0) {
    return error_new(error_msg_list.error_timespec);
  }
  *retval = ts.tv_nsec ^ ts.tv_sec ^ getpid();
  return NULL;
}

error randint(unsigned int seed, int *retval) {
  struct random_data state;

  // mutex_lock(&mutex);

  // generate random seed
  initstate_r(seed, statebuf, sizeof(statebuf), &state);

  if (random_r(&state, retval) == -1) {
    return error_new(error_msg_list.error_random_r);
  }

  // mutex_unlock(&mutex);

  return NULL;
}

#undef _DEFAULT_SOURCE
