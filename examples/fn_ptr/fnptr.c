#include <stdio.h>
#include <stdlib.h>
#define BUFF_SIZE 4
int myglob = 'a';

int get_myglob() { return myglob; }
int get_myglob_up() { return myglob + 'A' - 'a'; }
void set_myglob(int x) { myglob = x; }
void set_myglob_up(int x) { myglob = x + 'A' - 'a'; } 

int get_sign(int x) {
  char buff[BUFF_SIZE];
  if (x < 0)
    return 0;
  int (*get_charint)(void);
  if (x % 2 == 0)
    get_charint = get_myglob;
  else
    get_charint = get_myglob_up;
  for (int i = 0; i < BUFF_SIZE; i++) {
    buff[i] = get_charint() + i;
  }

  if (x > BUFF_SIZE)
    return -1;
  else
    return buff[x];
}

int main(int argc, char *argv[]) {
  int a;
  a = atoi(argv[1]);
  void (*set_charint)(int);
  set_charint = set_myglob;
  set_charint('a' + a);
  int result = get_sign(a);
  printf("%d\n", result);
  return 0;
}