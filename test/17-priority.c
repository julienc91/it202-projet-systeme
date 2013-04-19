#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "thread.h"

#define NB 2

static void * thfunc(void* k) {
  int i = (int) k;
  while(1) {
    i++;
    thread_yield();
  }
  return (void*) i;
}

int main(int argc, char **argv) {
  srand(time(NULL));
  thread_t *ths = malloc(NB*sizeof(thread_t));
  int i;
  for(i=0; i<NB; i++) {
    thread_create(&ths[i], thfunc, NULL);
    set_thread_priority(ths[i], rand()%5+1);
  }
  debug_priority();
  while(1) {
    thread_yield();
  }
  free(ths);
  return 0;
}
