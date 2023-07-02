#include "messageutil.h"

#include <fcntl.h>     // open
#include <stdio.h>     // snprintf
#include <stdlib.h>    // malloc
#include <string.h>    // memset
#include <sys/stat.h>  // open
#include <unistd.h>    // lseek,read,write,close

#include "errorutil.h"  // error

// == private ==

// internal error messages
struct _error_msg_list {
  const char *error_seek;
  const char *error_read;
  const char *error_write;
};
static const struct _error_msg_list error_msg_list = {
    .error_seek = "failed to allocate memory to output previous error message",
    .error_read = "cannot read",
    .error_write = "cannot write",
};

static error message_t_valid(const message_t *_message_t) {
  if (&(_message_t->number) == NULL) {
    return error_new("number must be set");
  }
  if (strlen(_message_t->id_1) > ID_MAXSIZE) {
    return error_new("id_1 too big");
  }
  if (strlen(_message_t->id_2) > ID_MAXSIZE) {
    return error_new("id_2 too big");
  }
  return NULL;
}


// continue reading till whole size read
static error continue_reading(int _store_fd, const int _data_index,
                              char *_read_dest, const int _want_read_length) {
  char read_buf[_want_read_length];
  ssize_t read_length = read(_store_fd, read_buf, _want_read_length);
  switch (read_length) {
    case -1:
      // read error.
      // fallthrough
    case 0:
      // read failed.
      // read offset was EOF or _want_read_length was 0.
      return error_new(error_msg_list.error_read);
    default:
      // it may read is not complete but its not error
      // because it could caused by sigint or pipe,term read.
      if (read_length < _want_read_length) {
        int remain_read_length = _want_read_length - read_length;
        char trailing_read_buf[remain_read_length];
        error err = continue_reading(_store_fd, _data_index, trailing_read_buf,
                                     remain_read_length);
        if (err != NULL) {
          return err;
        }
        // copy string from (read_but + trailing_read_buf) to _read_dest
        snprintf(_read_dest, _want_read_length + remain_read_length, "%s%s",
                 read_buf, trailing_read_buf);
        return NULL;
      }
      // successfly read whole data.
      // copy string from read_buf to _read_dest.
      snprintf(_read_dest, _want_read_length, "%s", read_buf);
      return NULL;
  }
  // --- unreachable ---
}

// continue writing till whole size wrote
static error continue_writing(int _store_fd, const int _data_index,
                              const char *_write_src,
                              const int _want_write_length) {
  char write_buf[_want_write_length];
  snprintf(write_buf, _want_write_length, "%s", _write_src);
  ssize_t wrote_length = write(_store_fd, write_buf, _want_write_length);
  switch (wrote_length) {
    case -1:
      // write error.
      // fallthrough
    case 0:
      // write failed or _want_write_length was 0.
      return error_new(error_msg_list.error_write);
    default:
      // it may write is not complete but its not error
      // because it could caused by sigint or pipe write.
      if (wrote_length < _want_write_length) {
        int remain_write_length = _want_write_length - wrote_length;
        // shift to remove leading alreasy wrote string
        memmove(write_buf, &(write_buf[wrote_length]),
                strlen(&(write_buf[wrote_length])));
        error err = continue_writing(_store_fd, _data_index, write_buf,
                                     remain_write_length);
        if (err != NULL) {
          return err;
        }
        return NULL;
      }
      return NULL;
  }
  // --- unreachable ---
}

// == public ==

// _message_t に引数のメンバをセットする
// return 0 if succeed. return -1 if err.
//
// 構造体メンバの文字列がポインタ(char *)で、それに値を入れたい場合、
// mallocをしなければ、snprintfが適切に動いてくれない(segmentation fault になる)
// なぜかというと、ポインタが宣言されただけでは、それが指す先がまだ不明なので、
// 不明なアドレスに対して文字列の書き込みを行ってしまっているから。
// そういう不正なメモリの書き換えは、segmentation fault を引き起こす。
// ここではmallocで新たに領域を確保して、そこに引数の文字列をコピーしているので、
// この関数に渡したポインタがdropしても、これで設定した構造体の中身は無事。
// ただし使い終わったらその中身のポインタをfreeするべきなので管理が面倒になる。
error message_t_init(message_t *_message_t, int number, const char *id_1,
                     const char *id_2) {
  char *id_1_p = (char *)malloc(ID_MAXSIZE);
  if (id_1_p == NULL) return error_new("message_t_init: failed to malloc");
  char *id_2_p = (char *)malloc(ID_MAXSIZE);
  if (id_2_p == NULL) return error_new("message_t_init: failed to malloc");
  snprintf(id_1_p, ID_MAXSIZE, "%s", id_1);
  snprintf(id_2_p, ID_MAXSIZE, "%s", id_2);
  _message_t->number = number;
  _message_t->id_1 = id_1_p;
  _message_t->id_2 = id_2_p;
  return NULL;
}

