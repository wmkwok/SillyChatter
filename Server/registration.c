#include <stdio.h>
#include <stdlib.h>

//takes username and pass and records into file
// so far the the mode is readable writable overwrite w+. Change to a+ for no overwrite.
int REGISTER(char *registration){
  FILE *fp;
  fp = fopen("user.txt", "a+");
  fputs(registration+9, fp);
  fputs("\n", fp);
  fclose(fp);
  printf("should be registering %s\n", registration+9);
  return 0;
}

int UNREGISTER(char * username){
  printf("should be de-registering...\n");
  return 0;
}
