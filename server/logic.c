#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>

#include "handle.h"
#include "linkedList.h"
#include "logic.h"

void addToken(char *str, SignalState signal) {
  int len = strlen(str);
  str[len] = '|';
  str[len+1] = '0' + signal;
  str[len+2] = '\0';
}

// login logout signup disconnect
void answer(int confd, char *message, SignalState signal) {
  char string[1000] = "";
  strcat(string, message);
  addToken(string, signal);
  if (send(confd, string, strlen(string), 0) <= 0) {
    printf("error by send function\n");
  };
}

void sendToOtherClients(User head, char *message, SignalState SIGNAL, int confd){
  User user = head;
  while(user != NULL) {
    if (user->online > 0 && user->online != confd) {
      answer(user->online, message, SIGNAL);
    }
    user = user->next;
  }
}

void sendAllServer(User head, char *message, SignalState SIGNAL) {
  User user = head;
  while(user != NULL) {
    if (user->online > 0) {
      answer(user->online, message, SIGNAL);
    }
    user = user->next;
  }
}

void logIn(User head, int confd, char *username, char *password) {
  User user = findByName(head, username); 
  if (user == NULL) {
    answer(confd, "User is not exist", FAILED_SIGNAL);
  } else if (user->online > -1) {
    answer(confd, "Account is login in other client", FAILED_SIGNAL);
  } else if (strcmp(user->password, password) == 0) {
    user->online = confd; // id of socket connection
    answer(confd, "Login successfully", SUCCESS_SIGNAL);
  } else {
    answer(confd, "Wrong password", FAILED_SIGNAL);
  }
}

User signUp(User head, int confd, char *username, char *password) {
  User user = findByName(head, username);
  if (user != NULL) {
    answer(confd, "Account is exist", FAILED_SIGNAL);
  } else {
    User newU = newUser(username, password);
    newU->online = confd;
    newU->next = head;
    saveToFile(newU, "account.txt");
    answer(confd, "Wellcome", SUCCESS_SIGNAL);
    return newU;
  }
  return head;
}

void logOut(User head, int confd) {
  User user = findById(head, confd);
  if(user != NULL) {
    user->online = -1;
  }
}

User player(User head, int confd) {
  User user = findById(head, confd);
  user->hp = 1000;
  answer(confd, "ok", SUCCESS_SIGNAL);
  return user;
}

void playerError(int confd) {
  answer(confd, "Wrong mode or the match has started", FAILED_SIGNAL);
}

void getInfoCurrGame(User head, User user1, int bet1, User user2, int bet2, int confd, int totalViewer) {
  // player 1
  if (user1 != NULL && user1->online == confd && user2 == NULL) {
    return;
  }

  // player 2
  if (user1 != NULL && user2 != NULL && user2->online == confd) {
    char data[100] = "";
    char num1[5] = "", num2[5] = "", total[5] ="";
    sprintf(num1, "%d", bet1);
    sprintf(num2, "%d", bet2);
    sprintf(total, "%d", totalViewer-2);

    strcat(data, user1->username);
    strcat(data, ":");
    strcat(data, num1);
    strcat(data, " ");
    strcat(data, user2->username);
    strcat(data, ":");
    strcat(data, num2);
    strcat(data, " ");
    strcat(data, total);
    // user1:15 user2:50
    // printf("error is not in server: %d, %d\n", user1->online, user2->online);
    answer(user1->online, data, GET_INFO_CURR_GAME);
    answer(user2->online, data, GET_INFO_CURR_GAME);
    return;
  }
  // viewer
  if (user1 != NULL && user2 != NULL) {
    char data[100] = "";
    char num1[5] = "", num2[5] = "", total[5] = "";
    sprintf(num1, "%d", bet1);
    sprintf(num2, "%d", bet2);
    sprintf(total, "%d", totalViewer-2);

    strcat(data, user1->username);
    strcat(data, ":");
    strcat(data, num1);
    strcat(data, " ");
    strcat(data, user2->username);
    strcat(data, ":");
    strcat(data, num2);
    strcat(data, " ");
    strcat(data, total);

    answer(confd, data, GET_INFO_CURR_GAME);
    sendToOtherClients(head, data, JOIN_STREAM, confd);

    return;
  }

  if ((user1 != NULL && user2 == NULL) || (user1 == NULL && user2 == NULL) || (user1 != NULL && user2 != NULL)) {
    answer(confd, "There is no match now", FAILED_SIGNAL);
    return;
  }
  // is viewer but players is not 2
}

void winLose(User head, User user1, User user2) {
  user1->win++;
  user2->loss++;
  saveToFile(head, "account.txt");

  char mess[100] = "Winner: ";
  strcat(mess, user1->username);
  strcat(mess, " - Loser: ");
  strcat(mess, user2->username);
  sendAllServer(head, mess, RESULT_SIGNAL);
}

void yell(User head, char *message, int confd) {
  User user = findById(head, confd);
  char str[100] = "";
  strcpy(str, user->username);
  strcat(str, "|");
  strcat(str, message);
  sendAllServer(head, str, YELL_SIGNAL);
  // printf("%s", message);
}

void getRank(User head, int confd) {
  char rankList[1000] = "";
  User user = findById(head, confd);
  char username[50];
  strcpy(username, user->username);
  // printf("%s\n", user->username);
  sortList(head);
  getRankList(head, rankList, username);
  answer(confd, rankList, GET_RANK_SIGNAL);
  // printf("%s\n", user->username);
}

void bet(User head, User user1, int bet1, User user2, int bet2){
  char data[100] = "";
  char num1[5] = "", num2[5] = "";
  sprintf(num1, "%d", bet1);
  sprintf(num2, "%d", bet2);

  strcat(data, user1->username);
  strcat(data, ":");
  strcat(data, num1);
  strcat(data, " ");
  strcat(data, user2->username);
  strcat(data, ":");
  strcat(data, num2);
  sendAllServer(head, data, BET);
}

void leave_stream(User head){
  char data[100] = "abc";
  sendAllServer(head, data, LEAVE_STREAM);
}