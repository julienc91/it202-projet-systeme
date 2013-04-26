#ifndef DEBUG_MODE
#define DEBUG_MODE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "thread.h"

#define MAX_PRIORITY 5
#define NB 5
#define LOOP 100

thread_t *ths;

void * thfunc(void* k) {
  static int count = 0;
  int i = 1;
  while(1){
    count++;
    if(i==LOOP) {
      thread_t t = thread_self();
      printf("Thread %d ended at %dth loop (priority %d)\n", t->id, count, t->default_priority);
      printf("\tPriority-Nb calls: ");
      for(i=0; i<NB; i++)
	printf(" %d-%d  ", ths[i]->default_priority, ths[i]->nb_calls);
      printf("\n");
      thread_exit(k);
    }
    thread_yield();
    i++;
  }
  return (void*) k;
}

int main(int argc, char **argv) {
  srand(time(NULL));
  ths = malloc(NB*sizeof(thread_t));
  int i;
  for(i=0; i<NB; i++) {
    thread_create(&ths[i], thfunc, (void*)(size_t)i);
    set_thread_priority(ths[i], rand()%MAX_PRIORITY+1);
  }
  //debug_priority();
  for(i=0; i<NB*LOOP; i++) {
    thread_yield();
  }
  free(ths);
  return 0;
}
