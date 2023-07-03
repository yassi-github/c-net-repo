#include <stdio.h>
#include <unistd.h>  // lseek

#include "errorutil.h"
#include "messageutil.h"

error child_process(const char *store_name) {
  error err;
  int store_fd;
  // open
  if ((err = message_store_new(&store_fd, store_name)) != NULL) {
    return err;
  }

  // create message string to send
  message_t message_members;
  char message_string_send[MESSAGE_MAXSIZE];
  // check store is blank or not
  if (lseek(store_fd, 0L, SEEK_END) == 0) {
    // create default message
    if ((err = message_t_init(&message_members, 0, "NONE", "NONE")) != NULL) {
      return err;
    }
    if ((err = message_string_new(&message_members, message_string_send)) !=
        NULL) {
      return err;
    }
  } else {
    // read last message from store file
    if ((err = message_store_read(store_fd, -1, message_string_send)) != NULL) {
      return err;
    }
  }

  // send msg to client
  printf("send message `%s` to client.\n", message_string_send);

  // recv msg from client
  char message_string_recv[MESSAGE_MAXSIZE] = "12 3AS G>8";
  printf("recv message `%s` from client.\n", message_string_recv);

  // increase number in message
  if ((err = message_extract(&message_members, message_string_recv)) != NULL) {
    return err;
  }
  message_members.number += 1;
  char message_string_tosend[MESSAGE_MAXSIZE];
  if ((err = message_string_new(&message_members, message_string_tosend)) !=
      NULL) {
    return err;
  }

  // append message to store
  if ((err = message_store_write(store_fd, 0, message_string_tosend)) != NULL) {
    return err;
  }

  close(store_fd);

  return NULL;
}

int main() {
  const char *store_name = "test.txt";
  error err = child_process(store_name);
  if (err != NULL) {
    error_exit(err);
  }
  return 0;
}
