#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int SEND(char * msg){
  char cmd[150];
  strcpy(cmd, "SEND ");
  strcat(cmd, msg);
  printf("sending message --%s-- with sizeof %d \n", cmd, sizeof(cmd));
  DoClient1("54.245.33.37", 7295, cmd, 1, sizeof(cmd));
  return 0;
}

int SENDPRIV(char * msg){
  return 0;
}
