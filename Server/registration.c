#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int VALID_USER(char * username){
  return 1;
  FILE *fp;
  char user[10];
  int len = strlen(username);
  fp = fopen("user.txt", "r");
  if(fp == NULL) return 1;
  while(fp != NULL && fgets(user, len, (FILE *)fp) != NULL){
    if(!strcmp(user, username)){
      fclose(fp);
      return 0;
    }
  }
  fclose(fp);
  return 1;
}

//takes username and pass and records into file
int REGISTER(char *registration){
  char * pass = malloc(strlen(registration)+1);
  char * begin = pass;
  char * username;
  strcpy(pass, registration);
  username = strsep(&pass, " ");
  if(VALID_USER(username)){
    FILE *fp;
    fp = fopen("user.txt", "a+");
    fputs(registration, fp);
    fputs("\n", fp);
    fclose(fp);
  }
  else{
    free(begin);
    printf("REGISTER: %s failed\n", registration);
    return 0;
   }
  free(begin);
  printf("REGISTER: %s succeeded\n", registration);
  return 1;
}

int LOGIN(char * login){
  char logins[20];
  FILE *fp;
  int len = strlen(login) + 1;
  fp = fopen("user.txt", "r");
  while(fp != NULL && fgets(logins, len, (FILE *)fp) != NULL){
    if(!strcmp(logins, login)){
      *(login+len-2) = '\0';
      printf("LOGIN: %s succeeded\n", login);
      return 1;
    }
  }
  if(fp != NULL) fclose(fp);
  *(login+len-2) = '\0';
  printf("LOGIN: %s failed\n", login);
  return 0;
}


int UNREGISTER(char * username){
  printf("should be de-registering...\n");
  return 0;
}
