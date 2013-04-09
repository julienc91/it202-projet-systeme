#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <ucontext.h> /* ne compile pas avec -std=c89 ou -std=c99 */
#include "queue.h"
#include "thread.h"

static Threads threadList;

void thread_init_function(void)
{
	if(!threadList.isInitialized)
	{
		threadList.isInitialized = TRUE;
		TAILQ_INIT(&threadList.list);

		// il faut récupérer le contexte courant et le mettre dans threadList.mainThread, ainsi que l'ajouter
		thread_t thread = malloc(sizeof(struct thread_t_));
		thread->state = READY;
		thread->already_done = FALSE;
		thread->retval = NULL;

		getcontext(&(thread->context));

		threadList.mainThread = thread;
		threadList.currentThread = thread;
		TAILQ_INSERT_HEAD(&(threadList.list), thread, entries);
	}
}


extern thread_t thread_self(void)
{
	thread_init_function();

	return threadList.currentThread;
}

extern int thread_create(thread_t *newthread, void *(*func)(void *), void *funcarg)
{
	thread_init_function();

	//Allocation
	*newthread = malloc(sizeof(struct thread_t_));
	if(*newthread == NULL) {
	  perror("malloc");
	  return -1;
	}

	//Gestion des contextes
	getcontext(&((*newthread)->context));
	((*newthread)->context).uc_stack.ss_size = STACK_SIZE;
	((*newthread)->context).uc_stack.ss_sp = malloc(STACK_SIZE);
	((*newthread)->context).uc_link = NULL;
	makecontext(&((*newthread)->context), (void (*)(void))func, 1, funcarg);

	//Initialisation des attributs
	(*newthread)->already_done = FALSE;
	(*newthread)->state = READY;
	(*newthread)->retval = NULL;

	//Ajout en tête de la pile des threads
	TAILQ_INSERT_HEAD(&(threadList.list), (*newthread), entries);

	return 0;
}

extern int thread_yield(void)
{
	thread_init_function();

	//Màj du currentThread dans la threadList
	thread_t tmp = threadList.currentThread;
	thread_t thread;

	//Recherche du premier thread prêt
	TAILQ_FOREACH(thread, &(threadList.list), entries) {
	  if(thread->state == READY && thread != tmp)
	    break;
	}
	if(thread->state != READY)
	  return 0;
	threadList.currentThread = thread;
	
	//Changement de contexte
	swapcontext(&(tmp->context), &(threadList.currentThread->context));

	return 0;
}

extern int thread_join(thread_t thread, void **retval)
{
	thread_init_function();

	//Echange de currentThread
	thread_t tmp = threadList.currentThread;
	threadList.currentThread = thread;

	tmp->state = SLEEPING;
	
	//Changement de contexte
	swapcontext(&(tmp->context), &(threadList.currentThread->context));

	*retval = thread->retval;

	return 0;
}

extern void thread_exit(void *retval)
{

	thread_init_function();
	//Affectation de la valeur de retour du thread courant à retval
	threadList.currentThread->retval = retval;

	//Terminaison du thread courant
	(threadList.currentThread)->state = DEAD;

	thread_yield();

	//Cette fonction ne doit pas terminer
	label:
		goto label;
}
