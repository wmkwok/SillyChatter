#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//takes username and pass and records into file
int REGISTER(char *registration){
  char * pass = malloc(strlen(registration)+1);
  char * begin = pass;
  strcpy(pass, registration);
  FILE *fp;
  fp = fopen("user.txt", "a+");
  fputs(registration, fp);
  fputs("\n", fp);
  fclose(fp);
 
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
