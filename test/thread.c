#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <ucontext.h> /* ne compile pas avec -std=c89 ou -std=c99 */
#include <valgrind/valgrind.h>
#include "queue.h"
#include "thread.h"


static Threads threadList ;
static ucontext_t return_t ;

//fonction appelée à la fin d'un thread, via uc_link
void thread_return()
{
	(threadList.currentThread)->state = DEAD;
	TAILQ_REMOVE(&(threadList.list), threadList.currentThread, entries);
	TAILQ_INSERT_TAIL(&(threadList.list_dead), threadList.currentThread, entries);

	thread_yield();
}
//fonction appelée dans le contexte lors de la création d'un thread
void stock_return(void * funcarg, void* (*func)())
{
	threadList.currentThread->retval = func(funcarg);
}                                      

//fonction appelée à la terminaison du programme pour libérer la mémoire
void threads_destroy()
{
	thread_t item, tmp_item;
	free(return_t.uc_stack.ss_sp);

	for (item = TAILQ_FIRST(&(threadList.list)); item != NULL; item = tmp_item)
        {
                tmp_item = TAILQ_NEXT(item, entries);
				/* Retire l'élément de la liste*/
				TAILQ_REMOVE(&(threadList.list), item, entries);

				/* Libère l'espace alloué */
				VALGRIND_STACK_DEREGISTER(item->valgrind_stackid);
				free(item->context.uc_stack.ss_sp);
				free(item);
        }
        
   	for (item = TAILQ_FIRST(&(threadList.list_sleeping)); item != NULL; item = tmp_item)
        {
                tmp_item = TAILQ_NEXT(item, entries);
				/* Retire l'élément de la liste*/
				TAILQ_REMOVE(&(threadList.list_sleeping), item, entries);

				/* Libère l'espace alloué */
				free(item->context.uc_stack.ss_sp);
				free(item);
        }
        
    for (item = TAILQ_FIRST(&(threadList.list_dead)); item != NULL; item = tmp_item)
        {
                tmp_item = TAILQ_NEXT(item, entries);
				/* Retire l'élément de la liste*/
				TAILQ_REMOVE(&(threadList.list_dead), item, entries);

				/* Libère l'espace alloué */
				free(item->context.uc_stack.ss_sp);
				free(item);
        }
}

void thread_init_function(void)
{
	if(!threadList.isInitialized)
	{
		threadList.isInitialized = TRUE;
		TAILQ_INIT(&threadList.list);
		TAILQ_INIT(&threadList.list_sleeping);
		TAILQ_INIT(&threadList.list_dead);
		
		atexit(threads_destroy);

		// il faut récupérer le contexte courant et le mettre dans threadList.mainThread, ainsi que l'ajouter
		thread_t thread = malloc(sizeof(struct thread_t_));
		thread->state = READY;
		thread->already_done = FALSE;
		thread->retval = NULL;

		getcontext(&(thread->context));

		threadList.mainThread = thread;
		threadList.currentThread = thread;
		TAILQ_INSERT_HEAD(&(threadList.list), thread, entries);
		
		
		getcontext(&return_t);
		return_t.uc_stack.ss_size = STACK_SIZE;
		return_t.uc_stack.ss_sp = malloc(STACK_SIZE);
		return_t.uc_link = NULL;
		makecontext(&return_t, (void (*)(void))thread_return, 0);
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
	(*newthread)->valgrind_stackid = VALGRIND_STACK_REGISTER(((*newthread)->context).uc_stack.ss_sp,((*newthread)->context).uc_stack.ss_sp + ((*newthread)->context).uc_stack.ss_size);

	((*newthread)->context).uc_link = &return_t;
	//makecontext(&((*newthread)->context), (void (*)(void))func, 1, funcarg);
	makecontext(&((*newthread)->context), (void (*)(void))*stock_return, 2, funcarg, (void (*)(void))func);


	//Initialisation des attributs
	(*newthread)->already_done = FALSE;
	(*newthread)->state = READY;
	(*newthread)->retval = NULL;

	//Ajout en tête de la pile des threads
	TAILQ_INSERT_HEAD(&(threadList.list), (*newthread), entries);

	getcontext(&(threadList.currentThread->context));
	
	return 0;
}
/*
 * Fonctionnement : yield() appelé depuis le main envoie vers le sommet de la file, 
 * sinon renvoie vers le main
 */
extern int thread_yield(void)
{
	thread_init_function();

	thread_t tmp = threadList.currentThread;
	thread_t thread;
	
	if ((tmp == threadList.mainThread) || (threadList.mainThread->state != READY))
	{
		//Recherche du premier thread prêt
		if(!TAILQ_EMPTY(&threadList.list)) //si il y a des éléments dans la liste des threads prêts
		{
			thread = TAILQ_FIRST(&(threadList.list));
			//si le premier thread est le thread courant, on prend le suivant
			if ((thread == tmp) && (TAILQ_NEXT(thread, entries) != NULL)) 
			{
				thread = TAILQ_NEXT(thread, entries);
			}
			//si le thread courant est le seul thread prêt, on continue l'exécution
			else if ((thread == tmp) && (TAILQ_NEXT(thread, entries) == NULL))
			{
				return 0;
			}
		}
		else if (!TAILQ_EMPTY(&threadList.list_sleeping))// si il n'y a plus que des threads endormis
		{
			thread = TAILQ_FIRST(&(threadList.list_sleeping));
			thread->state = READY;
			TAILQ_REMOVE(&(threadList.list_sleeping), thread, entries);
			TAILQ_INSERT_HEAD(&(threadList.list), thread, entries);
		}
		else //si tous les threads sont morts ou endormis
		{
			fprintf(stderr, "Fin : Plus de threads prets ou endormis\n");
			return 0;
		}

		//Màj du currentThread dans la threadList
		threadList.currentThread = thread;
		
		//Changement de contexte
		swapcontext(&(tmp->context), &(threadList.currentThread->context));
	}
	else
	{
		threadList.currentThread = threadList.mainThread;
		swapcontext(&(tmp->context), &(threadList.currentThread->context));
	}
	return 0;
}

extern int thread_join(thread_t thread, void **retval)
{
	thread_init_function();
	thread_t tmp;

	if (thread == NULL)
	{
		*retval = NULL;
		return 0;
	}

	while(thread->state != DEAD)
	{
		switch(thread->state)
		{
			case(READY):
			//Echange de currentThread
			tmp = threadList.currentThread;
			threadList.currentThread = thread;

			//mise en sommeil de l'ancien thread courant
			tmp->state = SLEEPING;
			TAILQ_REMOVE(&(threadList.list), tmp, entries);
			TAILQ_INSERT_TAIL(&(threadList.list_sleeping), tmp, entries);
		
			//Changement de contexte
			swapcontext(&(tmp->context), &(threadList.currentThread->context));
			break;
			
			case(SLEEPING):
			tmp->state = SLEEPING;
			TAILQ_REMOVE(&(threadList.list), tmp, entries);
			TAILQ_INSERT_TAIL(&(threadList.list_sleeping), tmp, entries);
			thread_yield();
			break;
			
			default:
			break;
		}
	}

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
	TAILQ_REMOVE(&(threadList.list), threadList.currentThread, entries);
	TAILQ_INSERT_TAIL(&(threadList.list_dead), threadList.currentThread, entries);

	thread_yield();

	//Cette fonction ne doit pas terminer
	label:
		goto label;
}
