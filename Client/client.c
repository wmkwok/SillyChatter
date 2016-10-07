#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <sys/timeb.h>
#include <fcntl.h>
#include <stdarg.h>
#include <time.h>
#include <pthread.h>
#include "registration.h"

//re-define things as a certain type
typedef unsigned char BYTE;
typedef unsigned int DWORD;
typedef unsigned short WORD;

const char * serverIP = "54.245.33.37";
//int port = 7295;
int inSend = 0; //for stopping DoReceive and send msg

/**********************utility functions********************/
//print error statements
void Error(const char* format, ...){
  char msg[4096];
  va_list argptr;
  va_start(argptr, format);
  vsprintf(msg, format, argptr);
  va_end(argptr);
  fprintf(stderr, "Error: %s\n", msg);
  exit(-1);
}

//print that a connection message, should use this for logging.
void Log(const char * format, ...){
  char msg[2048];
  va_list argptr;
  va_start(argptr, format);
  vsprintf(msg, format, argptr);
  va_end(argptr);
  fprintf(stderr, "%s\n", msg);
}

/**************************send/recieve functions*************************/
//send function, takes a socket file descriptor, data, and length
int Send_Blocking(int sockFD, const BYTE * data, int len){
  int nSent = 0; 
  //while not done sending
  while(nSent < len){ 
    //returns number of sent bytes
    int n = send(sockFD, data+nSent, len-nSent, 0);
    if (n >= 0) nSent += n;
    else if(n < 0 && (errno == ECONNRESET || errno == EPIPE)){
      Log("Connection closed.");
      close(sockFD);
      return -1;
    }
    else 
      Error("Unexpected error %d: %s.", errno, strerror(errno));
  }
  return 0;
}

//as a client, it receives the requested data
int Recv_Blocking(int sockFD, BYTE * data, int len){
  int nRecv = 0;
  while(nRecv < len){
    int n = recv(sockFD, data+nRecv, len-nRecv, 0);
    if (n > 0) nRecv += n;
    else if(n == 0 || (n < 0 && errno == ECONNRESET)){
      Log("Connection closed.");
      close(sockFD);
      return -1;
    }
    else 
      Error("Unexpected error %d: %s.", errno, strerror(errno));
  }
  printf("received: %s\n", data);
  return 0;
}

void DoReceive(const char * svrIP, int svrPort, char * msg, int size){
  //initialize socket
  struct sockaddr_in serverAddr;
  memset(&serverAddr, 0, sizeof(serverAddr));
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons((unsigned short)svrPort);
  inet_pton(AF_INET, svrIP, &serverAddr.sin_addr);
  
  //create socket
  int sockFD = socket(AF_INET, SOCK_STREAM, 0);
  if(sockFD == -1) Error("Cannot create socket.");
  
  //create a timer to time our requests
  struct timeb t;
  
  //connect to a server, or return a connect error
  if(connect(sockFD, (const struct sockaddr*)&serverAddr, sizeof(serverAddr)) != 0)
    Error("Cannot connect to server %s:%d.", svrIP, svrPort);
  else{
    while(1){
      if(!inSend){
	//allocate buffer
	BYTE * buf = (BYTE *)malloc(size);

	//record when we start
	//ftime(&t);
	//double startTime = t.time + t.millitm / (double)1000.0f;

	//read response
	Recv_Blocking(sockFD, buf, size);
  
	//record stop time
	//ftime(&t);
	//double endTime = t.time + t.millitm / (double)1000.0f;
  
	//Log("Transaction: %d bytes, %.2lf seconds.", size, endTime-startTime);

	//free mem space
	free(buf);
      }
    }
  }
  close(sockFD);
}

//Here we initialize sockets and send requests
void DoClient(const char * svrIP, int svrPort, char * msg, int size){
  //create buffer for data
  BYTE * buf = (BYTE *)malloc(size);

  //initialize socket
  struct sockaddr_in serverAddr;
  memset(&serverAddr, 0, sizeof(serverAddr));
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons((unsigned short) svrPort);
  inet_pton(AF_INET, svrIP, &serverAddr.sin_addr);

  //create the socket
  int sockFD = socket(AF_INET, SOCK_STREAM, 0);
  if(sockFD == -1) Error("Cannot create socket.");
  
  //create a timer to time our requests
  struct timeb t;
  ftime(&t);
  //record when we start
  double startTime = t.time + t.millitm / (double)1000.0f;
  
  //connect to a server, or return a connect error
  if(connect(sockFD, (const struct sockaddr*)&serverAddr, sizeof(serverAddr)) != 0)
    Error("Cannot connect to server %s:%d.", svrIP, svrPort);
  
  //send a request
  //printf("sending msg %s\n", msg);
  Send_Blocking(sockFD, (BYTE *)msg, size);

  //read response
  Recv_Blocking(sockFD, buf, size);
  
  ftime(&t);
  //record stop time
  double endTime = t.time + t.millitm / (double)1000.0f;
  
  //Log("Transaction: %d bytes, %.2lf seconds.", size, endTime-startTime);
  close(sockFD);

  free(buf);
}

/******************************************************Message creation********************************/
int SEND(char * msg){
  char cmd[150];
  strcpy(cmd, "SEND ");
  strcat(cmd, msg);
  DoClient("54.245.33.37", 7295, cmd, sizeof(cmd));
  return 0;
}

int SENDPRIV(char * msg){
  return 0;
}

/**********************************main function**************************************/


int main(){
  //DoClient("54.245.33.37", 7295, "CONNECT", 1, 9);
  const char * serverIP = "54.245.33.37";
  int port = 7295;
  int nReq = 50;
  int minSize = 10000;
  int maxSize = 100000;

  srand(time(NULL));

  char init[10];
  char msg[128];

  

  //ask for register or login, no availability to public message unless login or register.
  printf("Please type /register to register,\n/ligon to log on, \n/quit to quit.\n");
  while(1){
    printf(": ");
    scanf("%9s", init);
    if(!strcmp(init, "/register")){
      if(REGISTER() == 0){
	printf("Congrats! Your registration was successful. \n");
	break;
      }
      else{
	printf("Registration failed. Please try again or just give up.\n");
      }
    }
    else if(!strcmp(init, "/login")){
      if(LOGIN() == 0){
	break;	
      }
    }
    else if(!strcmp(init, "/quit")){
      break;
    }
  }

  //read in other commands and type public msgs
  printf("Type /help for list of available commands \n");
  while(1){
    int i;
    printf(": ");
    i = scanf(" %500[^\n]s", msg);
    if(i > 0){
      if(!strcmp(msg, "/help")) help();
      else if(!strcmp(msg, "/quit")){printf("quit\n"); break;}
      else {
	printf("you typed: %s\n", msg);
	SEND(msg);
      }
    }
  }

  //    Log("Usage: %s [server IP] [server Port] [# requests] [min_request_size] [max_request_size]", argv[0]);

  return 0;
}
