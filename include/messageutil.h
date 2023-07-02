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

// open new store file and contain fd into `_new_store_fd`.
// example:
// ```
// int main() {
//   int store_file;
//   error err = message_store_new(&store_file, "foo.txt");
//   if (err != NULL) error_exit(err)
//   char buf; read(store_file, buf, 1);
//   close(store_file);
// }
// ```
// we should close fd after use.
error message_store_new(int *_store_fd, const char *_store_name);

// read indexed data from store_fd to _read_dest.
// plus index reads from BOF,
// and minus index reads from EOF.
error message_store_read(int _store_fd, const int _data_index, char *_read_dest);

// write data from _write_src to indexed store_fd.
// plus index writes from BOF,
// and minus index writes from EOF.
// write is not insert just replace.
error message_store_write(int _store_fd, const int _data_index,
                  const char *_write_src);

// delete data on the store file
// delete: fill w/t zero.
error message_store_delete(int _store_fd, const int _data_index);

#endif
