#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <pthread.h>
#include <time.h>

#include "thread.h"

int N = 64;
int * tab_pt;
int j = 2;          //taille des sous tableaux
int k;      //nombre des sous tableaux
int wait = 0;
pthread_mutex_t mut= PTHREAD_MUTEX_INITIALIZER;

void print_tableau(void){
    int i;
    for(i = 0; i < N * 2; i++){
        printf("%d,", tab_pt[i]);
    }
    printf("\n");
}

void * tri_2_a_2(void * id){

    int tmp=0;

    if(tab_pt[(size_t)id * 2] > tab_pt[(size_t)id * 2 +1]){
        tmp = tab_pt[(size_t)id * 2];
        tab_pt[(size_t)id * 2] = tab_pt[(size_t)id * 2 + 1];
        tab_pt[(size_t)id * 2 + 1] = tmp;
    }

    return (void *) &tab_pt;
}

int tri_lin_rec(int id, int taille_g, int taille_d, int * tab_tmp){
    //int i;
    if(taille_d == 0 && taille_g == 0){
        //~for (i = 0; i < 2 * j; i++){
        //~printf("%d,", tab_tmp[i]);
        //~}
        //~printf("\n");
        return 1;
    }

    int g = tab_pt[id * 2 * j + j - taille_g];
    int d = tab_pt[id * 2 * j + 2*j  - taille_d];

    if(taille_d < 1){
        tab_tmp[2 * j - taille_g] = g;
        //printf("1:%d:%d\n",tab_tmp[2 * j - taille_g],g);
        taille_g--;

    }
    else if (taille_g < 1){
        tab_tmp[2 * j - taille_d] = d;
        //printf("2:%d:%d\n",tab_tmp[2 * j - taille_d],d);
        taille_d--;

    }
    else if(g < d){
        tab_tmp[2 * j - taille_g - taille_d] = g;
        //printf("3\%dn",tab_tmp[2 * j - taille_g - taille_d]);
        taille_g--;
    }
    else{
        tab_tmp[2 * j - taille_g - taille_d] = d;
        //printf("4:%d\n",tab_tmp[2 * j - taille_g - taille_d] );
        taille_d--;
    }

    tri_lin_rec(id, taille_g, taille_d, tab_tmp);

    return 1;
}

void * tri_lineaire(void * id){
    //printf("id:%d\n", (int) id);
    int tab_tmp[2 * j];
    int i;
    for (i = 0; i < 2 * j; i++){
        tab_tmp[i] = 0;
    }

    tri_lin_rec((size_t)id, j, j, tab_tmp);
    //printf("tab_tmp:\n");
    //~for (i = 0; i < 2 * j; i++){
        //~printf("%d,", tab_tmp[i]);
    //~}
    //~printf("\n");
    for (i = 0; i < 2 * j; i++){
        tab_pt[(size_t)id * 2 * j + i] = tab_tmp[i];
    }
    //print_tableau();

    pthread_mutex_lock(&mut);
    wait++;
    pthread_mutex_unlock(&mut);

    return (void *) 1;
}

int fusion(){
    int i;
    //print_tableau();
    thread_t tid[N];
    k = N;

    while(k > 1){
        wait = 0;

        for(i = 0; i < (k / 2); i++){
	  thread_create(&tid[i], tri_lineaire, (void *)(size_t)i);
        }

        for(i = 0; i < (k / 2); i++){
            thread_join(tid[i], NULL);

        }

        j = j * 2;
        k = k / 2;
        while(1){
            pthread_mutex_lock(&mut);
            if(wait == k)
                break;
        }
        pthread_mutex_unlock(&mut);
    }

    return 1;
}


int tri_fusion_parallel(void){

    thread_t tid[N];
    int i = 0;
    void * pt;

    for(i = 0 ; i < N; i++){
      tid[i] = (thread_t)(size_t)i;
      thread_create(&tid[i], tri_2_a_2, (void *)(size_t)i);
    }

    for(i = 0; i < N; i++)
        thread_join(tid[i], &pt);

    fusion();

    return 1;
}


int main(int argc, char * argv[]){

    int i = 0;

    if(argc != 2){
        printf("Pas d'arguments, nombre maximal de thread par défault:64\n");
    }
    else{
    N = atoi(argv[1]);
    }
    i =N;

    while(i != 1 && i != 2){
        i = i/2;
    }

    //~if(i != 2){
        //~printf("donner un nombre de threads de la forme 2^n");
        //~return EXIT_FAILURE;
    //~}

    tab_pt = (int *)malloc(sizeof(int)*N*2);

    srand ( time(NULL) );

    printf("Tableau à trier:\n");
    for(i = 0; i < (N * 2); i++){
        tab_pt[i] = rand()%20+1;
        printf("%d,", tab_pt[i]);
    }

    //~for(i = 8; i > 0 ; --i){
    //~tab_pt[i-1] = 8 - i;
    //~}
    printf("\n");


    tri_fusion_parallel();

    printf("Tableau trié:\n");

    print_tableau();
    printf("\n");


    return EXIT_SUCCESS;
}
