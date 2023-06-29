#include <stdio.h>   // fprintf
#include <string.h>  // strlen
#include <unistd.h>  // fork,write,read,close,getpid

#include "sockutil.h"  // init_socket,accept_socket
#include "utils.h"     // err_msg
#include "waitutil.h"  // wait_exit

#define TCP_PORT 20000
#define CTRL_A '\001'

// continue waiting until all child processes exited.
// exited childlen are terminated by wait.
void terminate_all_childlen(int child_process_counter) {
  // broadcast sigkill to own pgid
  if (kill(0, 9) == -1) {
    err_msg("server: failed to kill child processes");
  }
  while (child_process_counter > 0) {
    /* wait terminating child process */
    if (wait_exit() > 0) child_process_counter--;
  }
}

// check there is already exited process.
// exited process is terminated by wait.
void terminate_exited_processes(int child_process_counter) {
  int exited_pid;
  while (child_process_counter > 0 && (exited_pid = wait_exit()) > 0) {
    fprintf(stderr, "Terminated child process: %d\n", exited_pid);
    child_process_counter--;
  }
}

// create return string data
char *create_return_data(const char *head_char) {
  const int data_length = 10;
  char *return_data = (char *)malloc(sizeof(char) * data_length);
  sprintf(return_data, "%c [%02X] ", *head_char, *head_char & 0xff);
  return return_data;
}

// child process
int child_process(int child_process_counter, int listening_socket,
                  int accepted_socket) {
  // close unused listen socket
  close(listening_socket);

  int pid = getpid();
  fprintf(stderr, "\nI am child process %d\n", pid);

  while (1) {
    char _head_char;
    // read head 1 byte from socket
    if (read(accepted_socket, &_head_char, 1) != 1) {
      // err
      fprintf(stderr, "Illegal termination\n");
      close(accepted_socket);
      terminate_all_childlen(child_process_counter);
      return 1;
    }
    // const cast
    const char head_char = _head_char;
    // end when ^A received
    if (head_char == CTRL_A) {
      fprintf(stderr, "Finished child (%d)\n", pid);
      // terminate_all_childlen(child_process_counter, accepted_socket);
      close(accepted_socket);
      // terminate_exited_processes(child_process_counter);
      return 0;
    }

    char *return_data = create_return_data(&head_char);
    // defer free(return_data)

    // send data
    ssize_t err = write(accepted_socket, return_data, strlen(return_data));
    if (err == -1) {
      err_msg("server: cannot write to socket");
    }
    printf("child (%d) sended %s\n", pid, return_data);

    // deferred
    free(return_data);
  }
  // --- unreachable ---
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

  // open socket and ready to listen
  int listening_socket = init_socket(port_no);

  // number of child processes
  int child_process_counter = 0;
  while (1) {
    // terminate childlen already exited.
    terminate_exited_processes(child_process_counter);

    // accept
    int accepted_socket = accept_socket(listening_socket);
    if (accepted_socket == -1) {
      close(listening_socket);
      fprintf(stderr, "server: failed to accept.\n");
      // FIXME: retry
      terminate_all_childlen(child_process_counter);
      return 1;
    }

    // fork
    int pid = fork();
    if (pid == -1) {
      close(accepted_socket);
      close(listening_socket);
      fprintf(stderr, "server: failed to fork.\n");
      terminate_all_childlen(child_process_counter);
      return 1;
    }

    // parent closes socket fd and wait new connections.
    if (pid > 0) {
      close(accepted_socket);
      child_process_counter++;
      continue;
    }

    // child process start
    return child_process(child_process_counter, listening_socket,
                         accepted_socket);
    // child process end
  }
  // --- unreachable ---
}
