#include <stdio.h>
#include <stdint.h>
#include "hash.h"

//https://www.codingunit.com/c-tutorial-binary-file-io
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

struct pkt
{
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
  uint32_t sourcePrt:16, destPrt:16;
  uint32_t seq;
  uint32_t ack;
  uint16_t offset:4, reserved:3, flags:9;
  uint16_t winSize;
  uint16_t checksum;
  uint16_t urgPtr;
};

void ntoh(uint32_t * ptr, size_t byte){
  uint32_t word;
  int offset;
  for(offset = 0; offset < byte/4; offset++){
    word = ntohl(*(ptr+offset));
    *(ptr+offset) = word;
  }
}

void extract_header(uint32_t * ptr, struct pkt *p){
  uint32_t word;
  ntoh(ptr, sizeof(struct pkt));

  word = *ptr;
  p->version = (word >> 28) & 0x0f;
  p->ihl = (word >> 24) & 0x0f;
  p->dscp = (word >> 18) & 0x3f;
  p->ecn = (word >> 16) & 0x03;
  p->totalLen = word & 0xffff;

  word = *(ptr + 1);
  p->identification = (word >> 16) & 0xffff;
  p->flags = (word >> 13) & 0x07;
  p->fragOffset = word & 0x1fff;

  word = *(ptr + 2);
  p->timeToLive = (word >> 24) & 0xff;
  p->protocol = (word >> 16) & 0xff;
  p->checksum = word & 0xffff;
    
  word = *(ptr + 3);
  p->sourceAddr = word;
  word = *(ptr + 4);
  p->destAddr = word;

}
void t1(){
  int numPkt = 0; //number of packets traced
  int numIP = 0; //number of IP packets
  int numTCP = 0; //number of TCP packets
  int numUDP = 0; //number of UDP packets
  int conns = 0; //number of unique TCP connections
  struct pcapHdr gblHdr;
  struct pktHdr recHdr;
  struct pkt buf;
  struct pkt p;
  uint8_t ihlLen;
  uint32_t word;

  //read the global header of PCAP file
  printf("Read status: %i\n", fread(&gblHdr, 1, sizeof(gblHdr),  stdin));
  printf("feof return status: %i \n", feof(stdin));
  printf("Global Header: \n");
  printf("Magic Number: %x \n", gblHdr.magicNumber);
  printf("Version Major: %i \n", gblHdr.versionMajor);
  printf("Version Minor: %i \n", gblHdr.versionMinor);
  printf("thisZone: %i \n", gblHdr.thisZone);
  printf("sigFigs: %i\n", gblHdr.sigFigs);
  printf("snapLen: %i\n", gblHdr.snapLen);
  printf("network %i\n", gblHdr.network);

  //read through all pcap files
  while(fread(&recHdr, 1, sizeof(recHdr), stdin) > 0){
    //printf("packet no.: %i \ntimestamp: %i:%i \nincLen: %i \norigLen: %i \n", numPkt, recHdr.sec, recHdr.usec, recHdr.inclLen, recHdr.origLen);
    fseek(stdin, 14, SEEK_CUR); //seek past ethernet layer
    fread(&buf, 1, sizeof(struct pkt), stdin);
    extract_header((uint32_t *)&buf, &p);


    /*--------------------------------------showing IP Hdr-----------------------------------------*/
    /* printf("Packet no: %i\n", numPkt); */
    /* printf("Version: %x\n", p.version); */
    /* printf("ihl: %x\n", p.ihl); */
    /* printf("dscp:0x%x\n", p.dscp); */
    /* printf("ecn: %x\n", p.ecn); */
    /* printf("totalLen: %i\n", p.totalLen); */
    /* printf("identification: 0x%x\n", p.identification); */
    /* printf("flags: 0x%x\n", p.flags); */
    /* printf("fragOffset: %i\n", p.fragOffset); */
    /* printf("timeToLive: %i\n", p.timeToLive); */
    /* printf("protocol: %i\n", p.protocol); */
    /* printf("checksum: 0x%x\n", p.checksum); */
    /* printf("source: 0x%x\n", p.sourceAddr); */
    /* printf("dest: 0x%x\n\n", p.destAddr); */
    /*-------------------------------------------------------------------------------------*/

    numPkt++;    
    //only care if it is ipv4 packet
    if(p.version == 0x04){
      numIP++;
      //TCP
      if(p.protocol == 0x06)
    	numTCP++;
      //UDP
      if(p.protocol == 0x11)
    	numUDP++;
    }
    
    if(p.version != 0x04){
      printf("Unrecognized pkt version number %d: \n", p.version);
      //seek through the rest of the packet for now?
      fseek(stdin, (long)recHdr.inclLen - 14 - sizeof(struct pkt), SEEK_CUR); 
    }

    else if(p.version == 0x04){
      //check for options and seek past it
      int opt = (p.ihl-5)*4;
      if(p.ihl > 5){
	fseek(stdin, (long)opt, SEEK_CUR);
      }

      if(p.protocol == 0x06){
	
      }
      else if(p.protocol == 0x11){

      }
      //seek through the rest of the packet for now?
      fseek(stdin, (long)recHdr.inclLen - 14 - sizeof(struct pkt) - opt, SEEK_CUR);       
    }
  }
  printf("size of p %i \n", sizeof(p));
  printf("Total Packets: %i \nTotal IP Packets: %i\nTotal TCP: %i\nTotal UDP: %i\n", numPkt, numIP, numTCP, numUDP);
}

void t2(){
  printf("T2\n");
}

int main(){
  t1();
  return 0;
}

