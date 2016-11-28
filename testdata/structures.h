#define MAX_CONN_SIZE 1024

struct pcapHdr{
  uint32_t magicNumber;
  uint16_t versionMajor;
  uint16_t versionMinor;
  int32_t thisZone;
  uint32_t sigFigs;
  uint32_t snapLen;
  uint32_t network;
};

struct pktHdr{
  uint32_t sec;
  uint32_t usec;
  uint32_t inclLen;
  uint32_t origLen;
};

struct ethHdr{
  uint16_t type;
  /* uint8_t src; */
  /* uint8_t dest; */
  /* uint8_t type; */
};

struct pkt{
  uint32_t version:4, ihl:4, dscp:6, ecn:2, totalLen:16;
  uint32_t identification:16, flags:3, fragOffset:13;
  uint32_t timeToLive:8, protocol:8, checksum:16;
  uint32_t sourceAddr;
  uint32_t destAddr;
  /* uint32_t opt1; */
  /* uint32_t opt2; */
  /* uint32_t opt3; */
  /* uint32_t opt4; */
};

struct tcpHdr{
  uint16_t srcPrt;
  uint16_t destPrt:16;;
  uint32_t seq;
  uint32_t ack;
  uint16_t offset:4, reserved:4, flags:8;
  uint16_t winSize;
  uint16_t checksum;
  uint16_t urgPtr;
};

//tcp connection info, direction of information is important
struct tcpConn{ 
  uint32_t srcIP;
  uint32_t destIP;
  uint32_t srcPort;
  uint32_t destPort;
  struct tcpSeg * head;
};

//a node of a part of one connection
struct tcpSeg{
  uint32_t seq;
  uint8_t * data;
  struct tcpSeg * prev;
  struct tcpSeg * next;
};

//the whole entire linked list of tcp datas, with the connection info structure
struct tcpGroup{
  struct tcpConn conn;
  struct tcpSeg * head;
};

int numPkt = 0; //number of packets traced
int numIP = 0; //number of IP packets
int numTCP = 0; //number of TCP packets
int numUDP = 0; //number of UDP packets
int conns = 0; //number of unique TCP connections
int dataLen = 0;
struct pcapHdr gblHdr;
struct pktHdr recHdr;
struct ethHdr eHdr;
struct pkt buf;
struct pkt p;
struct tcpHdr tHdr;
struct tcpHdr tcpp;
struct tcpConn tcpConns[MAX_CONN_SIZE];
struct tcpConn newConn;
struct tcpSeg newSeg;
uint8_t ihlLen;
uint32_t word;
