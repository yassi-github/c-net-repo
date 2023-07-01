#include <fcntl.h>    // fcntl
#include <stdbool.h>  // bool
#include <stdio.h>    // fileno,fprintf
#include <stdlib.h>   // exit,atoi
#include <string.h>   // strlen
#include <unistd.h>   // read,write,close

#include "errorutil.h"  // error_msg
#include "sockutil.h"   // connect_server
#include "utils.h"      // mili_sleep,no_return

#define TCP_PORT 20000
#define CTRL_A '\001'
#define END_STRING "\nEnd.\n"

// ending message and close socket
no_return void clean_up(int socket_fd) {
  // stdout end message
  // fprintf じゃいかんのか？
  // +1 は何？？
  ssize_t err = write(fileno(stdout), END_STRING, strlen(END_STRING) + 1);
  if (err == -1) {
    error_msg("client: cannot write to stdout");
  }

  // deferred close
  close(socket_fd);
  exit(0);
}

// read stdin data to receiving_char.
// return:
//   -1 if failed to read,
//   0 if succeed.
int read_stdin(int socket_fd, char *sending_char) {
  if (read(fileno(stdin), sending_char, 1) == 1) {
    // end when ^A received after sending sending_char
    if (*sending_char == CTRL_A) {
      // keep trying to write sending_char to socket until success
      while (write(socket_fd, sending_char, 1) != 1)
        ;
      clean_up(socket_fd);
      // --- unreachable ---
    }
    return 0;
  }
  return -1;
}

// read socket data to sending_char.
// return:
//   -1 if failed to read,
//   0 if succeed.
int read_socket(int socket_fd, char *receiving_char) {
  if (read(socket_fd, receiving_char, 1) == 1) {
    // end when ^A received
    if (*receiving_char == CTRL_A) {
      clean_up(socket_fd);
      // --- unreachable ---
    }
    return 0;
  }
  return -1;
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Usage: client serv_addr [port_no:=%d]\n", TCP_PORT);
    exit(1);
  }

  int port_no = (argc > 2) ? atoi(argv[2]) : TCP_PORT;
  int socket_fd = connect_server(argv[1], port_no);
  // defer close(socket_fd)

  // set file statis flag to O_NONBLOCK.
  // operate to these fd do not block caller.
  // so read will fail, not wait data when sock buf is blank.
  // and write will immediately done, not wait enough blank.
  fcntl(fileno(stdin), F_SETFL, O_NONBLOCK);
  fcntl(fileno(stdout), F_SETFL, O_NONBLOCK);
  fcntl(socket_fd, F_SETFL, O_NONBLOCK);

  // expect input if ready_to_read else send data
  bool ready_to_read = true;
  bool ready_to_write = false;
  char receiving_char, sending_char;
  int err;

  while (true) {
    if (ready_to_read) {
      // we wanna read data.
      // try to get stdin to sending_char
      err = read_stdin(socket_fd, &sending_char);
      if (err != -1) {
        ready_to_read = false;
      }
    }

    if (!ready_to_read) {
      // we wanna send data.
      // try to send sending_char to socket
      if (write(socket_fd, &sending_char, 1) == 1) {
        ready_to_read = true;
      }
    }

    if (!ready_to_write) {
      // we wanna receive data.
      // try to receive data from socket to receiving_char
      err = read_socket(socket_fd, &receiving_char);
      if (err != -1) {
        ready_to_write = true;
      }
    }

    if (ready_to_write) {
      // we wanna write data.
      // try to write sending_char to stdout
      if (write(fileno(stdout), &receiving_char, 1) == 1) {
        ready_to_write = false;
      }
    }

    // For save CPU usage
    mili_sleep(10);
  }
  // --- unreachable ---
}
