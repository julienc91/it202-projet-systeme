#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <pthread.h>

#include "thread.h"
#define MAX_THREAD 1024 * 1024

pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;
int tab[MAX_THREAD];
int res;
int N;


void * sum_init(void * id){
    int id_self = (int )id;
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
    thread_t tid[N];
	int id[N];


    if(N%2!=0){
        printf("donner un nombre de threads pair");
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
        tid[i] = i;
        thread_create(&tid[i], sum_init, (void *) i);
    }

    void * pt;
	for (i=0; i < N; i++){
		thread_join(tid[i], &pt);
	}

    printf("Total attendu:1048576\n", MAX_THREAD);
    printf("Total des éléments du tableau: %d\n", res);
    return EXIT_SUCCESS;
}
