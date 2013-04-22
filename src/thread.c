#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <ucontext.h> /* ne compile pas avec -std=c89 ou -std=c99 */
#include <valgrind/valgrind.h>

#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include "queue.h"
#include "thread.h"

#define DEBUG(string) printf("Function : %s , line %d of file %s (\n", string, __LINE__, __FILE__);

static Threads threadList = {FALSE};
static ucontext_t return_t;
static pthread_key_t id_thread;
static pthread_mutex_t lock;
static unsigned int nb_cores;

//fonction qui renvoie le nombre de coeur d'un ordinateur
unsigned int get_cores(void)
{
	FILE *cmdline = fopen("/proc/cpuinfo", "rb");
	if(cmdline==NULL)
	{
		perror("fopen\n");
	}
	char *arg = 0;
	size_t size = 0;

	unsigned int num_proc = 0;
	while(getline(&arg, &size, cmdline) != -1)
	{
		arg[9] = 0;
		if(strcmp(arg, "processor") == 0)
			++num_proc;
	}
	free(arg);
	fclose(cmdline);
	return num_proc;
}

//fonction qui renvoie l'identifiant spécifique du thread courant
long get_id_thread(void)
{
	long tmp;
	if (pthread_getspecific(id_thread)==NULL)
	{
		tmp =-1;
	}
	else
	{
		tmp = (long) ((long) pthread_getspecific(id_thread) - (long) 1);
	}
	return tmp;
}

//fonction appelée lors de la creation d'un thread noyau
void *thread_pthread_handler(void * v)
{
	pthread_setspecific(id_thread, v);
	DEBUG ("handler")
	printf("id_thread (get_spe): %ld\n", (long) pthread_getspecific(id_thread));
	printf("id_thread (get_id): %ld\n", get_id_thread());

	thread_yield();
	return v;
}

//fonction appelée à la fin d'un thread, via uc_link
void thread_return()
{
	if(get_id_thread() != -1)
	{
	(threadList.currentThreads[get_id_thread()])->state = DEAD;

	pthread_mutex_lock(&lock);
	TAILQ_INSERT_TAIL(&(threadList.list_dead), threadList.currentThreads[get_id_thread()], entries);
	pthread_mutex_unlock(&lock);
	}

	thread_yield();
}
//fonction appelée dans le contexte lors de la création d'un thread
void stock_return(void * funcarg, void* (*func)())
{
	if(get_id_thread() != -1)
	{
		printf("Arguments de stock_return (id, string): %ld, %s\n", get_id_thread(), (char*)funcarg);
		threadList.currentThreads[get_id_thread()]->retval = func(funcarg);
	}
	else
	{
		DEBUG("stock_return")
	}
}

//fonction appelée à la terminaison du programme pour libérer la mémoire
void threads_destroy(void)
{
	DEBUG ("threads_destroy")

	unsigned int i;
	for(i = 0; i < nb_cores-1; i++)
	{
		pthread_cancel(*(threadList.pthreads[i]));
	}
	for(i = 0; i < nb_cores-1; i++)
	{
		free(threadList.pthreads[i]);
	}
	DEBUG ("Apres free des threads")

	thread_t item, tmp_item;
	free(return_t.uc_stack.ss_sp);

	DEBUG ("After free(return_t.uc_stack.ss_sp)")

	free(threadList.pthreads);
	free(threadList.currentThreads);

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
		VALGRIND_STACK_DEREGISTER(item->valgrind_stackid);
		free(item->context.uc_stack.ss_sp);
		free(item);
	}

	for (item = TAILQ_FIRST(&(threadList.list_dead)); item != NULL; item = tmp_item)
	{
		tmp_item = TAILQ_NEXT(item, entries);
		/* Retire l'élément de la liste*/
		TAILQ_REMOVE(&(threadList.list_dead), item, entries);

		/* Libère l'espace alloué */
		VALGRIND_STACK_DEREGISTER(item->valgrind_stackid);
		free(item->context.uc_stack.ss_sp);
		free(item);
	}

	DEBUG ("(fin de )threads_destroy")
}

