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
#include <poll.h>
#include <time.h>
#include "registration.h"

//just redefining sizes in better names...
typedef unsigned char BYTE;
typedef unsigned int DWORD;
typedef unsigned short WORD;

#define MAX_REQUEST_SIZE 10000000
#define MAX_CONCURRENCY_LIMIT 64
#define MAX_MSG_SIZE 4096

BYTE msg[MAX_MSG_SIZE];
int pmsg = 0;

//structure for a connection to record their status
struct CONN_STAT {
  int size;//0 if unknown yet
  int nRecv;
  int nSent;
};

//structure for recording online user, we probs dont need pass?
struct user{
  char * name;
  char * pass;
  char * addr;
};

//other variables we need, number of connections, and arrays of the above structures
//and an array of file descriptors to record FDs and poll
int nConns;
struct pollfd peers[MAX_CONCURRENCY_LIMIT+1];
struct CONN_STAT connStat[MAX_CONCURRENCY_LIMIT+1];
struct user users[MAX_CONCURRENCY_LIMIT+1];

/***********************************************utility functions**************************************/
//To report an error, also reduces main code by alot
void Error(const char * format, ...) {
  char msg[4096];
  va_list argptr;
  va_start(argptr, format);
  vsprintf(msg, format, argptr);
  va_end(argptr);
  fprintf(stderr, "Error: %s\n", msg);
  exit(-1);
}

//Similar to error, but i supppose we can change this to do the logging
//requirement of the project
void Log(const char * format, ...) {
  char msg[2048];
  va_list argptr;
  va_start(argptr, format);
  vsprintf(msg, format, argptr);
  va_end(argptr);
  fprintf(stderr, "%s\n", msg);
}

void logStat(const char * format, ...){
  char msg[2048];
  FILE * fp;
  time_t ltime; /* calendar time */
  ltime=time(NULL); /* get current cal time */
  va_list argptr;
  va_start(argptr, format);
  vsprintf(msg, format, argptr);
  fp = fopen("log.txt", "a+");
  if(fp != NULL){
    fputs(msg, fp);
    fputs("TIME:", fp);
      fputs(asctime(localtime(&ltime)), fp);
    fputs("\n", fp);
    fclose(fp);
  }
  va_end(argptr);
}

/****************************************Communication functions*************************************/

//Sends something to target file descriptor(socket), nonblocking.
int Send_NonBlocking(int sockFD, const BYTE * data, int len, struct CONN_STAT * pStat, struct pollfd * pPeer) {
  pStat->nSent = 0;
  while (pStat->nSent < len) {
    int n = send(sockFD, data + pStat->nSent, len - pStat->nSent, 0);
    if (n >= 0) {
      pStat->nSent += n;
    } else if (n < 0 && (errno == ECONNRESET || errno == EPIPE)) {
      Log("Connection closed.");
      close(sockFD);
      return -1;
    } else if (n < 0 && (errno == EWOULDBLOCK)) {
      pPeer->events |= POLLWRNORM;
      return 0;
    } else {
      Error("Unexpected send error %d: %s", errno, strerror(errno));
    }
  }
  pPeer->events &= ~POLLWRNORM;
  return 0;
}

//Removes a connection from peers, connStat, as well as users array. Decreases number of connections.
void RemoveConnection(int i) {
  close(peers[i].fd);
  if (i < nConns) {
    memmove(peers + i, peers + i + 1, (nConns-i) * sizeof(struct pollfd));
    memmove(connStat + i, connStat + i + 1, (nConns-i) * sizeof(struct CONN_STAT));
    memmove(users + i, users + i + 1, (nConns-i) * sizeof(struct user));
  }
  nConns--;
}

