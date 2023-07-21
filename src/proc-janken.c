#include <stdio.h>
#include <stdlib.h>  //exit
#include <string.h>  // strcmp
#include <unistd.h>  //fork

#include "errorutil.h"
#include "qutil.h"
#include "randomutil.h"
#include "waitutil.h"

// internal error messages
struct _error_msg_list {
  const char *error_fork;
  const char *error_kill_children;
  const char *error_input;
};
static const struct _error_msg_list error_msg_list = {
    .error_fork = "failed to fork",
    .error_kill_children = "failed to kill child processes",
    .error_input = "invalid input hand",
};

typedef enum {
  // ぐー
  Rock,
  // ちょき
  Paper,
  // ぱー
  Scissors,
} JankenCard;
struct _janken_card_str {
  const char *rock;
  const char *paper;
  const char *scissors;
  const char *unexpected;
};
static const struct _janken_card_str janken_card_str = {
    .rock = u8"✊",
    .paper = u8"🖐️",
    .scissors = u8"✌️",
    .unexpected = u8"👻",
};

// child

// じゃんけんの手を決める
error rand_card(JankenCard *card) {
  error err;
  int random_int;
  err = randint(&random_int);
  if (err != NULL) {
    return err;
  }
  *card = random_int % 3;
  return NULL;
}

// wait till command recv
error recv_parent_command(int commandq_id) {
  char _unused;
  // fixme: dont consume for mul children
  error err = q_pop(commandq_id, &_unused);
  if (err != NULL) {
    return err;
  }
  return NULL;
}

error send_card_to_parent(int q_id, JankenCard *card) {
  error err;
  err = q_push(q_id, (char *)card);
  if (err != NULL) {
    return err;
  }
  return NULL;
}

error child_janken_loop(int q_id, int commandq_id) {
  error err;
  bool is_loop = true;
  JankenCard card;
  // (仮: 5回ループ)
  int _loop_counter = 0;
  while (is_loop) {
    _loop_counter += 1;
    if (_loop_counter > 5) {
      return NULL;
    }

    err = recv_parent_command(commandq_id);
    if (err != NULL) {
      return err;
    }
    err = rand_card(&card);
    if (err != NULL) {
      return err;
    }
    err = send_card_to_parent(q_id, &card);
    if (err != NULL) {
      return err;
    }
  }
  return NULL;
}

error child_process(int q_id, int commandq_id) {
  error err;
  // 手を決める乱数を初期化
  err = init_seed();
  if (err != NULL) {
    return err;
  }
  // loop
  err = child_janken_loop(q_id, commandq_id);
  if (err != NULL) {
    return err;
  }
  return NULL;
}

// parent

error init(int *q_id, int *commandq_id) {
  error err;
  err = q_new(q_id);
  if (err != NULL) {
    return err;
  }
  err = q_new(commandq_id);
  if (err != NULL) {
    return err;
  }
  return NULL;
}

error spawn_child(int q_id, int commandq_id, int *child_counter) {
  int pid = fork();
  if (pid == -1) {
    return error_new(error_msg_list.error_fork);
  }
  *child_counter += 1;
  // parent process start
  if (pid > 0) {
    return NULL;
  }
  // parent process end

  // child process start
  error err = child_process(q_id, commandq_id);
  if (err != NULL) {
    error_exit(err);
  }
  exit(EXIT_SUCCESS);
  // child process end
}

// じゃんけんの手入力うけつけ
error input_card(JankenCard *card) {
  int card_in;
  printf(
      u8"じゃんけん…\n"
      u8"(%s: %d, %s: %d, %s: %d): ",
      janken_card_str.rock, Rock, janken_card_str.paper, Paper,
      janken_card_str.scissors, Scissors);
  if (scanf("%d", &card_in) == EOF) {
    return error_msg_list.error_input;
  }
  if (-1 < card_in && card_in < 3) {
    *card = card_in;
    return NULL;
  }
  return error_msg_list.error_input;
}

error send_janken_card_determine_command_to_child(int commandq_id) {
  char _unused = 0;
  error err = q_push(commandq_id, &_unused);
  if (err != NULL) {
    return err;
  }
  return NULL;
}

error recv_janken_card(int q_id, JankenCard *child_card) {
  char data;
  error err = q_pop(q_id, &data);
  if (err != NULL) {
    return err;
  }
  *child_card = (JankenCard)data;
  return NULL;
}

const char *janken_result(JankenCard *user_card, JankenCard *system_card) {
  const char *draw_string = u8"あいこ！";
  const char *win_string = u8"勝ち！";
  const char *lose_string = u8"負け！";
  switch (*user_card) {
    case Rock:
      switch (*system_card) {
        case Rock:
          return draw_string;
        case Paper:
          return lose_string;
        case Scissors:
          return win_string;
      }
      break;
    case Paper:
      switch (*system_card) {
        case Rock:
          return win_string;
        case Paper:
          return draw_string;
        case Scissors:
          return lose_string;
      }
      break;
    case Scissors:
      switch (*system_card) {
        case Rock:
          return lose_string;
        case Paper:
          return win_string;
        case Scissors:
          return draw_string;
      }
      break;
  }
  return janken_card_str.unexpected;
}

const char *janken_tostr(JankenCard *card) {
  switch (*card) {
    case Rock:
      return janken_card_str.rock;
    case Paper:
      return janken_card_str.paper;
    case Scissors:
      return janken_card_str.scissors;
  }
  // unexpected
  return janken_card_str.unexpected;
}

// 結果表示
error show_card(JankenCard *user_card, JankenCard child_cards[],
                int child_counter) {
  for (int idx = 0; idx < child_counter; idx++) {
    printf(
        u8"ほい\n"
        u8"あなた: %s vs %s ...%s\n",
        janken_tostr(user_card), janken_tostr(&(child_cards[idx])),
        janken_result(user_card, &(child_cards[idx])));
  }
  return NULL;
}

error parent_janken_loop(int q_id, int commandq_id, int child_counter) {
  error err;
  JankenCard user_card;
  bool is_loop = true;
  // (仮: 5回ループ)
  int _loop_counter = 0;
  while (is_loop) {
    _loop_counter += 1;
    if (_loop_counter > 5) {
      return NULL;
    }

    err = input_card(&user_card);
    if (err != NULL) {
      return err;
    }
    err = send_janken_card_determine_command_to_child(commandq_id);
    if (err != NULL) {
      return err;
    }
    JankenCard child_cards[child_counter];
    for (int child_idx = 0; child_idx < child_counter; child_idx++) {
      err = recv_janken_card(q_id, &(child_cards[child_idx]));
      if (err != NULL) {
        return err;
      }
    }
    err = show_card(&user_card, child_cards, child_counter);
    if (err != NULL) {
      return err;
    }
  }
  return NULL;
}

// continue waiting until all child processes exited.
error wait_terminate_all_childlen(int child_process_counter) {
  while (child_process_counter > 0) {
    /* wait terminating child process */
    if (wait_exit() > 0) child_process_counter--;
  }
  return NULL;
}

// main

int main() {
  printf("5回じゃんけん\n");

  error err;
  int q_id;
  int commandq_id;
  err = init(&q_id, &commandq_id);
  if (err != NULL) {
    error_exit(err);
  }

  int child_counter = 0;
  err = spawn_child(q_id, commandq_id, &child_counter);
  if (err != NULL) {
    error_exit(err);
  }

  err = parent_janken_loop(q_id, commandq_id, child_counter);
  if (err != NULL) {
    error_exit(err);
  }

  err = wait_terminate_all_childlen(child_counter);
  if (err != NULL) {
    error_exit(err);
  }
  return 0;
}
