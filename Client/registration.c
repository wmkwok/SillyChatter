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
  char user[10];
  char pass[10];
  char apass[10];
  char msg[30];
  char check[20];

printf("--------------------Registration--------------------\nUsername and Password must be between 4-8 letters and numbers. Password is case sensitive. \n");
  while(1){
    printf("Username: ");
    scanf("%9s", user);
    //check if user is doable

    printf("\nPassword: ");
    scanf("%9s", pass);
    
    if(strlen(pass) > 9 || strlen(pass) < 4){
      printf("invalid password length %lu\n", strlen(pass));
      continue;
    }
    
    printf("\nRepeat Password: ");
    scanf("%9s", apass);
    if(!strcmp(apass, pass)){
      //send check user
      strcpy(check, "USER ");
      strcat(check, user);
      
      if(DoClient("54.245.33.37", 7295, msg, sizeof(msg))){

      //send user registration request
      strcpy(msg, "REGISTER ");
      strcat(msg, user);
      strcat(msg, " ");
      strcat(msg, pass);
      DoClient("54.245.33.37", 7295, msg, sizeof(msg));
      }
      else{
	printf("username taken\n");
      }
      return 0;
    }
    else{
      printf("mismatching password\n");
    }
  }

}

int UNREGISTER(){
  printf("unregister\n");
  return 0;
}

int LOGIN(){
  char user[10];
  char pass[10];
  char msg[30];

  while(1){
    printf("Username: ");
    scanf("%8s", user);
    //check if user is doable
    
    printf("\nPassword: ");
    scanf("%8s", pass);

    strcpy(msg, "LOGIN ");
    strcat(msg, user);
    strcat(msg, " ");
    strcat(msg, pass);
    DoClient("54.245.33.37", 7295, msg, sizeof(msg));
    return 0;
  }
  printf("login %s\n", msg);
  return 0;
}

int LOGOUT(){
  printf("logon\n");
  return 0;
}


void help(){
  printf("no help for u\n");
}
