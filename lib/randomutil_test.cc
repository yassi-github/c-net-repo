#include "randomutil.h"

#include <gtest/gtest.h>

#include "errorutil.h"

extern void error_exit(error err);

TEST(randomTest, seedRand) {
  EXPECT_EQ(NULL, init_seed());
  int random_int1, random_int2, random_int3;
  EXPECT_EQ(NULL, randint(&random_int1));
  EXPECT_EQ(NULL, randint(&random_int2));
  EXPECT_EQ(NULL, randint(&random_int3));
  EXPECT_FALSE((random_int1 == random_int2) && (random_int1 == random_int3));
}
