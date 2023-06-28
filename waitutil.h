#include <sys/wait.h>
#include <bits/types/siginfo_t.h>

// check and wait any child process exited.
// return exited_pid. if err, return -1.
// if there is no waitable process, returns 0.
extern int wait_exit() {
  siginfo_t infop;
  int err = waitid(P_ALL, 0, &infop, WEXITED | WNOHANG);
  if (err != 0) {
    return -1;
  }
  return infop.si_pid;
}
