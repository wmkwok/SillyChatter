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
#include <poll.h>
#include "commands.h"

#define MAX_REQUEST_SIZE 10000000
#define MAX_CONCURRENCY_LIMIT 2
#define MAX_MSG_SIZE 512

//redefine types
typedef unsigned char BYTE;
typedef unsigned int DWORD;
typedef unsigned short WORD;

const char * svrIP = "54.245.33.37";
int svrPort = 7295;

//structure for connection
struct CONN_STAT{
  int size;
  int nRecv;
  int nSent;
};

//no need for structure to record users...we have two:server and local

//other variables for poll and statuses
int nConns;
struct pollfd peers[MAX_CONCURRENCY_LIMIT+1];
struct CONN_STAT connStat[MAX_CONCURRENCY_LIMIT+1];


/*******************************Utility Functions********************************/
//To report an Error
void Error(const char * format, ...){
  char msg[4096];
  va_list argptr;
  va_start(argptr, format);
  vsprintf(msg, format, argptr);
  va_end(argptr);
  fprintf(stderr, "Error: %s\n", msg);
  exit(-1);
}

//to log something?
void Log(const char * format, ...){
  char msg[4096];
  va_list argptr;
  va_start(argptr, format);
  vsprintf(msg, format, argptr);
  va_end(argptr);
  fprintf(stderr, "%s\n", msg);
}

BYTE * send_create(BYTE * msg){
  char * token;
  char * newMsg = (char *)msg;
  char * reg = "REGISTER wmkwok forever";
  char * log = "LOGIN test test";
  if(!strcmp(newMsg, "/register")){
    return (BYTE *)reg;
    //go through register process and return register msg for sending
    
  }
  else if(!strcmp(newMsg, "/login")){
    return (BYTE *)log;
    //go through login process and return login msg for sending
  }
  else{
    return "another poop";
    //assume it's a message and take apart to see if is PRIVMSG
  }
  return "poop";
}

int MsgHandle(BYTE * msg, int usrIndex){
  //if sent by local, send to server
  //if sent by server, print or OK
  printf("received msg %s\n", msg);
  char * token;
  char * newMsg = (char *)msg;

  if(!strcmp(newMsg, "OK")) return 1;
  else if(!strcmp(newMsg, "NOK")) return 0;
  else{
    return 0;
  }
  return 0;
}

int Send_NonBlocking(int sockFD, const BYTE * data, int len, struct pollfd * pPeer){
  int nSent = 0;
  while(nSent < len){
    //returns number of sent bytes
    int n = send(sockFD, data+nSent, len-nSent, 0);
    if(n>=0) nSent += n;
    else if(n < 0 && (errno == ECONNRESET || errno == EPIPE)){
      Log("Connection closed.");
      close(sockFD);
      return -1;
    }
    else if(n < 0 && (errno == EWOULDBLOCK)){
      pPeer->events |= POLLWRNORM;
      return 0;
    }
    else
      Error("Unexpected error %d: %s.", errno, strerror(errno));
  }
  pPeer->events &= ~POLLWRNORM;
  return 0;
}

int Recv_NonBlocking(int sockFD, BYTE * data, int len, struct pollfd * pPeer, int usrIndex){
  int nRecv = 0;
  while(nRecv < len){
    int n = recv(sockFD, data+nRecv, len-nRecv, 0);
    if(n > 0) nRecv += n;
    else if(n == 0 || (n < 0 && errno == ECONNRESET)){
      Log("Connection closed.");
      close(sockFD);
      exit(-1);
    }
    else if(n < 0 && (errno == EWOULDBLOCK)){
      printf("recieved: %s\n", (char*)data);
      return MsgHandle(data, usrIndex);
    }
    else
      Error("Unexpected recv error %d: %s.", errno, strerror(errno));
  }
  return 0;
}

void SetNonBlockIO(int fd){
  int val = fcntl(fd, F_GETFL, 0);
  if(fcntl(fd, F_SETFL, val | O_NONBLOCK) != 0)
    Error("Cannot set nonblocking I/O.");
}

void RemoveConnection(int i){
  close(peers[i].fd);
  if(i < nConns){
    memmove(peers+i, peers + i + 1, (nConns - i) * sizeof(struct pollfd));
    memmove(connStat+i, connStat + i + 1, (nConns - i) * sizeof(struct CONN_STAT));
  }
  nConns--;
}

//
void DoReceive(){
  //create sneeded variables
  //a BYTE array to store msgs
  int i;
  BYTE msg[512];

  //initialize internet structure
  struct sockaddr_in serverAddr;
  memset(&serverAddr, 0, sizeof(serverAddr));
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons((unsigned short) svrPort);
  inet_pton(AF_INET, svrIP, &serverAddr.sin_addr);
  
  int sockFD = socket(AF_INET, SOCK_STREAM, 0);
  //make sure socket created
  if(sockFD == -1) Error("Cannot create socket.");

  if(connect(sockFD, (const struct sockaddr*)&serverAddr, sizeof(serverAddr)) != 0)
    printf("Cannot connect to server %s: %d", svrIP, svrPort);
  
  //setup for peers for the socket
  nConns = 1;
  memset(peers, 0, sizeof(peers));
  peers[0].fd = sockFD;
  peers[0].events = POLLRDNORM;
  //set to non-blocking
  SetNonBlockIO(sockFD);


  //set up poll for stdin?
  peers[1].fd = fileno(stdin);
  peers[1].events = POLLIN;
  SetNonBlockIO(peers[1].fd);
  
  int connID = 0;
  while(1){

    //number of needy connections
    int nReady = poll(peers, 2, -1); //number of connections plus listener & stdin
    //if it's negative there's a big problem
    if(nReady < 0)
      Error("Ivalid poll() return value.");
    
    //if socket has incoming, recv
    if(peers[0].revents & POLLRDNORM){      
      //recv something from someone
      Recv_NonBlocking(peers[0].fd, msg, MAX_MSG_SIZE, &peers[0], 0);
    }
    //if stdin has incoming, take and send
    if(peers[1].revents & POLLIN){
      //take stdin input and process and send what it returns
      //int size = scanf("%500[^\n]s", msg);
      fgets(msg, 500, stdin);
      printf("got %s from stdin size %d\n", msg, strlen(msg));
      Send_NonBlocking(peers[0].fd, msg, strlen(msg) + 1, &peers[0]);
      //memset(&msg, 0, 512);
      //send_create(msg);
    }
  }
  close(sockFD);
}


int DoClient(const char svrIP, int svrPort, char * msg, int size){
  return 1;
}

void main(){
  DoReceive();
}
