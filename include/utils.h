#ifndef _MY_UTILS_H
#define _MY_UTILS_H 1

// ignore return value
// example:
//   ignore_retval foo()
#define ignore_retval (void)!

// = 1 mili sec in nano unit
#define MSEC 1000000

// sleep milisec
int mili_sleep(int mili_sec);

// strip string
void trim(char *string);

#endif