//fonction d'initialisation, à n'appeler qu'une seule fois au premier appel
void thread_init_function(void)
{
	if(!threadList.isInitialized)
	{
		// Code pour le multicoeur
		nb_cores = get_cores();
		pthread_mutex_init(&lock, NULL);
		unsigned long i;
		threadList.pthreads = calloc(nb_cores-1, sizeof(pthread_t*));
		threadList.currentThreads = calloc(nb_cores, sizeof(thread_t));

		for(i = 0; i < nb_cores; i++)
		{
			threadList.currentThreads[i]=NULL;
		}

		pthread_key_create(&id_thread, NULL);
		pthread_setspecific(id_thread, (void*)((long) 1));
		printf("id_main (get_spe): %ld\n", (long) pthread_getspecific(id_thread));
		printf("id_main (get_id): %ld\n", get_id_thread());


		// Code de base
		TAILQ_INIT(&threadList.list);
		TAILQ_INIT(&threadList.list_sleeping);
		TAILQ_INIT(&threadList.list_dead);

		atexit(threads_destroy);

		// il faut récupérer le contexte courant et le mettre dans threadList.mainThread, ainsi que l'ajouter
		thread_t thread = calloc(1, sizeof(struct thread_t_));
		if(thread == NULL)
		{
			perror("malloc");
			return;
		}

		// initialisation du thread
		thread->state = READY;
		thread->already_done = FALSE;
		thread->retval = NULL;
		getcontext(&(thread->context));

		if(!threadList.isInitialized)
		{
			thread->valgrind_stackid = VALGRIND_STACK_REGISTER(thread->context.uc_stack.ss_sp, thread->context.uc_stack.ss_sp + thread->context.uc_stack.ss_size);
			threadList.isInitialized = TRUE;

			threadList.mainThread = thread;
			threadList.currentThreads[0] = thread;

			getcontext(&return_t);
			return_t.uc_stack.ss_size = STACK_SIZE;
			return_t.uc_stack.ss_sp = malloc(STACK_SIZE);

			memset(return_t.uc_stack.ss_sp , 0, STACK_SIZE);
			if(return_t.uc_stack.ss_sp == NULL)
			{
				perror("malloc");
				return;
			}

			return_t.uc_link = NULL;
			makecontext(&return_t, (void (*)(void))thread_return, 0);

			for(i = 0; i < nb_cores-1; i++)
			{
				DEBUG("pthread_init_function")
				threadList.pthreads[i] = malloc(sizeof(pthread_t));
				pthread_create(threadList.pthreads[i], NULL, thread_pthread_handler, (void*)i+2);
				printf("id thread: %ld\n", i+2);
				usleep(100);
			}
		}
	}
}

extern thread_t thread_self(void)
{
	thread_init_function();
	if(get_id_thread() != -1)
	{
		return threadList.currentThreads[get_id_thread()];
	}
	return NULL;
}

extern int thread_create(thread_t *newthread, void *(*func)(void *), void *funcarg)
{
	thread_init_function();

	//Allocation
	*newthread = malloc(sizeof(struct thread_t_));
	if(*newthread == NULL)
	{
		perror("malloc");
		return -1;
	}

	//Gestion des contextes
	getcontext(&((*newthread)->context));
	((*newthread)->context).uc_stack.ss_size = STACK_SIZE;
	((*newthread)->context).uc_stack.ss_sp = malloc(STACK_SIZE);
	if(((*newthread)->context).uc_stack.ss_sp == NULL)
	{
		free(*newthread);
		perror("malloc");
		return -1;
	}

	(*newthread)->valgrind_stackid = VALGRIND_STACK_REGISTER(((*newthread)->context).uc_stack.ss_sp,((*newthread)->context).uc_stack.ss_sp + ((*newthread)->context).uc_stack.ss_size);

	((*newthread)->context).uc_link = &return_t;
	makecontext(&((*newthread)->context), (void (*)(void))*stock_return, 2, funcarg, (void (*)(void))func);

	//Initialisation des attributs
	(*newthread)->already_done = FALSE;
	(*newthread)->state = READY;
	(*newthread)->retval = NULL;

	//Ajout en tête de la pile des threads
	pthread_mutex_lock(&lock);
	TAILQ_INSERT_TAIL(&(threadList.list), (*newthread), entries);

	if(get_id_thread() != -1 && threadList.currentThreads[get_id_thread()]!=NULL)
	{
		getcontext(&(threadList.currentThreads[get_id_thread()]->context));
	}
	pthread_mutex_unlock(&lock);

	return 0;
}
/*
 * Fonctionnement : yield() appelé depuis le main envoie vers le sommet de la file,
 * sinon renvoie vers le main
 */
