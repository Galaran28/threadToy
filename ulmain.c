#include "ulthreads.c"

void foo(int n) {
  int i;

  printf("in foo, n is %d\n", n);

  for(i=0; i<n; i++) {
    printf("%d\n", i);
  }
}

int main() {
  xstate_t* xt;
  xt = malloc(sizeof(xstate_t));

  int *n = 10;
  char* stk[SIGSTKSZ];
  size_t stack_size = SIGSTKSZ;

  xstate_create(xt,(void*)&foo, n , stk, stack_size);
  printf("soooooo done?\n");
  return 0;
}

