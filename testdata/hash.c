#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdbool.h>

//we might not be able to limit this so low
#define MAXCONN 20

struct tcpConn{
  int * tuple;
  int data;
};

struct tcpConn * hashTable[MAXCONN];
struct tcpConn * node;

//returns where this key would be stored
int hashIdx(int * key){
  return key[0] % MAXCONN;
}

int equals(int * elem, int * key){
  int i;
  for(i=0;i<4;i++){
    if(elem[i] != key[i])
      return 0;
  }
  return 1;
}

struct tcpConn * search(int * key){
  //get the hash
  int idx = hashIdx(key);
  
  //go through the array
  while(hashTable[idx] != NULL){
    //if we've found the node, return it
    if(equals(hashTable[idx]->tuple, key))
      return hashTable[idx];
    
    //else keep going
    idx++;
    idx %= MAXCONN; //why do we need this?
  }
  //nothing is found so return NULL
  return NULL;
}

//finds a spot for the object
void insert(int * key, int data){
  //create the conn struct
  struct tcpConn * conn = (struct tcpConn*)malloc(sizeof(struct tcpConn));
  conn->tuple = key;
  conn->data = data;
  
  //if it's already in the hash then dont insert again
  if(search(key) != NULL){

  int idx = hashIdx(key);
  //check the spot for NULL or go to next cell
  while(hashTable[idx] != NULL && hashTable[idx]->tuple[0] != -1){
    idx++;
    idx %= MAXCONN;
  }
  hashTable[idx] = conn;
  }
}

struct tcpConn * delete(struct tcpConn * conn){
  int key = conn->tuple;
  int idx = hashIdx(key);

  //go through the array
  while(hashTable[idx] != NULL){
    //if we found the node, change the key number to -1
    if(equals(hashTable[idx]->tuple, key)){
      hashTable[idx]->tuple[0] = -1;
      return hashTable[idx];
    }
    idx++;
    idx %= MAXCONN;
  }
  //if we can't find it...
  return NULL;
}

void displayTable(){
  int i = 0;
  for(i=0;i<MAXCONN;i++){
    if(hashTable[i] != NULL)
      printf(" ({%d, %d, %d, %d}: %d)", hashTable[i]->tuple[0], hashTable[i]->tuple[1], hashTable[i]->tuple[2], hashTable[i]->tuple[3], hashTable[i]->data);
    /* else */
    /*   printf(" NULL "); */
  }
  printf("\n");
}

int countTable(){
  int i;
  int count = 0;
  for(i=0;i<MAXCONN;i++){
    if(hashTable[i] != NULL && hashTable[i]->tuple[0] != -1)
      count++;
  }
  return count;
}

int main(){
  struct tcpConn * tmpNode = (struct tcpConn*)malloc(sizeof(struct tcpConn));
  tmpNode->tuple = (int *)malloc(sizeof(int)*4);
  tmpNode->key[0] = -1;
  tmpNode->data = -1;
  printf("Hello world!\n");

  insert({1, 2, 3, 4}, 20);
  insert({2, 3, 4, 5}, 70);
  insert({0, 1, 2, 3}, 80);
  insert({3, 4, 5, 6}, 25);
  insert({4, 5, 6, 7}, 44);
  insert({5, 6, 7, 8}, 32);
  insert({6, 7, 8, 9}, 11);
  insert({7, 8, 9, 0}, 78);
  insert({8, 9, 0, 1}, 97);

  displayTable();
  /* tmpNode = search(37); */
  /* printf("size is %d\n", countTable()); */
  /* if(tmpNode != NULL){ */
  /*   printf("Element found: %d for key %d\n", tmpNode->data, tmpNode->key); */
  /* } */
  /* else{ */
  /*   printf("Element not found\n"); */
  /* } */
  
  /* delete(tmpNode); */
  /* tmpNode = search(37); */
  /* printf("size is %d\n", countTable()); */
  /* displayTable(); */
  /* if(tmpNode != NULL){ */
  /*   printf("Element found: %d for key %d\n", tmpNode->data, tmpNode->key); */
  /* } */
  /* else{ */
  /*   printf("Element not found\n"); */
  /* } */

  /* int test[4] = {1, 2, 3, 4}; */
  /* int test1[4] = {1, 2, 3, 4}; */
  /* int test2[4] = {2, 3, 4, 5}; */
  /* int ret1 = equals(&test[0], &test1[0]); */
  /* int ret2 = equals(&test[0], &test2[0]); */

  /* printf("testing for True: %d\n", ret1); */
  /* printf("Testing for False: %d \n", ret2); */
}
