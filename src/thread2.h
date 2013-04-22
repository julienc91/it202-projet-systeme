#ifndef __THREAD_H2__
#define __THREAD_H2__

#include <ucontext.h>
#include "queue.h"

#define TRUE 1
#define FALSE 0
#define STACK_SIZE 64*1024
#define DEFAULT_PRIORITY 1

typedef enum {READY, SLEEPING, DEAD} STATE;

typedef struct thread_t_
{
  #ifdef DEBUG_MODE
  int id;
  int nb_calls;
  #endif
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
  int default_priority;
  int current_priority;

} *thread_t;

typedef struct Threads
{
  int max_priority;
  int isInitialized;
  thread_t mainThread;
  thread_t currentThread;
  TAILQ_HEAD(, thread_t_) list;
  TAILQ_HEAD(, thread_t_) list_sleeping;
  TAILQ_HEAD(, thread_t_) list_dead;

  pthread_t *tab;

} Threads;

int set_thread_priority(thread_t thread, int priority);
void thread_return();
void stock_return(void * funcarg, void* (*func)());
void threads_destroy();
void thread_init_function(void);
int get_cores(void);
void debug_priority();
void update_max_priority();



#endif /* __THREAD_H2__ */
