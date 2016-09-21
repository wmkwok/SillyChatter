#include <stdio.h>
#include <string.h>


typedef unsigned char BYTE;
/*
 * Ask for username, password
 * Check for password consistancy 4-8
 * Check for Username Availability 4-8
 * return 0 or -1
 */
const char * severIP = "54.245.33.37";
int port = 7295;
int nReq = 50;
int minSize = 10000;
int maxSize = 100000;

int REGISTER(){
  char user[8];
  char pass[8];
  char apass[8];
  char msg[100];

printf("--------------------Registration--------------------\nUsername and Password must be between 4-8 letters and numbers. Password is case sensitive. \n");
  while(1){
    printf("Username: ");
    scanf("%s", &user);
    //check if user is doable

    printf("\nPassword: ");
    scanf("%s", &pass);
    
    if(strlen(pass) > 9 || strlen(pass) < 4){
      printf("invalid password length %d\n", strlen(pass));
      continue;
    }
    
    printf("\nRepeat Password: ");
    scanf("%s", &apass);
    if(!strcmp(apass, pass)){
      //send user registration request
      strcpy(msg, "REGISTER ");
      strcat(msg, user);
      strcat(msg, " ");
      strcat(msg, pass);
      strcat(msg, "\r\n");
      DoClient1("54.245.33.37", 7295, msg, 1, sizeof(msg));
      return 0;
    }
    else{
      printf("mismatching password\n");
    }
  }

}

int UNREGISTER(){

}

int LOGON(){
  printf("logon\n");
  return 0;
}

int LOGOUT(){
  printf("logon\n");
  return 0;
}
