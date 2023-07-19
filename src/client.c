#include <fcntl.h>    // fcntl
#include <stdbool.h>  // bool
#include <stdio.h>    // fileno,fprintf
#include <stdlib.h>   // exit,atoi
#include <string.h>   // strlen
#include <unistd.h>   // read,write,close

#include "errorutil.h"    // error_exit
#include "messageutil.h"  // MESSAGE_MAXSIZE
#include "sockutil.h"     // socket_connect
#include "utils.h"        // mili_sleep,no_return

#define TCP_PORT 20000
#define ID_MAXSIZE 35
#define CTRL_A '\001'
#define END_STRING "\nEnd.\n"

// internal error messages
struct _error_msg_list {
  const char *error_write_sock;
  const char *error_read_sock;
};
static const struct _error_msg_list error_msg_list = {
    .error_write_sock = "cannot write to socket",
    .error_read_sock = "cannot read from socket",
};

// ending message and close socket
no_return void clean_up(int socket_fd) {
  // stdout end message
  printf("%s", END_STRING);

  // deferred close
  close(socket_fd);
  exit(0);
}

// write socket data to sending_char.
error write_socket(int socket_fd, char *sending_char) {
  int wrote_size = write(socket_fd, sending_char, strlen(sending_char) + 1);
  if (wrote_size == -1) {
    return error_msg_list.error_write_sock;
  }
  return NULL;
}
// read socket data to sending_char.
error read_socket(int socket_fd, char *receiving_char) {
  int read_size = read(socket_fd, receiving_char, MESSAGE_MAXSIZE);
  if (read_size == -1) {
    // error to read
    return error_msg_list.error_read_sock;
  }
  if (receiving_char[0] == CTRL_A) {
    clean_up(socket_fd);
    // --- unreachable ---
  }
  return NULL;
}

no_return void usage() {
  fprintf(stderr,
          "Usage: ./client <server_name:[port:=%d]> <id(less than %d bytes)> "
          "<loop_max>\n",
          TCP_PORT, ID_MAXSIZE);
  exit(1);
}

int main(int argc, char *argv[]) {
  if (argc < 4) {
    usage();
  }
  error err;
  char *addr_port[2];
  size_t split_count;
  err = split(argv[1], ":", addr_port, sizeof(addr_port) / sizeof(addr_port[0]),
              &split_count);
  if (err != NULL) {
    usage();
  }
  const char *addr = addr_port[0];
  const int port = (split_count == 2) ? atoi(addr_port[1]) : TCP_PORT;
  const char *id = argv[2];
  if (strlen(id) > ID_MAXSIZE) {
    usage();
  }
  const int loop_max = atoi(argv[3]);

  char receiving_char[MESSAGE_MAXSIZE];
  char sending_char[MESSAGE_MAXSIZE];

  for (int i = 0; i < loop_max; i++) {
    // connect to server
    int socket_fd;
    err = socket_connect(&socket_fd, addr, port);
    if (err != NULL) {
      error_exit(err);
    }
    // defer close(socket_fd)

    // try to receive data from socket to receiving_char
    memset(receiving_char, '\0', MESSAGE_MAXSIZE);
    err = read_socket(socket_fd, receiving_char);
    if (err != NULL) {
      error_exit(err);
    }

    // stdout received char
    printf("%s\n", receiving_char);

    // create new message to send
    message_t message_struct_recv;
    err = message_extract(&message_struct_recv, receiving_char);
    if (err != NULL) {
      error_exit(err);
    }
    message_t message_struct_send;
    err = message_t_init(&message_struct_send, message_struct_recv.number + 1,
                         id, message_struct_recv.id_2);
    if (err != NULL) {
      error_exit(err);
    }
    err = message_string_new(&message_struct_send, sending_char);
    if (err != NULL) {
      error_exit(err);
    }

    // try to send sending_char to socket
    err = write_socket(socket_fd, sending_char);
    if (err != NULL) {
      error_exit(err);
    }

    // disconnect
    close(socket_fd);  // deferred

    // wait a bit
    mili_sleep(1000);
  }
  return 0;
}
