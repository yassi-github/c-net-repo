#ifndef _MY_UTILS_H
#define _MY_UTILS_H 1

// bool type
#define bool _Bool
#define false ((bool)+0)
#define true ((bool)+1)

// ignore return value
// example:
//   ignore foo()
#define ignore (void)!

void err_msg(char *msg);

// = 1 mili sec in nano unit
#define MSEC 1000000

// sleep milisec
int mili_sleep(int mili_sec);

// strip string
void trim(char *string);

#endif
