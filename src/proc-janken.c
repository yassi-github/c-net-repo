#include <stdio.h>
#include <unistd.h>  //fork

#include "errorutil.h"
#include "randomutil.h"

#define _DEFAULT_SOURCE 1

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
  Lock,
  // ちょき
  Paper,
  // ぱー
  Scissors,
} JankenCard;

// child

// じゃんけんの手を決める
error rand_card(int seed, JankenCard *card) {
  error err;
  int random_int;
  err = randint(seed, &random_int);
  if (err != NULL) {
    return err;
  }
  *card = random_int % 3;
  return NULL;
}

error recv_parent_command() { return NULL; }

error send_card_to_parent() { return NULL; }

error child_janken_loop(int radom_seed) {
  // TODO: use msgQ
  error err;
  bool is_loop = true;
  JankenCard card;
  while (is_loop) {
    err = recv_parent_command();
    if (err != NULL) {
      return err;
    }
    err = rand_card(radom_seed, &card);
    if (err != NULL) {
      return err;
    }
    err = send_card_to_parent();
    if (err != NULL) {
      return err;
    }
  }
  return NULL;
}

error child_process() {
  error err;
  // 手を決める乱数を初期化
  unsigned int random_seed;
  err = init_seed(&random_seed);
  if (err != NULL) {
    return err;
  }
  // loop
  err = child_janken_loop(random_seed);
  if (err != NULL) {
    return err;
  }
  return NULL;
}

// parent

error init() {
  // error err;
  // TODO: init msgQ
  return NULL;
}

error spawn_child(int *child_counter) {
  int pid = fork();
  if (pid == -1) {
    return error_new(error_msg_list.error_fork);
  }
  child_counter += 1;
  // parent process start
  if (pid > 0) {
    return NULL;
  }
  // parent process end
  // child process start
  return child_process();
  // child process end
}

// じゃんけんの手入力うけつけ
error input_card(JankenCard *card) {
  int card_in;
  printf("input your hand[0,1,2]:");
  if (scanf("%d", &card_in) == EOF) {
    return error_msg_list.error_input;
  }
  if (-1 < card_in && card_in < 3) {
    *card = card_in;
    return NULL;
  }
  return error_msg_list.error_input;
}

error send_janken_card_determine_command_to_child() { return NULL; }

error recv_janken_card() { return NULL; }

// 結果表示
error show_card(JankenCard *child_cards, int child_counter) {
  for (int idx = 0; idx > child_counter; idx++) {
    printf("hoi: %d\n", child_cards[idx]);
  }
  return NULL;
}

error parent_janken_loop(int *child_counter) {
  // TODO: use msgQ
  error err;
  JankenCard user_card;
  err = input_card(&user_card);
  if (err != NULL) {
    return err;
  }
  err = send_janken_card_determine_command_to_child();
  if (err != NULL) {
    return err;
  }
  JankenCard child_card[*child_counter];
  err = recv_janken_card(child_card, *child_counter);
  if (err != NULL) {
    return err;
  }
  err = show_card(child_card, *child_counter);
  if (err != NULL) {
    return err;
  }
  return NULL;
}

// main

int main() {
  error err;

  err = init();
  if (err != NULL) {
    error_exit(err);
  }

  int child_counter = 0;
  err = spawn_child(&child_counter);
  if (err != NULL) {
    error_exit(err);
  }

  err = parent_janken_loop(&child_counter);
  if (err != NULL) {
    error_exit(err);
  }

  return 0;
}

#undef _DEFAULT_SOURCE
