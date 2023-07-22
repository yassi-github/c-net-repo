#define _GNU_SOURCE  // enable MAP_ANONYMOUS at mman.h

#include <sched.h>     // CLONE_*
#include <signal.h>    // kill
#include <stdio.h>     // printf,fprintf
#include <stdlib.h>    // atoi
#include <string.h>    // strlen
#include <sys/file.h>  //flock
#include <sys/mman.h>  // mmap
#include <unistd.h>    // fork,close,flock

#include "errorutil.h"    // error_exit
#include "messageutil.h"  // message_t
#include "sockutil.h"     // socket_listen,socket_accept
#include "utils.h"        // mili_sleep
#include "waitutil.h"     // wait_exit

#define TCP_PORT 20000
#define CTRL_A '\001'
#define STACK_SIZE (1024 * 1024)  // Stack size for cloned child

// internal error messages
struct _error_msg_list {
  const char *error_fork;
  const char *error_mmap;
  const char *error_clone;
  const char *error_kill_children;
  const char *error_write_sock;
  const char *error_read_sock;
};
static const struct _error_msg_list error_msg_list = {
    .error_fork = "failed to fork",
    .error_mmap = "failed to mmap",
    .error_clone = "failed to clone",
    .error_kill_children = "failed to kill child processes",
    .error_write_sock = "cannot write to socket",
    .error_read_sock = "cannot read from socket",
};

error do_child_process(int listening_socket, int accepted_socket,
                       const char *store_name) {
  // close unused listen socket
  close(listening_socket);

  error err;
  int store_fd;
  // open
  if ((err = message_store_new(&store_fd, store_name)) != NULL) {
    return err;
  }

  flock(store_fd, LOCK_EX);

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

  flock(store_fd, LOCK_UN);

  close(store_fd);

  return NULL;
}

struct child_process_args {
  int listening_socket;
  int accepted_socket;
  const char *store_name;
};
int child_process(void *args) {
  struct child_process_args *arg = args;
  int listening_socket = arg->listening_socket;
  int accepted_socket = arg->accepted_socket;
  const char *store_name = arg->store_name;
  error err = do_child_process(listening_socket, accepted_socket, store_name);
  if (err != NULL) {
    close(accepted_socket);
    error_exit(err);
  }
  return 0;  // child process end
}

// continue waiting until all child processes exited.
// exited childlen are terminated by wait.
error terminate_all_childlen(int child_process_counter) {
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

    // clone
    char *stack = mmap(NULL, STACK_SIZE, PROT_READ | PROT_WRITE,
                       MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK, -1, 0);
    if (stack == MAP_FAILED) {
      return error_new(error_msg_list.error_mmap);
    }
    char *stackTop = stack + STACK_SIZE;
    struct child_process_args args = {
        .listening_socket = listening_socket,
        .accepted_socket = accepted_socket,
        .store_name = data_file,
    };
    // CLONE_VM: 呼び出し元のプロセスと子プロセスは同じメモリー空間で実行される
    int pid = clone(child_process, stackTop, CLONE_VM | SIGCHLD, (void *)&args);
    if (pid == -1) {
      close(accepted_socket);
      close(listening_socket);
      err = terminate_all_childlen(child_process_counter);
      if (err != NULL) {
        return err;
      }
      return error_new(error_msg_list.error_clone);
    }

    // parent falls through to here
    child_process_counter++;
    // close connected socket
    close(accepted_socket);
    // terminate childlen already exited.
    err = terminate_exited_processes(child_process_counter);
    if (err != NULL) {
      return err;
    }
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