extern int thread_yield(void)
{
	yield_begin:
		thread_init_function();
		thread_t tmp = NULL;
		thread_t thread = NULL;

		if (get_id_thread() != -1)
		{
			tmp = threadList.currentThreads[get_id_thread()];
		}
		else
		{
			DEBUG("thread_yield")
		}
		 //Recherche du premier thread prêt
	 	pthread_mutex_lock(&lock);

		if(!TAILQ_EMPTY(&threadList.list)) //si il y a des éléments dans la liste des threads prêts
		{
				thread = TAILQ_FIRST(&(threadList.list));
				TAILQ_REMOVE(&(threadList.list), thread, entries);
				pthread_mutex_unlock(&lock);

		}
		else if (!TAILQ_EMPTY(&threadList.list_sleeping))// si il n'y a plus que des threads endormis
		{
				thread = TAILQ_FIRST(&(threadList.list_sleeping));
				thread->state = READY;
				TAILQ_REMOVE(&(threadList.list_sleeping), thread, entries);
				pthread_mutex_unlock(&lock);
		}
		else //si tous les threads sont morts
		{
				unsigned int i;
				int yield_again = FALSE;
				for (i = 0; i< nb_cores; i++)
				{
					if(threadList.currentThreads[i]!=NULL && threadList.currentThreads[i]->state!=DEAD)
					{
						yield_again = TRUE;
						break;
					}
				}
				pthread_mutex_unlock(&lock);

				if(tmp == threadList.mainThread)
				{
					DEBUG("fin du main")
					pthread_mutex_lock(&lock);
					tmp->state = DEAD;
					TAILQ_INSERT_TAIL(&(threadList.list_dead), threadList.currentThreads[get_id_thread()], entries);
					pthread_mutex_unlock(&lock);
				}

				if(yield_again)
				{
					usleep(100);
					goto yield_begin;
				}

				return 0;
		}

		//Màj du currentThread dans la threadList
		if (get_id_thread() != -1)
		{
		threadList.currentThreads[get_id_thread()] = thread;
		}

		//Changement de contexte
		if(tmp == NULL)
		{
			setcontext(&(threadList.currentThreads[get_id_thread()]->context));
		}
		else
		{
			swapcontext(&(tmp->context), &(threadList.currentThreads[get_id_thread()]->context));
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

	if (get_id_thread() == -1)
	{
	DEBUG ("thread_join")
	return 0;
	}

	while(thread->state != DEAD)
	{
		switch(thread->state)
		{
			case(READY):
				//Echange de currentThread
				tmp = threadList.currentThreads[get_id_thread()];
				threadList.currentThreads[get_id_thread()] = thread;

				//mise en sommeil de l'ancien thread courant
				if (tmp != NULL)
				{
					pthread_mutex_lock(&lock);
					tmp->state = SLEEPING;
					TAILQ_INSERT_TAIL(&(threadList.list_sleeping), tmp, entries);
					pthread_mutex_unlock(&lock);
				}

				//Changement de contexte
				if (tmp != NULL)
				{
					swapcontext(&(tmp->context), &(threadList.currentThreads[get_id_thread()]->context));
				}
				else
				{
					setcontext(&(threadList.currentThreads[get_id_thread()]->context));
				}
				break;

			case(SLEEPING):
			if (tmp != NULL)
				{
					pthread_mutex_lock(&lock);
					tmp->state = SLEEPING;
					TAILQ_INSERT_TAIL(&(threadList.list_sleeping), tmp, entries);
					pthread_mutex_unlock(&lock);
				}
				thread_yield();
				break;

			default:
			break;
		}
	}

	if(retval!=NULL)
	{
		*retval = thread->retval;
	}

	return 0;
}

extern void thread_exit(void *retval)
{
	DEBUG("entering thread_exit");
	thread_init_function();
	if(get_id_thread() != -1)
	{
		//Affectation de la valeur de retour du thread courant à retval
		threadList.currentThreads[get_id_thread()]->retval = retval;

		//Terminaison du thread courant
		pthread_mutex_lock(&lock);
		(threadList.currentThreads[get_id_thread()])->state = DEAD;
		TAILQ_INSERT_TAIL(&(threadList.list_dead), threadList.currentThreads[get_id_thread()], entries);
		pthread_mutex_unlock(&lock);
	}
	else
	{
		DEBUG("thread_exit")
	}
	thread_yield();

	printf("DEBUG : Retval: %s  id_thread: %ld  state: %d \n", (char*) retval, get_id_thread(), (threadList.currentThreads[get_id_thread()])->state);
	DEBUG("thread_exit, goto label")

	//Cette fonction ne doit pas terminer
	label:
		goto label;
}
