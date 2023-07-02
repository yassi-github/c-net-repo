#include "messageutil.h"

#include <stdio.h>   // snprintf
#include <stdlib.h>  // malloc
#include <string.h>  // memset

#include "errorutil.h"  // error

// == private ==

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
    error_msg(err);
  }

  // message string must fill with zero
  char *message_string = (char *)calloc(MESSAGE_MAXSIZE, sizeof(char));

  snprintf(message_string, message_size, "%d %s %s", _message_t->number,
           _message_t->id_1, _message_t->id_2);

  return message_string;
}

void message_string_delete(char *_message_string) { free(_message_string); }