// このように直接代入すれば、mallocなどしなくても動く。
// メンバの指すアドレスに対して、引数で与えられた文字列のアドレスを渡しているから。
// なのでこの関数の引数に渡したポインタがdropすると、これで設定した構造体の中身も消える。
// でもすぐに消えるわけではないので保持されているように見えてしまう。
// int message_t_init(message_t *_message_t, int number, char *id_1, char *id_2)
// {
//   _message_t->number = number;
//   _message_t->id_1 = id_1;
//   _message_t->id_2 = id_2;
// return 0;
// }

// extract message string into message_t member.
error message_extract(message_t *_message_t, const char *_message_string) {
  int number;
  char id_1[ID_MAXSIZE];
  char id_2[ID_MAXSIZE];
  sscanf(_message_string, "%d %s %s", &number, id_1, id_2);

  return message_t_init(_message_t, number, id_1, id_2);
}

void message_t_delete(const message_t *_message_t) {
  free(_message_t->id_1);
  free(_message_t->id_2);
}

// create new message.
// we should free returned addr.
char *message_string_new(const message_t *_message_t, const int message_size) {
  error err = message_t_valid(_message_t);
  if (err != NULL) {
    error_exit(err);
  }

  // message string must fill with zero
  char *message_string = (char *)calloc(MESSAGE_MAXSIZE, sizeof(char));

  snprintf(message_string, message_size, "%d %s %s", _message_t->number,
           _message_t->id_1, _message_t->id_2);

  return message_string;
}

void message_string_delete(char *_message_string) { free(_message_string); }

// open new store file.
error message_store_new(int *_new_store_fd, const char *_store_name) {
  const int open_mode = 0644;
  *_new_store_fd = open(_store_name, O_RDWR | O_CREAT, open_mode);
  if (*_new_store_fd == -1) {
    return error_new("store util: failed to open store file");
  }
  return NULL;
}

// read indexed data from store_fd to _read_dest.
// plus index reads from BOF,
// and minus index reads from EOF.
error message_store_read(int _store_fd, const int _data_index,
                         char *_read_dest) {
  off_t offset;
  if (_data_index < 0) {
    offset = lseek(_store_fd, _data_index * MESSAGE_MAXSIZE, SEEK_END);
  } else {
    offset = lseek(_store_fd, _data_index * MESSAGE_MAXSIZE, SEEK_SET);
  }
  if (offset == -1) {
    return error_new("store_read: failed to lseek");
  }

  error err =
      continue_reading(_store_fd, _data_index, _read_dest, MESSAGE_MAXSIZE);
  if (err != NULL) {
    return err;
  }

  return NULL;
}

// write data from _write_src to indexed store_fd.
// plus index writes from BOF,
// and minus index writes from EOF.
// write is not insert just replace.
error message_store_write(int _store_fd, const int _data_index,
                          const char *_write_src) {
  off_t offset;
  if (_data_index < 0) {
    offset = lseek(_store_fd, _data_index * MESSAGE_MAXSIZE, SEEK_END);
  } else {
    offset = lseek(_store_fd, _data_index * MESSAGE_MAXSIZE, SEEK_SET);
  }
  if (offset == -1) {
    return error_new("store_write: failed to lseek");
  }

  error err =
      continue_writing(_store_fd, _data_index, _write_src, MESSAGE_MAXSIZE);
  if (err != NULL) {
    return err;
  }

  return NULL;
}

// delete data on the store file
// delete: fill w/t zero.
error message_store_delete(int _store_fd, const int _data_index) {
  char zero[MESSAGE_MAXSIZE];
  memset(zero, '\0', MESSAGE_MAXSIZE);
  // TODO: chg behavior zero fill to shift actual data
  error err = message_store_write(_store_fd, _data_index, zero);
  if (err != NULL) {
    return err;
  }
  return NULL;
}
