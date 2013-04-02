#include "../src/thread.h"

extern thread_t thread_self(void){
  
  return (thread_t) 0;
}

extern int thread_create(thread_t *newthread, void *(*func)(void *), void *funcarg){

  return 0;
}

extern int thread_yield(void){

  return 0;
}

extern int thread_join(thread_t thread, void **retval){
  
  return 0;
}

extern void thread_exit(void *retval) {

  for (;;);
}

