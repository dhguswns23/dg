#include <stdio.h>
#include <stdlib.h>
#define BUFF_SIZE 4
int myglob = 'a';

int get_sign(int x) {
  char buff[BUFF_SIZE];
  if (x < 0)
    return 0;
  for (int i = 0; i < BUFF_SIZE; i++) {
    buff[i] = myglob + i;
  }

  if (x > BUFF_SIZE)
    return -1;
  else
    return buff[x];
}

int main(int argc, char * argv[]) {
  int a;
  a = atoi(argv[1]);
  int result = get_sign(a);
  printf("%d\n", result);
  return 0;
}