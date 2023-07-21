#ifndef _MY_RANDOMUTIL_H
#define _MY_RANDOMUTIL_H 1

#include <stdint.h>  // uint32_t

#include "errorutil.h"

// initialize random state.
// we can call it by self but its optional because
// we call it in the randint() if not initialized.
error init_seed();

// return random int (0 to 2^31 - 1)
error randint(int *retval);

#endif