//after recv reads msg it is picked apart and
//executed by MsgHandle.
int MsgHandle(BYTE * msg, int usrIndex){
  logStat("%s: %s\n",users[usrIndex].addr, msg);
  char * token;
  char * newMsg = malloc(strlen((char *)msg)+1);
  char * begin = newMsg;
  strcpy(newMsg, (char *)msg);
  if(!strcmp(newMsg, "HELP\n")){
    strcpy((char *)msg, "Available commands: \n");
    strcat((char *)msg, "WHOAMI :shows your username tag when you message\n");
    strcat((char *)msg, "REGISTER [username] [password]: registration for an account\n");
    strcat((char *)msg, "LOGIN [username] [password]: log into an account\n");
    strcat((char *)msg, "SEND [message]: sends a public message to all online users, anonymous included\n");
    strcat((char *)msg, "PSEND [username] [password]: sends a private message to online user\n");
    strcat((char *)msg, "QUIT: closes the connection and your client\n");
    strcat((char *)msg, "LOGOUT: closes the connection and your client\n");
    pmsg = 1;
    Send_NonBlocking(peers[usrIndex].fd, msg, strlen((char*)msg), &connStat[usrIndex], &peers[usrIndex]);
    logStat("SENT to %s: %s\n", (users[usrIndex].name == NULL)? users[usrIndex].addr:users[usrIndex].name, (char*)msg);
    free(begin);
    return 0;
  }  

  //receive a connection request
  else if(!strcmp(newMsg, "WHOAMI\n")){
    strcpy((char *)msg, "You are: ");
    if(users[usrIndex].name != NULL){
      strcat((char *)msg, users[usrIndex].name);
      strcat((char *)msg, "\n");
    }
    else{
      strcat((char *)msg, users[usrIndex].addr);
      strcat((char *)msg, "\n");
    }
    pmsg = 1;
    Send_NonBlocking(peers[usrIndex].fd, msg, strlen((char*)msg), &connStat[usrIndex], &peers[usrIndex]);
    logStat("SENT to %s: %s\n", (users[usrIndex].name == NULL)? users[usrIndex].addr:users[usrIndex].name, (char*)msg);
    free(begin);
    return 0;
  }
  
  else if(!strcmp(newMsg, "LIST\n")){
    int i;
    strcpy((char *)msg, "Users Online:\n");
    for(i=1;i<=nConns+1;i++){
      if(users[i].name != NULL){
	strcat((char *)msg, users[i].name);
	strcat((char *)msg, "\n");
      }
    }
    free(begin);
    return 0;
  }

  else if(!strcmp(newMsg, "LOGOUT\n") || !strcmp(newMsg, "QUIT\n")){
    logStat("Removing user %s from connections\n", (users[usrIndex].name == NULL)? users[usrIndex].addr:users[usrIndex].name);
    strcpy((char *)msg, (users[usrIndex].name == NULL)? users[usrIndex].addr:users[usrIndex].name);
    strcat((char *)msg, " is offline");
    RemoveConnection(usrIndex);
    free(begin);
    return 0;
  }
  
  if((token = strsep(&newMsg, " ")) != NULL){
    if(!strcmp(token, "REGISTER") && users[usrIndex].name == NULL){
      if(REGISTER(newMsg)){
	users[usrIndex].name = (char *)malloc(strstr(newMsg, " ") - newMsg); 
	strcpy(users[usrIndex].name, strsep(&newMsg, " "));
	logStat("LOGIN success with user %s.",users[usrIndex].name );
	strcpy((char *)msg, users[usrIndex].name);
	strcat((char *)msg, " is online");
	free(begin);
	return 1;
      }
      else{
	logStat("REGISTER failed.");
	strcpy((char*)msg, "Registration failed! Make sure you aren't already logged on.\n");
	pmsg = 1;
	Send_NonBlocking(peers[usrIndex].fd, msg, strlen((char*)msg), &connStat[usrIndex], &peers[usrIndex]);
	logStat("SENT to %s: %s\n", (users[usrIndex].name == NULL)? users[usrIndex].addr:users[usrIndex].name, (char*)msg);
	free(begin);
	return 0;
      }
    }
    
    //receive login request
    else if(!strcmp(token, "LOGIN")){
      int i;
      if(LOGIN(newMsg)){
	char * name = (char *)malloc(strstr(newMsg, " ") - newMsg + 1);
	strcpy(name, strsep(&newMsg, " "));
	
	for(i=1;i<=nConns;i++){
	  if(users[i].name != NULL && !strcmp(users[i].name, name)){
	    RemoveConnection(i);
	    break;
	  }
	}
	users[usrIndex].name = (char *)malloc(strlen(name)); 
	strcpy(users[usrIndex].name, name);
	logStat("LOGIN success for user %s.", users[usrIndex].name);
	strcpy((char *)msg, users[usrIndex].name);
	strcat((char *)msg, " is online");
      }
      else{
	strcpy((char*)msg, "LOGIN failed\n");
      }
      free(begin);
      return 0;
    }
    
    //receive send public message request
    else if(!strcmp(token, "SEND")){
      //logStat("%s said: %s\n", users[usrIndex].name, newMsg);
      strcpy((char *)msg, (users[usrIndex].name==NULL)? users[usrIndex].addr:users[usrIndex].name);
      strcat((char *)msg, " said: ");
      strcat((char *)msg, newMsg);
      free(begin);
      return 0;
    }
      //receive send public message request
      else if(!strcmp(token, "PSEND")){
	if((token = strsep(&newMsg, " ")) != NULL){
	  int i;
	  int found = 0;
	  for(i=1;i<=nConns;i++){
	    if(!strcmp(token, users[i].name)){
	      strcpy((char *)msg, "PM from ");
	      strcat((char *)msg, users[usrIndex].name);
	      strcat((char *)msg, ":");
	      strcat((char *)msg, newMsg);
	      strcat((char *)msg, "\0");
	      found = 1;
	      pmsg = 1;
	      Send_NonBlocking(peers[i].fd, msg, strlen((char*)msg), &connStat[i], &peers[i]);
	      logStat("SENT to %s: %s\n", (users[i].name == NULL)? users[i].addr:users[i].name, (char*)msg);
	      break;
	    }
	  }
	  if(!found){
	    logStat("PSEND failed: user %s not found\n", token);
	    pmsg = 1;
	    strcpy((char *)msg, "PSEND user not found online\n");
	    Send_NonBlocking(peers[usrIndex].fd, msg, strlen((char*)msg), &connStat[usrIndex], &peers[usrIndex]);
	    logStat("SENT to %s: %s\n", (users[usrIndex].name == NULL)? users[usrIndex].addr:users[usrIndex].name, (char*)msg);
	  }
	}
	free(begin);
	return 0;
      }
    
    //some sort of problem with msg
    else{
      memset(msg, 0, MAX_MSG_SIZE);
      free(begin);
      return 0;
    }
  }
  return 0;
}

