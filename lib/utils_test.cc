#include "utils.h"

#include <gtest/gtest.h>

TEST(trim_test, trim_lead_and_trail_space) {
  char string[] = "  ws string   ";
  trim(string);
  EXPECT_STREQ("ws string", string);
}

TEST(split_test, split_lead_and_trail_space) {
  char string[] = "  ws string   ";
  char* dest[8];
  size_t split_size;
  split(string, " ", dest, sizeof(dest) / sizeof(dest[0]), &split_size);
  EXPECT_EQ(2, split_size);
  EXPECT_STREQ("ws", dest[0]);
  EXPECT_STREQ("string", dest[1]);
}

TEST(split_failtest, split_result_no_space) {
  char string[] = "  ws string   ";
  char* dest[1];
  size_t split_size;
  error err = split(string, " ", dest, sizeof(dest) / sizeof(dest[0]), &split_size);
  EXPECT_STREQ("split: too many words to dest", err);
}
