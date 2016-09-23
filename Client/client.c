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
#include "registration.h"
#include "send.h"

//re-define things as a certain type
typedef unsigned char BYTE;
typedef unsigned int DWORD;
typedef unsigned short WORD;

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

//generate random number use to send random request size.
int GetRandom(int min, int max){
  DWORD r = 0;
  int i;
  for (i=0; i<4; i++){
    r = (r | (DWORD)(rand() % 256)) << 8;
  }
  return (int)(r % (max-min+1) + min);
}

//checks the data, this is test so sends alphabet, we know it should be alphabet
//dont think we need this anymore
void CheckData(BYTE * buf, int size){
  int i;
  for (i=0; i<size; i++)
    if (buf[i] != 'A' + i % 26)
      Error("Received wrong data.");
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
  //  printf("gotten: %s \n", data);
  return 0;
}

//Here we initialize sockets and send requests
void DoClient(const char * svrIP, int svrPort, int nReq, int minSize, int maxSize){
  //create buffer for data
  BYTE * buf = (BYTE *)malloc(maxSize);

  //initialize socket
  struct sockaddr_in serverAddr;
  memset(&serverAddr, 0, sizeof(serverAddr));
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons((unsigned short) svrPort);
  inet_pton(AF_INET, svrIP, &serverAddr.sin_addr);

  int i;
  //make n requests
  for(i=0; i<nReq; i++){
    //create the socket
    int sockFD = socket(AF_INET, SOCK_STREAM, 0);
    if(sockFD == -1) Error("Cannot create socket.");

    //generate a random request size, based on min/max given
    int size = GetRandom(minSize, maxSize);
    
    //create a timer to time our requests
    struct timeb t;
    ftime(&t);
    //record when we start
    double startTime = t.time + t.millitm / (double)1000.0f;
    
    //connect to a server, or return a connect error
    if(connect(sockFD, (const struct sockaddr*)&serverAddr, sizeof(serverAddr)) != 0)
    Error("Cannot connect to server %s:%d.", svrIP, svrPort);
    
    //send a 4-Byte request
    if(Send_Blocking(sockFD, (const BYTE *)&size, 4) < 0) break;
    
    //read response
    if(Recv_Blocking(sockFD, buf, size) < 0) break;
    
    ftime(&t);
    //record stop time
    double endTime = t.time + t.millitm / (double)1000.0f;

    Log("Transaction %d: %d bytes, %.2lf seconds.", i, size, endTime-startTime);
    CheckData(buf, size);
    close(sockFD);
  }
  free(buf);
}

//Here we initialize sockets and send requests
void DoClient1(const char * svrIP, int svrPort, char * msg, int nReq, int size){
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


int main(){
  DoClient1("54.245.33.37", 7295, "CONNECT", 1, 9);
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
    i = scanf("%127s", msg);
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


  const char * serverIP = "54.245.33.37";
  int port = 7295;
  int nReq = 50;
  int minSize = 10000;
  int maxSize = 100000;

  srand(time(NULL));

  return 0;
}
