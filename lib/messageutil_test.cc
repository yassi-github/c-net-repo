#include "messageutil.h"

#include <gtest/gtest.h>

#include "errorutil.h"

extern void error_msg(error err);

TEST(member_init, init) {
  message_t msg_member;
  error err = message_t_init(&msg_member, 0, "NONE", "NONE");
  if (err != NULL) {
    error_msg(err);
  }
  // defer message_t_delete(&msg_member);

  EXPECT_EQ(0, msg_member.number);
  EXPECT_STREQ("NONE", msg_member.id_1);
  EXPECT_STREQ("NONE", msg_member.id_2);
}

TEST(member_extract, extract) {
  message_t ext_msg_member;
  error err = message_extract(&ext_msg_member, "0 NONE NONE");
  if (err != NULL) {
    error_msg(err);
  }
  // defer message_t_delete(&ext_msg_member);

  EXPECT_EQ(0, ext_msg_member.number);
  EXPECT_STREQ("NONE", ext_msg_member.id_1);
  EXPECT_STREQ("NONE", ext_msg_member.id_2);
}

TEST(new_string, string) {
  message_t msg_member;
  error err = message_t_init(&msg_member, 0, "NONE", "NONE");
  if (err != NULL) {
    error_msg(err);
  }

  char *message_string = message_string_new(&msg_member, MESSAGE_MAXSIZE);
  // defer message_string_delete(message_string);

  EXPECT_STREQ("0 NONE NONE", message_string);
}
