#include "qutil.h"

#include <gtest/gtest.h>

#include "errorutil.h"

extern void error_exit(error err);

TEST(qtest, NewSendRecv) {
  int q_id;
  // check create new queue success
  EXPECT_EQ(NULL, q_new(&q_id));

  const char *send_data = "testStr";
  // check push data to queue success
  EXPECT_EQ(NULL, q_push(q_id, send_data));

  char recv_data[strlen(send_data) + 1];
  // check pop data from queue success
  EXPECT_EQ(NULL, q_pop(q_id, recv_data));

  // check recv data is same with send data
  EXPECT_STREQ(recv_data, send_data);
}
