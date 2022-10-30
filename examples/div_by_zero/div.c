#include <stdio.h>
#include <stdlib.h>

int test(int x) { return x; }

int get_sign(int x) {
  if (x == 1)
     return 0;
  
  if (x < 0) {
    return -1;
  } else {
    return test(60 / (x - 2));
  }
}

int main(int argc, char * argv[]) {
  int a;
  a = atoi(argv[1]);
  int result = get_sign(a);
  printf("%d\n", result);
  return 0;
}