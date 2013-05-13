#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include "thread.h"
#include "../test/pthread_test.h"

/* test de plein de switch par plein de threads
 *
 * la durée du programme doit etre proportionnelle au nombre de threads et de yields donnés en argument
 *
 * support nécessaire:
 * - thread_create()
 * - thread_yield() depuis ou vers le main
 * - retour sans thread_exit()
 * - thread_join() avec récupération de la valeur de retour
 */

static void * thfunc(void *_id)
{
  int id = (size_t) _id;
  int i = 0;

  while(1) {
    i++;
    
    fprintf(stderr, "[%2d] %d\n", id, i);
  }
    
  return NULL;
}

int main(int argc, char *argv[])
{
  set_preemption_active(1);

  int nbth, i, err;
  thread_t *ths;
  struct timeval tv1, tv2;
  unsigned long us;

  if (argc != 2) {
    printf("Usage: <nb threads>\n");
    return -1;
  }

  nbth = atoi(argv[1]);

  ths = malloc(nbth * sizeof(thread_t));
  assert(ths);

  gettimeofday(&tv1, NULL);

  for(i=0; i<nbth; i++) {
    err = thread_create(&ths[i], thfunc, (void*)(size_t)(i));
    assert(!err);
  }
  set_thread_priority(ths[1], 2);
  sleep(50);
  
  gettimeofday(&tv2, NULL);
  us = (tv2.tv_sec-tv1.tv_sec)*1000000+(tv2.tv_usec-tv1.tv_usec);
  printf("yield avec %d threads: %ld us\n", nbth, us);

  #ifdef DEBUG_MODE
  for(i=0; i<nbth; i++) {
    printf("%d: %d calls\n", i+1, ths[i]->nb_calls);
  }
  #endif

  free(ths);

  return 0;
}
