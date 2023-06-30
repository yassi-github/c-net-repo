#ifndef _MY_MESSAGEUTIL_H
#define _MY_MESSAGEUTIL_H 1

#include <stdio.h>   // snprintf
#include <stdlib.h>  // malloc
#include <string.h>  // memset

#define MESSAGE_MAXSIZE 100
#define ID_MAXSIZE 35

// void pr(char *msg, int msg_size) {
//   for (int i = 0; i < msg_size; i++) {
//     printf("%d: %x(%c)\n", i, msg[i], msg[i]);
//   }
//   printf("\n");
// }

typedef struct {
  int number;
  char *id_1;
  char *id_2;
} message_t;

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
int message_t_init(message_t *_message_t, int number, char *id_1, char *id_2) {
  char *id_1_p = (char *)malloc(ID_MAXSIZE);
  if (id_1_p == NULL) return -1;
  char *id_2_p = (char *)malloc(ID_MAXSIZE);
  if (id_2_p == NULL) return -1;
  snprintf(id_1_p, ID_MAXSIZE, "%s", id_1);
  snprintf(id_2_p, ID_MAXSIZE, "%s", id_2);
  _message_t->number = number;
  _message_t->id_1 = id_1_p;
  _message_t->id_2 = id_2_p;
  return 0;
}

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
int valid(const message_t *member) {
  if (strlen(member->id_1) > ID_MAXSIZE) {
    fprintf(stderr, "id_1 too big");
    return -1;
  }
  if (strlen(member->id_2) > ID_MAXSIZE) {
    fprintf(stderr, "id_2 too big");
    return -1;
  }
  return 0;
}

// create new message.
// we should free returned addr.
char *new_message(const message_t message_member, const int message_size) {
  if (valid(&message_member) != 0) {
    exit(1);
  }

  // char zero[message_size];
  // memset(zero, 0, message_size);

  char *message = (char *)malloc(sizeof(char) * message_size);

  // char message[message_size];
  // snprintf(message, message_size, "%d %s %s%s", message_member.number,
  //          message_member.id_1, message_member.id_2, zero);
  snprintf(message, message_size, "%d %s %s", message_member.number,
           message_member.id_1, message_member.id_2);

  // strncpy(message_ret, message, message_size);
  return message;
}

// extract message string into message_t member.
int message_extract(message_t *_message_t, const char *message) {
  int number;
  char id_1[ID_MAXSIZE];
  char id_2[ID_MAXSIZE];
  sscanf(message, "%d %s %s", &number, id_1, id_2);

  // (void)!message;
  // message_t message_member = { .number = number, .id_1 = id_1, .id_2 = id_2
  // };
  //   message_t msg_member = {0};
  // message_t *msg_member_p = (message_t *)malloc(sizeof(message_t));

  //   message_t *msg_member_p = (message_t *)malloc(sizeof(message_t));
  // message_t *msg_member_p = &msg_member;
  // (*msg_member_p).number = number;
  //   msg_member_p->id_1 = id_1;
  //   msg_member_p->id_2 = id_2;
  //   msg_member_p->id_1 = "nl";
  //   msg_member_p->id_2 = "nl";
  //   msg_member_p->id_1 = msg_member.id_1;
  //   msg_member_p->id_2 = msg_member.id_2;
  //   snprintf(msg_member_p->id_1, ID_MAXSIZE, "%s", id_1);
  //   snprintf(msg_member_p->id_2, ID_MAXSIZE, "%s", id_2);
  // strcpy((*msg_member_p).id_1, "nl");
  // strcpy((*msg_member_p).id_2, "nl");
  //   strcpy((*msg_member_p).id_1, id_1);
  //   strcpy((*msg_member_p).id_2, id_2);
  //   if (valid(msg_member_p) != 0) {
  //     exit(1);
  //   }

  // message_t_init(msg_member_p);
  // message_t_init(msg_member_p, "hage", "hgoe");
  return message_t_init(_message_t, number, id_1, id_2);
}

#endif
