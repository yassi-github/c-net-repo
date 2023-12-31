#ifndef _MY_UTILS_H
#define _MY_UTILS_H 1

// ignore return value
// example:
//   ignore_retval foo()
#define ignore_retval (void)!

// we should use utils.h to use noreturn.
// because at c++, noreturn is not compat.
// but it using [[noreturn]] instead.
#ifdef __cplusplus
// noreturn compat c++
// example:
//   noreturn void foo() { exit 0; }
#define no_return [[noreturn]]
#else                     // __cplusplus
#include <stdnoreturn.h>  // noreturn
#define no_return noreturn
#endif  // __cplusplus

// = 1 mili sec in nano unit
#define MSEC 1000000

// sleep milisec
int mili_sleep(int mili_sec);

// strip string
void trim(char *string);

// #include "errorutil.h" // circular reference
typedef const char *error;

#include <stddef.h>  // size_t
// split string.
//
// example:
// ```
// int main() {
//   char str[] = "addr:port";
//   char* dest[MAX_LEN];
//   size_t split_size;
//   split(str, ":", dest, sizeof(dest) / sizeof(dest[0]), &split_size);
//   for (size_t idx = 0; idx < split_size; idx++) {
//     printf("%s\n", dest[idx]);
//   }
//   return 0;
// }
// ```
// output:
// ```
// addr
// port
// ```
error split(char *src, const char *delim, char **dest, size_t dest_len,
            size_t *split_count);

#endif
