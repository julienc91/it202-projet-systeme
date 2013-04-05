#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h> /* ne compile pas avec -std=c89 ou -std=c99 */
#include "queue.h"

typedef enum { READY, SLEEPING, DEAD} STATE;

 typedef struct thread_t_{
	/* state of the thread */
	STATE state;
	/* context of the thread*/
	ucontext_t context;
	/* contains pointers to next and previous node */
	TAILQ_ENTRY(tailq_entry) entries;
	};
	
typedef struct thread_t_ thread_t;


static thread_t main_t;
static TAILQ_HEAD(,thread_t_) thread_list;

extern thread_t thread_self(void)
{
	if (TAILQ_NEXT(TAILQ_FIRST(&thread_list),entries) == NULL)
	{
		return (thread_t) *TAILQ_LAST(&thread_list, thread_t_);
	}
	return (thread_t) *TAILQ_FIRST(&thread_list);
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
	
}
