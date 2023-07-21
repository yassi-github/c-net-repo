#include "qutil.h"

#include <stdio.h>    //snprintf
#include <sys/ipc.h>  // msgrcv,msgsnd
#include <sys/msg.h>  // msgrcv,msgsnd

#include "errorutil.h"

// 配列のサイズはマクロ定数か直書き
// {static,}const int はエラーになる
#define MTEXT_MAXSIZE 80

// internal error messages
struct _error_msg_list {
  const char *error_q_init;
  const char *error_q_gen;
  const char *error_q_send;
  const char *error_q_recv;
  const char *error_malloc;
};
static const struct _error_msg_list error_msg_list = {
    .error_q_init = "failed to init Q",
    .error_q_gen = "failed to gen Q",
    .error_q_send = "failed to send to Q",
    .error_q_recv = "failed to recv from Q",
    .error_malloc = "failed to malloc",
};

typedef struct _msgbuf {
  long mtype;
  char mtext[MTEXT_MAXSIZE];
  // 末尾のメンバは任意長配列として扱えるが扱いは難しい
  // size_t mtext_size;
  // char mtext[];
} Qmsg;

static const int MSG_TYPE = 1;

static error q_init(Qmsg *q_msg, long type, const char *text) {
  q_msg->mtype = type;
  // q_msg->mtext_size = strlen(text); // strlenは\0がない場合こわれる
  // if (snprintf(q_msg->mtext, q_msg->mtext_size, "%s", text) < 0) {
  // mtextのサイズすべてをsnprintfしたほうが安全
  if (snprintf(q_msg->mtext, sizeof(q_msg->mtext), "%s", text) < 0) {
    return error_msg_list.error_q_init;
  }
  return NULL;
}

error q_new(int *new_q_id) {
  int q_id = msgget(IPC_PRIVATE, IPC_CREAT | 0666);
  if (q_id == -1) {
    return error_new(error_msg_list.error_q_gen);
  }
  *new_q_id = q_id;
  return NULL;
}

error q_pop(int q_id, char *data) {
  Qmsg q_msg;
  // MSG_NOERROR: データがサイズに収まるように切り詰める
  // if (msgrcv(q_id, &q_msg, q_msg.mtext_size, MSG_TYPE, MSG_NOERROR) == -1) {
  if (msgrcv(q_id, &q_msg, sizeof(q_msg.mtext), MSG_TYPE, MSG_NOERROR) == -1) {
    return error_new(error_msg_list.error_q_recv);
  }
  // if (snprintf(data, q_msg.mtext_size, "%s", q_msg.mtext) < 0) {
  if (snprintf(data, sizeof(q_msg.mtext), "%s", q_msg.mtext) < 0) {
    return error_msg_list.error_q_recv;
  }
  return NULL;
}

error q_push(int q_id, const char *data) {
  Qmsg q_msg;
  error err = q_init(&q_msg, MSG_TYPE, data);
  if (err != NULL) {
    return err;
  }
  // if (msgsnd(q_id, &q_msg, q_msg.mtext_size, 0) == -1) {
  if (msgsnd(q_id, &q_msg, sizeof(q_msg.mtext), 0) == -1) {
    return error_new(error_msg_list.error_q_send);
  }
  return NULL;
}
