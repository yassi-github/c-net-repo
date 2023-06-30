#ifndef _MY_MESSAGEUTIL_H
#define _MY_MESSAGEUTIL_H 1

#define MESSAGE_MAXSIZE 100
#define ID_MAXSIZE 35

typedef struct {
  int number;
  char *id_1;
  char *id_2;
} message_t;

// _message_t に引数のメンバをセットする
// return 0 if succeed. return -1 if err.
int message_t_init(message_t *_message_t, int number, char *id_1, char *id_2);

// 構造体メンバの文字列がポインタ(char *)で、それに値を入れたい場合、
// mallocをしなければ、snprintfが適切に動いてくれない(segmentation fault になる)
// なぜかというと、ポインタが宣言されただけでは、それが指す先がまだ不明なので、
// 不明なアドレスに対して文字列の書き込みを行ってしまっているから。
// そういう不正なメモリの書き換えは、segmentation fault を引き起こす。
// ここではmallocで新たに領域を確保して、そこに引数の文字列をコピーしているので、
// この関数に渡したポインタがdropしても、これで設定した構造体の中身は無事。
// ただし使い終わったらその中身のポインタをfreeするべきなので管理が面倒になる。
// int message_t_init(message_t *_message_t, int number, char *id_1, char *id_2) {
//   char *id_1_p = (char *)malloc(ID_MAXSIZE);
//   if (id_1_p == NULL) return -1;
//   char *id_2_p = (char *)malloc(ID_MAXSIZE);
//   if (id_2_p == NULL) return -1;
//   snprintf(id_1_p, ID_MAXSIZE, "%s", id_1);
//   snprintf(id_2_p, ID_MAXSIZE, "%s", id_2);
//   _message_t->number = number;
//   _message_t->id_1 = id_1_p;
//   _message_t->id_2 = id_2_p;
//   return 0;
// }
//
// このように直接代入すれば、mallocなどしなくても動く。
// メンバの指すアドレスに対して、引数で与えられた文字列のアドレスを渡しているから。
// なのでこの関数の引数に渡したポインタがdropすると、これで設定した構造体の中身も消える。
// int message_t_init(message_t *_message_t, int number, char *id_1, char *id_2)
// {
//   _message_t->number = number;
//   _message_t->id_1 = id_1;
//   _message_t->id_2 = id_2;
// return 0;
// }

// return 0 if ok, -1 if failed.
// just print err msg to stderr not exit.
int valid(const message_t *member);

// create new message.
// we should free returned addr.
char *new_message(const message_t message_member, const int message_size);

// extract message string into message_t member.
int message_extract(message_t *_message_t, const char *message);

#endif
