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
    printf("looking at user %s\n", user);
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
  printf("%s\n", registration);
  char * pass = malloc(strlen(registration)+1);
  char * begin = pass;
  char * username;
  strcpy(pass, registration);
  username = strsep(&pass, " ");
  if(VALID_USER(username)){
    printf("ready to add %s into file \n", registration);
    FILE *fp;
    fp = fopen("user.txt", "a+");
    fputs(registration, fp);
    fputs("\n", fp);
    fclose(fp);
    printf("should be registering %s\n", registration);
  }
  else{
    free(begin);
    return 0;
   }
  free(begin);
  return 1;
}

int UNREGISTER(char * username){
  printf("should be de-registering...\n");
  return 0;
}

int LOGIN(char * login){
  char logins[20];
  FILE *fp;
  int len = strlen(login) + 1;
  fp = fopen("user.txt", "r");
  while(fp != NULL && fgets(logins, len, (FILE *)fp) != NULL){
    if(!strcmp(logins, login)){
      printf("sucessful login\n");
      return 1;
    }
  }
  if(fp != NULL) fclose(fp);
  return 0;
}

