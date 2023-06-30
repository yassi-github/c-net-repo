#ifndef _MY_WAITUTILS_H
#define _MY_WAITUTILS_H 1

// check and wait any child process exited.
// return exited_pid. if err, return -1.
// if there is no waitable process, returns 0.
int wait_exit();

#endif
