// Chris Sleys and Jon Trotter, first implementation of user level threads
#ifndef ULTHREADS
#define ULTHREADS

#include <stdlib.h>
#include<stdio.h>
#include<setjmp.h>
#include<signal.h>
#include<unistd.h>

#define xstate_switch(old_xstate, new_xstate) \
  if(setjmp((old_xstate)->jb) == 0) \
    longjmp(new_xstate->jb,1)

#define xstate_save(xstate) \
  setjmp(xstate->jb)

#define xstate_restore(xstate)\
  longjmp(xstate->jb,1)

typedef struct xstate_t{
  jmp_buf jb;
} xstate_t;

xstate_t* caller_xstate;
void  (* worker)(void*);
void* worker_args;

stack_t saved_sigStack;
stack_t alt_sigStack;

sigset_t saved_mask;
sigset_t alt_mask;

struct sigaction saved_sigaction;
struct sigaction alt_sigaction;

int already_captured = 0;
xstate_t* capture_state;
xstate_t* create_state;
xstate_t* boot_state;


//calls threads  method
void xstate_boot(void)
{
  //printf("in boot\n");fflush(0);
  //(11)
  void(*t_main)(void*) = worker;
  void* t_arg = worker_args;
  boot_state = malloc(sizeof(xstate_t));

  //(10)       //set signal mask to original signal mask
  sigprocmask(SIG_SETMASK,&saved_mask,NULL);

  //printf("calling func\n");fflush(0);
  t_main(t_arg);
  //printf("returned from func\n");fflush(0);
  //(12-13)   //go back to capture_state
}


void xstate_capture_stack(int sig)//sig handler method
{
  capture_state = malloc(sizeof(xstate_t));
  xstate_save(capture_state);
  //(5) //capture the state
  if( !already_captured )
  //set flag for return
  {
    already_captured = 1;
    return;
  }

  //printf("in capstack again\n")	;fflush(0);
  //(9)        //call boot func
  xstate_boot();
  //printf("back from boot\n")	;fflush(0);
  return;
}

void xstate_create(xstate_t* Caller_Xstate, void (*thread_main)(void*),void *thread_main_arg, void* stack, size_t stack_size)
{   																							//creates thread
  sigemptyset(&alt_mask);                                          	//(1)		//create empty sig_set
  sigaddset(&alt_mask, SIGUSR1);                            				// add SIGUSR1 signal to set
  sigprocmask(SIG_BLOCK, &alt_mask,&saved_mask);     		//block  signals in mask, save old mask in saved_mask

  alt_sigaction.sa_handler = xstate_capture_stack;        //(2)      //define what our alternate sig action will do
  alt_sigaction.sa_flags  = SA_ONSTACK;									//sigaction to happen on alt stack
  sigaction(SIGUSR1, &alt_sigaction,&saved_sigaction); 			//replace  old sig action for SIGUSR1 with alternate. save old action.

  alt_sigStack.ss_size = stack_size;                           //(3)       //set size of alt_sigStack to match that of size passed as arg
  alt_sigStack.ss_sp = stack;												   //set stack * to stack
  alt_sigStack.ss_flags = 0;
  sigaltstack(&alt_sigStack,&saved_sigStack);						  // switch default sigStack w/ new sigStack- save default sigStack

  worker = thread_main;									 		//(4)       //save params in globel vars.
  worker_args = thread_main_arg;
  caller_xstate = Caller_Xstate;

  kill(0,SIGUSR1);																 //send SIGUSR1 signal
  sigprocmask(SIG_UNBLOCK,&alt_mask,NULL);					 //allow signal through

  //xstate_capture_stack called through signal handler here

  //returned from xstate_capture_stack w/state saved

  sigaltstack(&saved_sigStack,NULL);									//(6) restore old signal stack
  sigaction(SIGUSR1,&saved_sigaction,NULL);						//associate old sigaction w/ SIGUSR1
  sigprocmask(SIG_SETMASK,&saved_mask,NULL);			//restore old mask

  create_state = malloc(sizeof(xstate_t));								//(7) init create_state
  xstate_switch(create_state,capture_state);
  //(8) save current state, then jump to saved capture_state

  //printf("back to xstate_create\n")	;fflush(0);
}
#endif
