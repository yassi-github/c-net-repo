#include "messageutil.h"

#include <gtest/gtest.h>
#include <stdio.h>   // remove
#include <unistd.h>  // access

#include "errorutil.h"

extern void error_exit(error err);

TEST(member_init, init) {
  message_t msg_member;
  error err = message_t_init(&msg_member, 0, "NONE", "NONE");
  if (err != NULL) {
    error_exit(err);
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
    error_exit(err);
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
    error_exit(err);
  }

  char message_string[MESSAGE_MAXSIZE];
  err = message_string_new(&msg_member, message_string);
  if (err != NULL) {
    error_exit(err);
  }

  EXPECT_STREQ("0 NONE NONE", message_string);
}

TEST(store_crud, testtxt) {
  error err;
  int test_store_file;
  const char *fname = "test.txt";

  if ((err = message_store_new(&test_store_file, fname)) != NULL) {
    error_exit(err);
  }
  // file exist
  EXPECT_EQ(0, access(fname, F_OK));

  const char wdata[MESSAGE_MAXSIZE] = "10 test data";
  // write to EOF (append)
  if ((err = message_store_write(test_store_file, 0, wdata)) != NULL) {
    error_exit(err);
  }
  // no error occured
  EXPECT_EQ(NULL, err);

  char rdata[MESSAGE_MAXSIZE];
  // read last data
  if ((err = message_store_read(test_store_file, -1, rdata)) != NULL) {
    error_exit(err);
  }
  // read data is wrote data
  EXPECT_STREQ(wdata, rdata);

  if ((err = message_store_delete(test_store_file, -1)) != NULL) {
    error_exit(err);
  }
  // file size is same but data is filled by zero
  char zero[MESSAGE_MAXSIZE];
  memset(zero, '\0', MESSAGE_MAXSIZE);
  EXPECT_EQ(1 * MESSAGE_MAXSIZE, lseek(test_store_file, 0L, SEEK_END));
  char rdata_deleted[MESSAGE_MAXSIZE];
  if ((err = message_store_read(test_store_file, -1, rdata_deleted)) != NULL) {
    error_exit(err);
  }
  EXPECT_STREQ(zero, rdata_deleted);

  close(test_store_file);
  remove(fname);
}
