#ifndef _MY_RANDOMUTIL_H
#define _MY_RANDOMUTIL_H 1

#include "errorutil.h"

error init_seed(unsigned int *retval);

error randint(unsigned int seed, int *retval);

#endif
