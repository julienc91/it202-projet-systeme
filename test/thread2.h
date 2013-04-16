#ifndef __THREAD_H2__
#define __THREAD_H2__

#include <ucontext.h>
#include "queue.h"

#define TRUE 1
#define FALSE 0
#define STACK_SIZE 64*1024

typedef enum {READY, SLEEPING, DEAD} STATE;

typedef struct thread_t_
{
  /* state of the thread */
  STATE state;
  /* context of the thread*/
  ucontext_t context;
  /* contains pointers to next and previous node */
  TAILQ_ENTRY(thread_t_) entries;
  /* return value */
  void *retval;
  /* boolean */
  int already_done;
  int valgrind_stackid;
  
} *thread_t;

typedef struct Threads
{
  int isInitialized;
  thread_t mainThread;
  thread_t currentThread;
  TAILQ_HEAD(, thread_t_) list;
  TAILQ_HEAD(, thread_t_) list_sleeping;
  TAILQ_HEAD(, thread_t_) list_dead;

} Threads;

void thread_return();
void stock_return(void * funcarg, void* (*func)());
void threads_destroy();
void thread_init_function(void);




#endif /* __THREAD_H2__ */
