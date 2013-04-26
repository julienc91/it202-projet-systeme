#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <pthread.h>

#include "thread.h"
#include "../test/pthread_test.h"
#define MAX_THREAD 4096 * 4096 //1024 * 1024

pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;
int tab[MAX_THREAD];
int res;
int N;


void *sum_init(void *id){
  int id_self = (size_t) id;
  int i;

  for(i = 0; i < MAX_THREAD/N; i++){
    pthread_mutex_lock(&mut);
    res += tab[N * id_self + i];
    pthread_mutex_unlock(&mut);
  }
  return NULL;
}


int main(int argc, char * argv[]){

  int i;

  if(argc != 2){
    printf("mauvais nombre d'arguments");
    return EXIT_FAILURE;
  }

  N = atoi(argv[1]);
  printf("N = %d\n", N);
  thread_t tid[N];
  //int id[N];
  i =N;

  while(i != 1 && i != 2){
    i = i/2;
  }

  //~printf("i = %d\n", i);

  if(i != 2){
    printf("donner un nombre de threads de la forme 2^n");
    return EXIT_FAILURE;
  }

  if(N > MAX_THREAD){
    printf("nombre de thread trop grand");
    return EXIT_FAILURE;
  }

  for(i=0; i<MAX_THREAD; i++){
    tab[i] = 1;
  }

  for(i=0; i<N ; i++){
    tid[i] = 
#ifndef PTHREAD
      (void *)(size_t)
#endif
      i;
    thread_create(&tid[i], sum_init, (void *)(size_t)i);
  }

  void * pt;
  for (i=0; i < N; i++){
    thread_join(tid[i], &pt);
  }

  printf("Total attendu:1048576\n");
  printf("Total des éléments du tableau: %d\n", res);
  return EXIT_SUCCESS;
}
