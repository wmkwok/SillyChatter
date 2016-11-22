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
  uint32_t srcPrt:16, destPrt:16;
  uint32_t seq;
  uint32_t ack;
  uint16_t offset:4, reserved:3, flags:9;
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
