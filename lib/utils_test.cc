#include "utils.h"

#include <gtest/gtest.h>

TEST(trim_test, trim_lead_and_trail_space) {
  char string[] = "  ws string   ";
  trim(string);
  EXPECT_STREQ("ws string", string);
}
