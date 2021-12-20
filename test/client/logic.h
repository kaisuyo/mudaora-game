#ifndef __LOGIC_H__
#define __LOGIC_H__

typedef enum {
  LOGIN_SIGNAL,
  REGISTER_SIGNAL,
  LOGOUT_SIGNAL,
  GET_RANK_SIGNAL,
  GET_INFO_CURR_GAME,
  CANCEL_MATCH,

  ATTACK_SIGNAL,
  ATTACKED_SIGNAL,
  YELL_SIGNAL,

  MENU_SIGNAL,
  SUCCESS_SIGNAL,
  FAILED_SIGNAL,
  SPEED_SIGNAL,
  STRENGTH_SIGNAL,
  GIVE_IN,
  BET,
  RESULT_SIGNAL
} SignalState;

#endif