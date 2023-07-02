#include "errorutil.h"

#include <fcntl.h>
#include <gtest/gtest.h>
#include <sys/stat.h>
#include <sys/types.h>

TEST(error_new_concat_test, concat_error_message) {
  ignore_retval open("/etc/shadow", O_RDONLY);
  EXPECT_STREQ("foobar: Permission denied", error_new("foobar"));
}

TEST(error_is_test, is_any_message) {
  ignore_retval open("/etc/shadow", O_RDONLY);
  EXPECT_TRUE(error_is(error_new("foobar"), (error) "foobar: Permission denied"));
  EXPECT_FALSE(error_is(error_new("foobar"), (error) "---"));
}