//recieves something from file descriptor(socket), nonblocking.
//Takes extra usrIndex integer as used in peers and connStat, needed for knowing who sent it to server
int Recv_NonBlocking(int sockFD, BYTE * data, int len, struct CONN_STAT * pStat, struct pollfd * pPeer, int usrIndex) {
  pStat->nRecv = 0;
  while (pStat->nRecv < len) {
    int n = recv(sockFD, data + pStat->nRecv, len - pStat->nRecv, 0);
    if (n > 0) {
      pStat->nRecv += n;
    } else if (n == 0 || (n < 0 && errno == ECONNRESET)) {
      Log("Connection closed.");
      close(sockFD);
      return -1;
    } else if (n < 0 && (errno == EWOULDBLOCK)) { //assuming all done
      //logStat("received something: %s\n", data);
      return (MsgHandle(data, usrIndex));
    } else {
      Error("Unexpected recv error %d: %s.", errno, strerror(errno));
    }
  }
  //  logStat("received something: %s\n", data);
  return 0;
}

//set a fd to non-blocking
void SetNonBlockIO(int fd) {
  int val = fcntl(fd, F_GETFL, 0);
  if (fcntl(fd, F_SETFL, val | O_NONBLOCK) != 0) {
    Error("Cannot set nonblocking I/O.");
  }
}

