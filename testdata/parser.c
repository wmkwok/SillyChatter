#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "structures.h"

#define MAX_CONN_SIZE 1024

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
  struct ethHdr eHdr;
  struct pkt buf;
  struct pkt p;
  struct tcpHdr tHdr;
  struct tcpHdr tcpp;
  struct tcpConn tcpConns[MAX_CONN_SIZE];
  uint8_t ihlLen;
  uint32_t word;

  /**************************************global headers*****************************************/
  printf("Read status: %i\n", fread(&gblHdr, 1, sizeof(gblHdr),  stdin));
  /* printf("feof return status: %i \n", feof(stdin)); */
  /* printf("Global Header: \n"); */
  /* printf("Magic Number: %x \n", gblHdr.magicNumber); */
  /* printf("Version Major: %i \n", gblHdr.versionMajor); */
  /* printf("Version Minor: %i \n", gblHdr.versionMinor); */
  /* printf("thisZone: %i \n", gblHdr.thisZone); */
  /* printf("sigFigs: %i\n", gblHdr.sigFigs); */
  /* printf("snapLen: %i\n", gblHdr.snapLen); */
  /* printf("network %i\n", gblHdr.network); */
  /**********************************************************************************************/

  //read through all pcap files
  while(fread(&recHdr, 1, sizeof(recHdr), stdin) > 0){
    //printf("packet no.: %i \ntimestamp: %i:%i \nincLen: %i \norigLen: %i \n", numPkt, recHdr.sec, recHdr.usec, recHdr.inclLen, recHdr.origLen);

    fseek(stdin, 12, SEEK_CUR); //seek past the dest and src
    fread(&eHdr, 1, 2, stdin); //read in the type
    eHdr.type = ntohs(eHdr.type); //convert the number

    fread(&buf, 1, sizeof(struct pkt), stdin); //read pkt header
    extract_header((uint32_t *)&buf, &p); //call function that converts the numbers

    /*--------------------------------------showing ETH/IP Hdr-----------------------------------------*/
    /* printf("Ethernet Frame type: %x\n", eHdr.type); */
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
    if(p.version == 4 && eHdr.type == 2048){
      numIP++;
      //TCP
      if(p.protocol == 6)
    	numTCP++;
      //UDP
      if(p.protocol == 17)
    	numUDP++;

      if(p.version == 4){
	//check for options and seek past it
	int opt = (p.ihl-5)*4;
	fseek(stdin, opt, SEEK_CUR);

	if(p.protocol == 6){ //TCP packet
	  read(&tHdr, 1, sizeof(struct tcpHdr), stdin); //read to tcp header
	  /**************************print TCP header**************************************************/
	  printf("packet no.: %i \ntimestamp: %i:%i \nincLen: %i \norigLen: %i \n", numPkt, recHdr.sec, recHdr.usec, recHdr.inclLen, recHdr.origLen);
	  
	  printf("TCP src port: %x\n", ntohs(tHdr.srcPrt));
	  printf("TCP dest port: %x\n", ntohs(tHdr.destPrt));
	  printf("TCP seq num: %x\n", ntohs(tHdr.seq));
	  printf("TCP ack num: %x\n", ntohs(tHdr.ack));
	  printf("TCP offset: %x, reserved: %x, flags: %x\n", tHdr.offset, tHdr.reserved, tHdr.flags);
	  printf("TCP winSize: %x\n", ntohs(tHdr.winSize));
	  printf("TCP checksum: %x\n", ntohs(tHdr.checksum));
	  printf("TCP urgPtr: %x\n", ntohs(tHdr.urgPtr));
	  /*******************************************************************************************/
	  load_tcp_hdr((uint32_t *)&tHdr, &tcpp);
	}
	else if(p.protocol == 17){ //UDP packet. SEEK past it?
	  //maybe ignore and seek past it all together afterwards...
	}
	//seek through the rest of the packet for now?
	fseek(stdin, (long)recHdr.inclLen - 14 - sizeof(struct pkt) - opt -  (p.version == 6? sizeof(struct tcpHdr): 0), SEEK_CUR);       
      } 
    }
    
    else{
      //printf("Unrecognized pkt version number %d at %d,%d: \n", p.version, recHdr.sec, recHdr.usec);
      //seek through the rest of the packet for now?
      fseek(stdin, (long)recHdr.inclLen - 14 - sizeof(struct pkt), SEEK_CUR); 
    }
  }
  printf("size of p %i \n", sizeof(p));
  printf("Total Packets: %i \nTotal IP Packets: %i\nTotal TCP: %i\nTotal UDP: %i\n", numPkt, numIP, numTCP, numUDP);
}

int load_tcp_hdr(uint32_t *buf, struct tcpHdr *hdr) {

  if (buf != NULL) {
    memcpy(hdr, buf, sizeof(struct tcpHdr));
  }

  hdr->srcPrt = ntohs(hdr->srcPrt);
  hdr->destPrt = ntohs(hdr->destPrt);
  hdr->seq = ntohl(hdr->seq);
  hdr->ack = ntohl(hdr->ack);
  /* data_offset (4 bits) is unchanged
   * reserved (3 bits) is unchanged
   * flags (9 bits) TODO
   */
  
  hdr->winSize = ntohs(hdr->winSize);
  hdr->checksum = ntohs(hdr->checksum);
  hdr->urgPtr = ntohs(hdr->urgPtr);

  return 0;
}

//haven't tested insert
void insert(uint32_t seq, uint8_t * data, struct tcpGroup * grp){
  struct tcpSeg * node = (struct tcpSeg *)malloc(sizeof(struct tcpSeg));
  node->seq = seq;
  node->data = data;

  struct tcpSeg * ptr = grp->head;
  while(ptr != NULL){
    if(seq > ptr->seq){
      ptr->prev->next = node; //set the first one to the node
      node->prev = ptr->prev; //set the node to the first one
      node->next = ptr;       //set the node to the second node
      ptr->prev = node;       //set the second node to the tmp node
    }
  }

}

void t2(){
  printf("T2\n");
}

int main(){
  t1();
  return 0;
}

