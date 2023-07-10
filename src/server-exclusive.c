#include <signal.h>    // kill
#include <stdio.h>     // printf,fprintf
#include <stdlib.h>    // atoi
#include <string.h>    // strlen
#include <sys/file.h>  //flock
#include <unistd.h>    // fork,close,flock

#include "errorutil.h"    // error_exit
#include "messageutil.h"  // message_t
#include "sockutil.h"     // socket_listen,socket_accept
#include "utils.h"        // mili_sleep
#include "waitutil.h"     // wait_exit

#define TCP_PORT 20000
#define CTRL_A '\001'

// internal error messages
struct _error_msg_list {
  const char *error_fork;
  const char *error_kill_children;
  const char *error_write_sock;
  const char *error_read_sock;
};
static const struct _error_msg_list error_msg_list = {
    .error_fork = "failed to fork",
    .error_kill_children = "failed to kill child processes",
    .error_write_sock = "cannot write to socket",
    .error_read_sock = "cannot read from socket",
};

error child_process(int listening_socket, int accepted_socket,
                    const char *store_name) {
  // close unused listen socket
  close(listening_socket);

  error err;
  int store_fd;
  // open
  if ((err = message_store_new(&store_fd, store_name)) != NULL) {
    return err;
  }

#ifdef FLOCK
  flock(store_fd, LOCK_EX);
  puts("locked");
#endif

  // create message string to send
  message_t message_members;
  char message_string_tosend[MESSAGE_MAXSIZE];
  // check store is blank or not
  if (lseek(store_fd, 0L, SEEK_END) == 0) {
    // create default message
    if ((err = message_t_init(&message_members, 0, "NONE", "NONE")) != NULL) {
      return err;
    }
    if ((err = message_string_new(&message_members, message_string_tosend)) !=
        NULL) {
      return err;
    }
  } else {
    // read last message from store file
    if ((err = message_store_read(store_fd, -1, message_string_tosend)) !=
        NULL) {
      return err;
    }
  }

  // send msg to client
  ssize_t wrote_size = write(accepted_socket, message_string_tosend,
                             strlen(message_string_tosend));
  if (wrote_size == -1) {
    return error_new(error_msg_list.error_write_sock);
  }

  // recv msg from client
  char message_string_torecv[MESSAGE_MAXSIZE];
  ssize_t read_size =
      read(accepted_socket, message_string_torecv, MESSAGE_MAXSIZE);
  if (read_size == -1) {
    return error_new(error_msg_list.error_read_sock);
  }

  // increase number in message
  if ((err = message_extract(&message_members, message_string_torecv)) !=
      NULL) {
    return err;
  }
  message_members.number += 1;
  char message_string_tostore[MESSAGE_MAXSIZE];
  if ((err = message_string_new(&message_members, message_string_tostore)) !=
      NULL) {
    return err;
  }

  // save message to store
  if ((err = message_store_write(store_fd, 0, message_string_tostore)) !=
      NULL) {
    return err;
  }

#ifdef FLOCK
  flock(store_fd, LOCK_UN);
  puts("unlocked");
#endif

  close(store_fd);

  return NULL;
}

// continue waiting until all child processes exited.
// exited childlen are terminated by wait.
error terminate_all_childlen(int child_process_counter) {
  // broadcast sigkill to own pgid
  if (kill(0, 9) == -1) {
    return error_new(error_msg_list.error_kill_children);
  }
  while (child_process_counter > 0) {
    /* wait terminating child process */
    if (wait_exit() > 0) child_process_counter--;
  }
  return NULL;
}

// check there is already exited process.
// exited process is terminated by wait.
error terminate_exited_processes(int child_process_counter) {
  int exited_pid;
  while (child_process_counter > 0 && (exited_pid = wait_exit()) > 0) {
    fprintf(stderr, "Terminated child process: %d\n", exited_pid);
    child_process_counter--;
  }
  return NULL;
}

error run_server(const char *data_file, const int port_no) {
  error err;
  // open socket and ready to listen
  int listening_socket;
  err = socket_listen(&listening_socket, port_no);
  if (err != NULL) {
    return error_new(err);
  }

  int child_process_counter = 0;
  while (1) {
    // accept
    int accepted_socket = -1;
    err = socket_accept(&accepted_socket, listening_socket);
    if (err != NULL) {
      close(listening_socket);
      // FIXME: retry
      err = terminate_all_childlen(child_process_counter);
      if (err != NULL) {
        return err;
      }
      return error_new(err);
    }

    // fork
    int pid = fork();
    if (pid == -1) {
      close(accepted_socket);
      close(listening_socket);
      err = terminate_all_childlen(child_process_counter);
      if (err != NULL) {
        return err;
      }
      return error_new(error_msg_list.error_fork);
    }

    // parent process start
    if (pid > 0) {
      child_process_counter++;
      // close connected socket
      close(accepted_socket);
      // terminate childlen already exited.
      err = terminate_exited_processes(child_process_counter);
      if (err != NULL) {
        return err;
      }
      continue;
    }
    // parent process end

    // child process start
    err = child_process(listening_socket, accepted_socket, data_file);
    if (err != NULL) {
      close(accepted_socket);
      return err;
    }
    // child process end
  }
  return NULL;
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr,
            "Usage: ./server-exclusive <storeFile> [portNo(defaults 20000)]\n");
    exit(1);
  }
  // data store file name
  const char *data_file = argv[1];
  // listen port number
  const int port_no = (argc > 2) ? atoi(argv[2]) : TCP_PORT;

  printf(
      "[server]\n"
      "Ctrl+A to stop me.\n"
      "data file is `%s`.\n"
      "listening on port %d ...\n",
      data_file, port_no);

  error err = run_server(data_file, port_no);
  if (err != NULL) {
    fprintf(stderr, "%s\n", err);
    exit(1);
  }

  return 0;
}
