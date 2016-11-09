struct tcpConn{
  int key;
  int data;
};

int hashIdx(int key);
struct tcpConn * search(int key);
void insert(int key, int data);
struct tcpConn * delete(struct tcpConn * conn);
void displayTable(void);
