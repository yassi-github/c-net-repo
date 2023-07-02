#ifndef _MY_MESSAGEUTIL_H
#define _MY_MESSAGEUTIL_H 1

#include "errorutil.h"  // error

// max size of message string
#define MESSAGE_MAXSIZE 100
// max size of message_t id members
#define ID_MAXSIZE 35

typedef struct {
  int number;
  char *id_1;
  char *id_2;
} message_t;

// _message_t のメンバ領域を確保し引数の値をコピーする
error message_t_init(message_t *_message_t, int number, const char *id_1,
                     const char *id_2);

// extract message string into message_t member.
// _message_t のメンバ領域を新たに確保し、message_string の値をコピーする
error message_extract(message_t *_message_t, const char *_message_string);

// free up _message_t member
void message_t_delete(const message_t *_message_t);

// create new message.
char *message_string_new(const message_t *_message_t, const int message_size);

// free up message string
void message_string_delete(char *_message_string);

#endif
