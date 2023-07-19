// 親プロセスから子プロセスに対してメッセージを送る．このとき，親プロセ
// スは 0.5秒 ごとにメッセージを10回送り，子プロセスは連続してメッセー
// ジを受け取るようにする．
// 送り側のタイミングに合わせて受け側がメッセージを受け取っていることを
// 示す例となっている．

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define MSGNUM 10
#define MAXMSG 100  // 最大メッセージ長

int main() {
  int pid;
  int status;
  int i;
  int msgid;
  struct msgbuf {  // メッセージを入れる構造体の定義
    long mtype;
    char mtext
        [1];  // この部分にメッセージを入れるが，必要なメッセージ長に合わせ動的に確保
  } * mbuf;
  int msgsize;

  // メッセージキューの確保
  msgid = msgget(IPC_PRIVATE, IPC_CREAT | 0666);
  if (msgid == -1) {
    perror("msgget");
    exit(1);
  }

  // メッセージデータを入れる領域の確保
  if ((mbuf = (struct msgbuf *)malloc(sizeof(struct msgbuf) + MAXMSG)) ==
      NULL) {
    perror("memory");
    exit(1);
  }
  pid = fork();
  switch (pid) {
    case 0:
      printf("I am child process\n");
      for (i = 0; i < MSGNUM; i++) {
        if ((msgsize = msgrcv(msgid, mbuf, MAXMSG, 0, 0)) == -1) {
          //                                       ^  ^
          // これら2つのパラメタを設定することで受け取るメッセージを選択できる
          // (この例では全て)
          perror("Message receive");
        } else {
          mbuf->mtext[msgsize] = '\0';
          printf("Child receives \"%s\"\n", mbuf->mtext);
        }
      }
      return 0;
    case -1:
      printf("Fork error\n");
      exit(1);
    default:
      printf("Process id of child process is %d\n", pid);
      mbuf->mtype = 100;
      for (i = 0; i < MSGNUM; i++) {
        snprintf(mbuf->mtext, MAXMSG, "%d番目のメッセージ!", i);
        msgsize = strlen(mbuf->mtext);
        if (msgsnd(msgid, mbuf, msgsize, 0)) {
          perror("Message Send");
        }
        printf("Parent sends \"%s\"\n", mbuf->mtext);
        usleep(500000);
      }
      if (wait(&status) == -1) {
        perror("Wait error\n");
      } else {
        printf("Return value is %d\n", status);
      }
      // メッセージバッファの削除
      if (msgctl(msgid, IPC_RMID, NULL)) {
        perror("shmctl");
        exit(1);
      }
  }
  return 0;
}
