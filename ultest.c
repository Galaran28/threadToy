#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>

#include "ulthreads.c"

xstate_t back_to_future;
int limit = 0;

static void producer_func(void *xstate)
{
  printf("entering producer\n"); fflush(stdout);

  // do something

  limit++;
  if(limit == 3) {
    xstate_restore(&back_to_future);
  } else {
    xstate_restore((xstate_t*) xstate);
  }
}

static void client_func(void *xstate)
{
  printf("starting client\n"); fflush(stdout);

  // do something

  xstate_restore((xstate_t*) xstate);
}

int main(int argc, char *argv[])
{
  xstate_t pxstate, cxstate;
  void *pstack, *cstack;

  pstack = (void *) malloc(MINSIGSTKSZ);
  cstack = (void *) malloc(MINSIGSTKSZ);

  int rcP = xstate_create(&pxstate, producer_func, (void *) &cxstate, pstack, MINSIGSTKSZ);
  int rcC = xstate_create(&cxstate, client_func, (void *) &pxstate, cstack, MINSIGSTKSZ);

  xstate_switch(&back_to_future, &pxstate);

  printf("back again\n"); fflush(stdout);

  exit(0);
}
