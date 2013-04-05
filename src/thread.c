#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <ucontext.h> /* ne compile pas avec -std=c89 ou -std=c99 */
#include "queue.h"

typedef enum { READY, SLEEPING, DEAD} STATE;

struct thread_t_
{
	/* state of the thread */
	STATE state;
	/* context of the thread*/
	ucontext_t context;
	/* contains pointers to next and previous node */
	TAILQ_ENTRY(thread_t_) entries;
};

typedef struct thread_t_ thread_t;

TAILQ_HEAD(Threads, thread_t_);


static thread_t main_t;

extern thread_t thread_self(void)
{
	TAILQ_HEAD(Threads, thread_t_) thread_list = TAILQ_HEAD_INITIALIZER(thread_list);
	TAILQ_INIT(&thread_list);

	//penser à faire une méthode de recopie
	thread_t *ret = malloc(sizeof(thread_t));
	ret->state = TAILQ_FIRST(&thread_list)->state;
	ret->context = TAILQ_FIRST(&thread_list)->context;

	//TAILQ_REMOVE(&thread_list, ret, entries);

	return *ret;
	/*if (TAILQ_NEXT(TAILQ_FIRST(&thread_list),entries) == NULL)
	{
		return (thread_t) *TAILQ_LAST(&thread_list, thread_t_);
	}
	return (thread_t) *TAILQ_FIRST(&thread_list);*/
}

extern int thread_create(thread_t *newthread, void *(*func)(void *), void *funcarg)
{

	return 0;
}

extern int thread_yield(void)
{

	return 0;
}

extern int thread_join(thread_t thread, void **retval)
{

	return 0;
}

extern void thread_exit(void *retval)
{
	label:
		goto label;
}
