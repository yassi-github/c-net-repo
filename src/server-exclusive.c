#include <stdio.h>
#include <stdlib.h>  // malloc
#include <string.h>

#include "errorutil.h"    // error_msg
#include "messageutil.h"  // message_extract

int main() {
  // message_t msg_member = {.number = 0, .id_1 = "NONE", .id_2 = "NONE"};
  // char *msg = new_message(msg_member, MESSAGE_MAXSIZE);
  // char message[MESSAGE_MAXSIZE];
  // strncpy(message, msg, MESSAGE_MAXSIZE);
  message_t ext_msg_member;
  int err = message_extract(&ext_msg_member, "0 a b");
  if (err != 0) {
    error_msg("failed to extract msg");
  }
  // // (void )! extract_message(msg);
  // // free affects till message_t members
  // // free(msg);
  printf("%d %s %s\n", ext_msg_member.number, ext_msg_member.id_1,
         ext_msg_member.id_2);
  return 0;
}

// typedef struct {
//   int year;       /* 学年 */
//   int clas;       /* クラス */
//   int number;     /* 出席番号 */
//   char *name;     /* 名前 */
//   double stature; /* 身長 */
//   double weight;  /* 体重 */
// } student;

// int main(void) {
//   // student *pdata = malloc(sizeof(student));

//   student data;
//   student *pdata;
//   pdata = &data;                  /* 初期化 */
//   char name_sized[64];
//   pdata->name = name_sized;

//   (*pdata).year = 10;             /* 通常変数モードへの切り替え */
//   strcpy((*pdata).name, "MARIO"); /* 通常変数モードへの切り替え */

//   printf("%d %s\n", pdata->year, pdata->name);

//   return 0;
// }