void DoServer(int svrPort, int maxConcurrency) {
  //create some needed variables: i for loop, listening socket
  //a BYTE array to store incoming msg
  int i;
  int listenFD = socket(AF_INET, SOCK_STREAM, 0);
  //make sure listener was made
  if (listenFD < 0) {
    Error("Cannot create listening socket.");
  }
  //set to nonblocking
  SetNonBlockIO(listenFD);
  
  //initialize internet structure
  struct sockaddr_in serverAddr;
  memset(&serverAddr, 0, sizeof(struct sockaddr_in));
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons((unsigned short) svrPort);
  serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

  //prepare data change options, bind and start listening.
  //check for any problems each step of the way
  int optval = 1;
  int r = setsockopt(listenFD, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
  if (r != 0) {
    Error("Cannot enable SO_REUSEADDR option.");
  }
  
  if (bind(listenFD, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) != 0) {
    Error("Cannot bind to port %d.", svrPort);
  }
  
  if (listen(listenFD, 16) != 0) {
    Error("Cannot listen to port %d.", svrPort);
  }
  
  //setup for peers for the listener
  nConns = 0;
  memset(peers, -1, sizeof(peers));
  peers[0].fd = listenFD;
  peers[0].events = POLLRDNORM;
  memset(connStat, 0, sizeof(connStat));

  while (1) {//the main loop
    //number of needy connections
    int nReady = poll(peers, nConns + 1, -1);
    //if it's negative there's some big problem
    if (nReady < 0) {
      Error("Invalid poll() return value.");
    }
    
    //create an internet structure for clients
    struct sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);
    
    //if something is trying to connect and we have space, accept and add into peer, users, connStat
    if ((peers[0].revents & POLLRDNORM) && (nConns < maxConcurrency)) {
      int fd = accept(listenFD, (struct sockaddr *)&clientAddr, &clientAddrLen);
      if (fd != -1) {
	SetNonBlockIO(fd);
	nConns++;
	peers[nConns].fd = fd;
	peers[nConns].events = POLLRDNORM;
	peers[nConns].revents = 0;
	
	//user connected add as a user connection
	users[nConns].addr = (char *)malloc(sizeof(char));
	if(inet_ntop(AF_INET, &(clientAddr.sin_addr), users[nConns].addr, clientAddrLen) == NULL){
	  Log("address is returned as NULL\n");
	}
	logStat("Address connection %d: %s %s\n", nConns, users[nConns].addr, users[nConns].name);
	memset(&connStat[nConns], 0, sizeof(struct CONN_STAT));
      }
      //if nothing go on
      if (--nReady <= 0) continue;
    }
    
    //go through each connection and recv if it needs it
    for (i=1; i<=nConns; i++) { //main read/write loop
      if (peers[i].revents & (POLLRDNORM | POLLERR | POLLHUP)) {
	int fd = peers[i].fd;
	//read request
	if (Recv_NonBlocking(fd, msg, MAX_MSG_SIZE, &connStat[i], &peers[i], i) < 0) {
	  RemoveConnection(i);
	  goto NEXT_CONNECTION;
	}
	else{
	  if(pmsg == 0){
	    int onlines;
	    for(onlines=1;onlines<=nConns;onlines++){
	      if(strlen((char*)msg) != 0){
		if (Send_NonBlocking(peers[onlines].fd, (BYTE *)msg, strlen((char*)msg)+1, &connStat[onlines], &peers[onlines]) < 0) {
		  RemoveConnection(onlines);
		  goto NEXT_CONNECTION;
		}
		else{
		  logStat("SENT to %s: %s\n", (users[onlines].name == NULL)? users[onlines].addr:users[onlines].name, (char*)msg);
		}
	      }
	    }
	  }
	  else{
	    pmsg = 0;
	    continue;
	  }
	}
      }
      //if this needs to resume sending, then do so
      if (peers[i].revents & POLLWRNORM) {
	int size = connStat[i].nRecv;
	if (Send_NonBlocking(peers[i].fd, msg, size, &connStat[i], &peers[i]) < 0 || connStat[i].nSent == size) {
	  RemoveConnection(i);
	  goto NEXT_CONNECTION;
	}
      }
      
      //NEXT_CONNECTION is so we don't go through all the code if no requests
    NEXT_CONNECTION:
      if (--nReady <= 0) break;
    }
  }
}

int main(int argc, char * * argv) {
  //make sure enough arguments
  if (argc != 3) {
    Log("Usage: %s [server Port] [max concurrency]", argv[0]);
    return -1;
  }
  
  //set arguments
  int port = atoi(argv[1]);
  int maxConcurrency = atoi(argv[2]);
  //call DoServer
  logStat("Waiting for connections...\n");
  DoServer(port, maxConcurrency);
  
  return 0;
}
