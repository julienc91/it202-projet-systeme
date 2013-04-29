#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <ucontext.h> /* ne compile pas avec -std=c89 ou -std=c99 */
#include <valgrind/valgrind.h>
#include <signal.h>

#include <unistd.h>

#include <pthread.h>
#include <string.h>
#include "queue.h"
#include "thread.h"

#define SIG_YIELD SIGUSR1


static Threads threadList ;
static ucontext_t return_t;
static pthread_t preemption_thread;
static pthread_t waiting_thread;
static int run_preemption; //A revoir...

void fun() {
  fprintf(stderr, "TOTO\n");
  thread_yield();
  fprintf(stderr, "TITI\n");
}

void* wait_signal(void* n) {
  sigset_t sig;
  sigemptyset(&sig);
  sigaddset(&sig, SIG_YIELD);
  int i;
  signal(SIG_YIELD, fun);
  fprintf(stderr, "Waiting for signal\n");
  sigwait(&sig, &i);
  fprintf(stderr, "Signal received\n");
  return (void*) n;
}

void* preemption_signal(void* n) {

  /* struct sigaction *act = malloc(sizeof(struct sigaction)); */
  /* act->sa_handler = fun; */
  /* act->sa_flags = 0; */
  /* sigaction(SIG_YIELD, act, NULL); */

  fprintf(stderr, "\tStart preemption\n");
  while(run_preemption) {
    pthread_create(&waiting_thread, NULL, (void*) fun, (void*) 0);
    fprintf(stderr, "TATA\n");
    sleep(1);
    pthread_cancel(waiting_thread);
    fprintf(stderr, "\tNew yield with preemption: %p\n", threadList.currentThread);
  }
  fprintf(stderr, "\tEnd of preemption\n");
  return (void*) n;
}


void set_preemption_active(int n) {
  static int is_preemption_done = 0;
  run_preemption = n;

  //A la création du premier thread, on active la fonction de préemtion dans un thread noyau
  if(n) {
    if(!is_preemption_done) {
      run_preemption = 1;
      pthread_create(&preemption_thread, NULL, (void*) preemption_signal, (void*) 0);
      is_preemption_done = 1;
      fprintf(stderr, "Preemption activated\n");
    }
    else
      fprintf(stderr, "Preemption already activated\n");
  }
  else {
    if(is_preemption_done) {
      run_preemption = 0;
      pthread_join(preemption_thread, NULL);
      is_preemption_done = 0;
      fprintf(stderr, "Preemption deactivated\n");
    }
    else
      fprintf(stderr, "Preemption already deactivated\n");
  }
}

//A appeler lorsque le thread ayant la priorité maximale sort de la liste
void update_max_priority() {
  thread_t item, tmp_item;
  threadList.max_priority = 1;
  for (item = TAILQ_FIRST(&(threadList.list)); item != NULL; item = tmp_item)
    {
      tmp_item = TAILQ_NEXT(item, entries);
      if(threadList.max_priority < item->default_priority)
	threadList.max_priority = item->default_priority;
    }
}

void debug_priority() {
  thread_t item, tmp_item;
  for (item = TAILQ_FIRST(&(threadList.list)); item != NULL; item = tmp_item)
    {
      tmp_item = TAILQ_NEXT(item, entries);
      printf("(%d - %d) ", item->default_priority, item->current_priority);
    }
  printf("\n");
}

//priority > 0
int set_thread_priority(thread_t thread, int priority) {
  if(priority <= 0)
    return 0;
  thread->default_priority = priority;
  if(threadList.max_priority < priority)
    threadList.max_priority = priority;
  return 1;
}

