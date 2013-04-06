#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <ucontext.h> /* ne compile pas avec -std=c89 ou -std=c99 */
#include "queue.h"
#include "thread.h"

static Threads threadList;

void thread_init_function(void)
{
	if(!threadList.isInitialized)
	{
		TAILQ_INIT(&threadList.list);
		threadList.isInitialized = 1;
		// il faut récupérer le contexte courant et le mettre dans threadList.mainThread, ainsi que l'ajouter

		//getcontext, craeatecontext, etc...
	}
}


extern thread_t thread_self(void)
{
	thread_t ret;
	//penser à faire une méthode de recopie
	if(!TAILQ_EMPTY(&threadList.list))
	{

		ret.state = TAILQ_FIRST(&threadList.list)->state;
		ret.context = TAILQ_FIRST(&threadList.list)->context;

		//TAILQ_REMOVE(&thread_list, ret, entries);
	}

	thread_init_function();
	return ret;
	/*if (TAILQ_NEXT(TAILQ_FIRST(&thread_list),entries) == NULL)
	{
		return (thread_t) *TAILQ_LAST(&thread_list, thread_t_);
	}
	return (thread_t) *TAILQ_FIRST(&thread_list);*/
}

extern int thread_create(thread_t *newthread, void *(*func)(void *), void *funcarg)
{
	thread_init_function();
	//Création d'un nouveau thread
	
	//Exécution de func avec funcarg en paramètre
	func(funcarg);
	
	return 0;
}

extern int thread_yield(void)
{
	thread_init_function();
	
	return 0;
}

extern int thread_join(thread_t thread, void **retval)
{
	thread_init_function();
	return 0;
}

extern void thread_exit(void *retval)
{
	thread_init_function();
	//Affectation de la valeur de retour du thread courant à retval
	  // => Il faudrait peut-être un champ (void*) retval dans thread_t pour la récupérer facilement ?

	//Terminaison du thread courant
	TAILQ_FIRST(&threadList.list)->state = DEAD;
	
	//Cette fonction ne doit pas terminer
	label:
		goto label;
}
