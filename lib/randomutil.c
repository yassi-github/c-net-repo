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
  const char *error_need_init_seed;
};
static const struct _error_msg_list error_msg_list = {
    .error_timespec = "failed to get timespec",
    .error_random_r = "failed to random",
    .error_need_init_seed = "failed to random: need to init seed first",
};

static char statebuf[128];
static struct random_data rand_data;
static bool seeded = false;

error init_seed() {
  struct timespec ts;
  if (timespec_get(&ts, TIME_UTC) == 0) {
    return error_new(error_msg_list.error_timespec);
  }

  // generate random seed
  rand_data.state = NULL;
  initstate_r(ts.tv_nsec ^ ts.tv_sec ^ getpid(), statebuf, 128, &rand_data);

  seeded = true;
  return NULL;
}

error randint(int *retval) {
  if (!seeded) {
    init_seed();
  }
  if (random_r(&rand_data, retval) == -1) {
    return error_new(error_msg_list.error_random_r);
  }

  // mutex_unlock(&mutex);

  return NULL;
}

#undef _DEFAULT_SOURCE
