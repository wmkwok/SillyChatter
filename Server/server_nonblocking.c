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
#include "registration.h"

typedef unsigned char BYTE;
typedef unsigned int DWORD;
typedef unsigned short WORD;

#define MAX_REQUEST_SIZE 10000000
#define MAX_CONCURRENCY_LIMIT 64
#define MAX_MSG_SIZE 512

struct CONN_STAT {
  int size;//0 if unknown yet
  int nRecv;
  int nSent;
};

struct user{
  char * name;
  char * pass;
  char * addr;
};

int nConns;
struct pollfd peers[MAX_CONCURRENCY_LIMIT+1];
struct CONN_STAT connStat[MAX_CONCURRENCY_LIMIT+1];
struct user users[MAX_CONCURRENCY_LIMIT + 1];
/***********************************************utility functions**************************************/
void Error(const char * format, ...) {
  char msg[4096];
  va_list argptr;
  va_start(argptr, format);
  vsprintf(msg, format, argptr);
  va_end(argptr);
  fprintf(stderr, "Error: %s\n", msg);
  exit(-1);
}

void Log(const char * format, ...) {
  char msg[2048];
  va_list argptr;
  va_start(argptr, format);
  vsprintf(msg, format, argptr);
  va_end(argptr);
  fprintf(stderr, "%s\n", msg);
}

void CheckData(BYTE * buf, int size) {
  int i;
  for (i=0; i<size; i++) if (buf[i] != 'A' + i % 26) {
      Error("Received wrong data.");
    }
}

//after recv reads msg it is picked apart and
//executed by MsgHandle.
//TODO: break up command from parameters
int MsgHandle(char * msg, int usrIndex){
  char * cmd;
  if((cmd = strsep(&msg, " ")) != NULL){
    printf("msg handle parsed: %s\n", cmd);
  }
  else{
  if(!strcmp(msg, "CONNECT")) return 0;
  }
  if(!strcmp(cmd, "REGISTER")){
    REGISTER(cmd);
  }
  printf("using msg handler\n");
}

/****************************************Communication functions*************************************/
int Send_NonBlocking(int sockFD, const BYTE * data, int len, struct CONN_STAT * pStat, struct pollfd * pPeer) {
  
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

int Recv_NonBlocking(int sockFD, BYTE * data, int len, struct CONN_STAT * pStat, struct pollfd * pPeer, int usrIndex) {
  while (pStat->nRecv < len) {
    int n = recv(sockFD, data + pStat->nRecv, len - pStat->nRecv, 0);
    if (n > 0) {
      pStat->nRecv += n;
    } else if (n == 0 || (n < 0 && errno == ECONNRESET)) {
      Log("Connection closed.");
      close(sockFD);
      return -1;
    } else if (n < 0 && (errno == EWOULDBLOCK)) {
      printf("received something: %s\n", data);
      MsgHandle(data, usrIndex);
      return 0;
    } else {
      Error("Unexpected recv error %d: %s.", errno, strerror(errno));
    }
  }
  return 0;
}

void SetNonBlockIO(int fd) {
  int val = fcntl(fd, F_GETFL, 0);
  if (fcntl(fd, F_SETFL, val | O_NONBLOCK) != 0) {
    Error("Cannot set nonblocking I/O.");
  }
}

void RemoveConnection(int i) {
  close(peers[i].fd);
  if (i < nConns) {
    memmove(peers + i, peers + i + 1, (nConns-i) * sizeof(struct pollfd));
    memmove(connStat + i, connStat + i + 1, (nConns-i) * sizeof(struct CONN_STAT));
    memmove(users + i, users + i + 1, (nConns-i) * sizeof(struct user));
  }
  nConns--;
}

void DoServer(int svrPort, int maxConcurrency) {
  BYTE * buf = (BYTE *)malloc(MAX_REQUEST_SIZE);
  
  int listenFD = socket(AF_INET, SOCK_STREAM, 0);
  if (listenFD < 0) {
    Error("Cannot create listening socket.");
  }
  SetNonBlockIO(listenFD);
  
  struct sockaddr_in serverAddr;
  memset(&serverAddr, 0, sizeof(struct sockaddr_in));
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons((unsigned short) svrPort);
  serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

  //prepare data
  int i;
  BYTE msg[MAX_MSG_SIZE];

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
  
  nConns = 0;
  memset(peers, 0, sizeof(peers));
  peers[0].fd = listenFD;
  peers[0].events = POLLRDNORM;
  memset(connStat, 0, sizeof(connStat));
  
  int connID = 0;
  while (1) {//the main loop
    
    int nReady = poll(peers, nConns + 1, -1);
    
    if (nReady < 0) {
      Error("Invalid poll() return value.");
    }
    
    struct sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);
    
    if ((peers[0].revents & POLLRDNORM) && (nConns < maxConcurrency)) {
      int fd = accept(listenFD, (struct sockaddr *)&clientAddr, &clientAddrLen);
      if (fd != -1) {
	SetNonBlockIO(fd);
	nConns++;
	peers[nConns].fd = fd;
	peers[nConns].events = POLLRDNORM;
	peers[nConns].revents = 0;
	
	//users[i].addr = inet_ntoa(clientAddr.sin_addr);
	//printf("user address set: %s\n", users[i].addr);
	memset(&connStat[nConns], 0, sizeof(struct CONN_STAT));
      }
      
      if (--nReady <= 0) continue;
    }
    
    for (i=1; i<=nConns; i++) {
      if (peers[i].revents & (POLLRDNORM | POLLERR | POLLHUP)) {
	int fd = peers[i].fd;
	//read request
	if (connStat[i].nRecv < 4) {
	  
	  if (Recv_NonBlocking(fd, msg, MAX_MSG_SIZE, &connStat[i], &peers[i], i) < 0) {
	    RemoveConnection(i);
	    goto NEXT_CONNECTION;
	  }
	  
	  if (connStat[i].nRecv == 4) {
	    int size = connStat[i].size;
	    if (size <= 0 || size > MAX_REQUEST_SIZE) {
	      Error("Invalid size: %d.", size);
	    } 
	    Log("Transaction %d: %d bytes", ++connID, size);
	  }
	}
	
	//send response
	if (connStat[i].nRecv != 0) {
	  int size = connStat[i].nRecv;
	  if (Send_NonBlocking(fd, "received", size, &connStat[i], &peers[i]) < 0 || connStat[i].nSent == size) {
	    RemoveConnection(i);
	    goto NEXT_CONNECTION;
	  }
	}
      }
      
      if (peers[i].revents & POLLWRNORM) {
	int size = connStat[i].nRecv;
	if (Send_NonBlocking(peers[i].fd, msg, size, &connStat[i], &peers[i]) < 0 || connStat[i].nSent == size) {
	  RemoveConnection(i);
	  goto NEXT_CONNECTION;
	}
      }
      
    NEXT_CONNECTION:
      if (--nReady <= 0) break;
    }
  }
}

int main(int argc, char * * argv) {
  
  if (argc != 3) {
    Log("Usage: %s [server Port] [max concurrency]", argv[0]);
    return -1;
  }
  
  int port = atoi(argv[1]);
  int maxConcurrency = atoi(argv[2]);
  DoServer(port, maxConcurrency);
  
  return 0;
}

