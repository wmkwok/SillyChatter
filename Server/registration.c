#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//takes username and pass and records into file
// so far the the mode is readable writable overwrite w+. Change to a+ for no overwrite.
int REGISTER(char *registration){
  char * name;
  FILE *fp;
  fp = fopen("user.txt", "a+");
  fputs(registration, fp);
  fputs("\n", fp);
  fclose(fp);
  printf("should be registering %s\n", registration);
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
  while(fgets(logins, len, (FILE *)fp) != NULL){
    if(!strcmp(logins, login)){
      printf("sucessful login\n");
      return 1;
    }
  }
  fclose(fp);
  return 0;
}
