#include <signal.h>  // kill
#include <stdio.h>   // printf,fprintf
#include <stdlib.h>  // atoi
#include <unistd.h>  // fork,close

#include "errorutil.h"  // error_exit
#include "sockutil.h"   // socket_listen,socket_accept
#include "utils.h"      // mili_sleep
#include "waitutil.h"   // wait_exit

#define TCP_PORT 20000
#define CTRL_A '\001'

// internal error messages
struct _error_msg_list {
  const char *error_fork;
  const char *error_kill_children;
};
static const struct _error_msg_list error_msg_list = {
    .error_fork = "failed to fork",
    .error_kill_children = "failed to kill child processes",
};

error child_process(int listening_socket, int accepted_socket,
                    const char *store_name) {
  printf("CHILD: start childprocess\n");
  mili_sleep(500);
  printf("CHILD: close listening socket(%d)\n", listening_socket);
  mili_sleep(500);
  printf("CHILD: open file %s\n", store_name);
  mili_sleep(500);
  printf("CHILD: lock file %s\n", store_name);
  mili_sleep(500);
  printf("CHILD: write message from file %s\n", store_name);
  mili_sleep(500);
  printf("CHILD: send message to client(socket %d)\n", accepted_socket);
  mili_sleep(500);
  printf("CHILD: recv message to client(socket %d)\n", accepted_socket);
  mili_sleep(500);
  printf("CHILD: write message to file %s\n", store_name);
  mili_sleep(500);
  printf("CHILD: unlock file %s\n", store_name);
  mili_sleep(500);
  printf("CHILD: close file %s\n", store_name);
  mili_sleep(500);
  printf("CHILD: end childprocess\n\n");
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
    return child_process(listening_socket, accepted_socket, data_file);
    // child process end
  }
  return NULL;
}

int main(int argc, char *argv[]) {
  // data store file name
  const char *data_file = argv[0];
  // listen port number
  const int port_no = (argc > 1) ? atoi(argv[1]) : TCP_PORT;

  printf(
      "[server]\n"
      "Ctrl+A to stop me.\n"
      "data file is `%s`.\n"
      "listening on port %d ...\n",
      data_file, port_no);

  error err = run_server(data_file, port_no);
  if (err != NULL) {
    error_exit(err);
  }

  return 0;
}
