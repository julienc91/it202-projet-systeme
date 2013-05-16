#ifndef DEBUG_MODE
#define DEBUG_MODE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "thread.h"

#define NB 5
#define LOOP 100

void * thfunc(void* k) {
     int i=0;
     while(i<10*LOOP){
	  i++;
	  thread_yield();
     }
     thread_exit(NULL);
     return (void*) k;
}

int main(int argc, char **argv) {
     srand(time(NULL));
     thread_t *ths = malloc(NB*sizeof(thread_t));
     int i;
     for(i=0; i<NB; i++) {
	  thread_create(&ths[i], thfunc, (void *)(size_t)i);
	  set_thread_priority(ths[i], rand()%5+1);
     }
     //debug_priority();
     for(i=0; i<LOOP; i++) {
	  thread_yield();
     }
     printf("Thread priority:     ");
     for(i=0; i<NB; i++)
	  printf("| %d   ", ths[i]->default_priority);
     printf("|\n");
     printf("Nb calls per thread: ");
     for(i=0; i<NB; i++)
	  printf("| %d ", ths[i]->nb_calls);
     printf("|\n");

     for(i=0; i<NB; i++)
	  thread_join(ths[i], NULL);
  
     free(ths);
     return 0;
}
