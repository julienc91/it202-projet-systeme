#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <ucontext.h> /* ne compile pas avec -std=c89 ou -std=c99 */
#include "queue.h"
#include "thread.h"

static Threads threadList;

thread_t thread_copy(thread_t *th){

	thread_t copie;

	copie.state = th->state;
	copie.context = th->context;
	copie.already_done = th->already_done;
	copie.retval_size = th->retval_size ;


	copie.retval = malloc(th->retval_size);
	memcpy(copie.retval, th->retval, th->retval_size);

	copie.entries = th->entries ;
	return copie;
}

void thread_init_function(void)
{
	if(!threadList.isInitialized)
	{
		threadList.isInitialized = TRUE;
		TAILQ_INIT(&threadList.list);

		// il faut récupérer le contexte courant et le mettre dans threadList.mainThread, ainsi que l'ajouter
		thread_t *thread = malloc(sizeof(thread_t));
		thread->state = READY;
		thread->already_done = FALSE;
		thread->retval = NULL;
		thread->retval_size = 0;

		getcontext(&(thread->context));

		threadList.mainThread = thread;
		TAILQ_INSERT_HEAD(&(threadList.list), thread, entries);

	}
}


extern thread_t thread_self(void)
{
	thread_init_function();

	return thread_copy(TAILQ_FIRST(&threadList.list));
}

extern int thread_create(thread_t *newthread, void *(*func)(void *), void *funcarg)
{
	thread_init_function();

	//Gestion des contextes
	getcontext(&(newthread->context));
	(newthread->context).uc_stack.ss_size = STACK_SIZE;
	(newthread->context).uc_stack.ss_sp = malloc(STACK_SIZE);
	(newthread->context).uc_link = NULL;
	makecontext(&(newthread->context), (void (*)(void))func, 1, funcarg);

	//Initialisation des attributs
	newthread->already_done = FALSE;
	newthread->state = READY;

	//Ajout en tête de la pile des threads
	TAILQ_INSERT_HEAD(&(threadList.list), newthread, entries);

	//Déplacement dans le contexte
	ucontext_t previous;
	swapcontext(&previous, &(newthread->context));

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