int get_cores(void)
{
	FILE *cmdline = fopen("/proc/cpuinfo", "rb");
	char *arg = 0;
	size_t size = 0;

	int num_proc = 0;
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

//fonction appelée à la fin d'un thread, via uc_link
void thread_return(){}

//fonction appelée dans le contexte lors de la création d'un thread
void stock_return(void * funcarg, void* (*func)())
{
	threadList.currentThread->retval = func(funcarg);
	(threadList.currentThread)->state = DEAD;
	if(threadList.currentThread->default_priority == threadList.max_priority)
	  update_max_priority();
	TAILQ_REMOVE(&(threadList.list), threadList.currentThread, entries);
	TAILQ_INSERT_TAIL(&(threadList.list_dead), threadList.currentThread, entries);

	thread_yield();
}

//fonction appelée à la terminaison du programme pour libérer la mémoire
void threads_destroy()
{
        if(run_preemption) {
	  set_preemption_active(0);
	}

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
		if(thread == NULL)
		{
			perror("malloc");
			return;
		}

		#ifdef DEBUG_MODE
		thread->id = 0;
		thread->nb_calls = 0;
		#endif

		thread->state = READY;
		thread->retval = NULL;
		thread->default_priority = DEFAULT_PRIORITY;
		thread->current_priority = DEFAULT_PRIORITY;

		getcontext(&(thread->context));

		thread->valgrind_stackid = VALGRIND_STACK_REGISTER((thread->context).uc_stack.ss_sp,
								   (thread->context).uc_stack.ss_sp +
								   (thread->context).uc_stack.ss_size);


		threadList.max_priority = 1;
		threadList.mainThread = thread;
		threadList.currentThread = thread;
		TAILQ_INSERT_HEAD(&(threadList.list), thread, entries);


		getcontext(&return_t);
		return_t.uc_stack.ss_size = STACK_SIZE;
		return_t.uc_stack.ss_sp = malloc(STACK_SIZE);

		if(return_t.uc_stack.ss_sp == NULL)
		{
			perror("malloc");
			return;
		}

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

	(*newthread)->valgrind_stackid = VALGRIND_STACK_REGISTER(((*newthread)->context).uc_stack.ss_sp,
								 ((*newthread)->context).uc_stack.ss_sp + 
								 ((*newthread)->context).uc_stack.ss_size);

	((*newthread)->context).uc_link = &return_t;
	makecontext(&((*newthread)->context), (void (*)(void))*stock_return, 2, funcarg, (void (*)(void))func);

	//Initialisation des attributs
	#ifdef DEBUG_MODE
	static int id = 1;
	(*newthread)->id = id++;
	(*newthread)->nb_calls = 0;
	#endif

	(*newthread)->state = READY;
	(*newthread)->retval = NULL;
	(*newthread)->default_priority = DEFAULT_PRIORITY;
	(*newthread)->current_priority = DEFAULT_PRIORITY;

	//Ajout en tête de la pile des threads
	TAILQ_INSERT_TAIL(&(threadList.list), (*newthread), entries);

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
	tmp->current_priority--;

		//Recherche du premier thread prêt
		if(!TAILQ_EMPTY(&threadList.list)) //si il y a des éléments dans la liste des threads prêts
		{
		  do {
		    thread = TAILQ_FIRST(&(threadList.list));
		    if ((thread == tmp) && (TAILQ_NEXT(thread, entries) == NULL)) {
		      return 0;
		    }

		    if(thread->current_priority <= thread->default_priority-threadList.max_priority) {
		      thread->current_priority = thread->default_priority;
		    }
		    else if(thread->current_priority <= 0) {
		      thread->current_priority--;
		    }

		    /*
		    //si le premier thread est le thread courant, on prend le suivant
		    if ((thread == tmp) && (TAILQ_NEXT(thread, entries) != NULL))
		    {
		    thread = TAILQ_NEXT(thread, entries);
		    }*/
		    //si le thread courant est le seul thread prêt, on continue l'exécution
		    
		    TAILQ_REMOVE(&(threadList.list), thread, entries);
		    TAILQ_INSERT_TAIL(&(threadList.list), thread, entries);
		  } while(thread->current_priority < 0);

		}
		else if (!TAILQ_EMPTY(&threadList.list_sleeping))// si il n'y a plus que des threads endormis
		{
			thread = TAILQ_FIRST(&(threadList.list_sleeping));
			thread->state = READY;
			if(thread->default_priority > threadList.max_priority)
			  threadList.max_priority = thread->default_priority;
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
		
		#ifdef DEBUG_MODE
		thread->nb_calls++;
		fprintf(stderr, "Using thread %d (time %d) with priority: %d/%d\n", thread->id, thread->nb_calls, thread->current_priority, thread->default_priority);
		#endif
		
		//Changement de contexte
		swapcontext(&(tmp->context), &(threadList.currentThread->context));
	return 0;
}

extern int thread_join(thread_t thread, void **retval)
{
	thread_init_function();
	thread_t tmp;

	if (thread == NULL)
	{
		if (retval != NULL)
		{
			*retval = NULL;
		}
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
				if(tmp->default_priority == threadList.max_priority)
				  update_max_priority();
				TAILQ_REMOVE(&(threadList.list), tmp, entries);
				TAILQ_INSERT_TAIL(&(threadList.list_sleeping), tmp, entries);

				//Changement de contexte
				swapcontext(&(tmp->context), &(threadList.currentThread->context));
				break;

			case(SLEEPING):
				tmp->state = SLEEPING;
				if(tmp->default_priority == threadList.max_priority)
				  update_max_priority();
				TAILQ_REMOVE(&(threadList.list), tmp, entries);
				TAILQ_INSERT_TAIL(&(threadList.list_sleeping), tmp, entries);
				thread_yield();
				break;

			default:
			break;
		}
	}

	if (retval == NULL)
	{	
		return 0;
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
	if(threadList.currentThread->default_priority == threadList.max_priority)
	  update_max_priority();

	TAILQ_REMOVE(&(threadList.list), threadList.currentThread, entries);
	TAILQ_INSERT_TAIL(&(threadList.list_dead), threadList.currentThread, entries);

	thread_yield();

	//Cette fonction ne doit pas terminer
	label:
		goto label;
}
