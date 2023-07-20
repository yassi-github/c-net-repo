#include "randomutil.h"

#include <gtest/gtest.h>

#include "errorutil.h"

extern void error_exit(error err);

TEST(randomTest, seedRand) {
  unsigned int seed;
  EXPECT_EQ(NULL, init_seed(&seed));
  int random_int;
  EXPECT_EQ(NULL, randint(seed, &random_int));
}
