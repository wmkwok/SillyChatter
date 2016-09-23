#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int SEND(char * msg){
  char cmd[150];
  strcpy(cmd, "SEND ");
  strcat(cmd, msg);
  DoClient1("54.245.33.37", 7295, msg, 1, sizeof(msg));
  return 0;
}

int SENDPRIV(char * msg){
  return 0;
}
