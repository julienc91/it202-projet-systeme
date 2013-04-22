#ifdef PTHREAD
#include <pthread.h>
#define thread_t pthread_t
#define thread_create(a,b,c) pthread_create(a, NULL, b, c)
#define thread_join pthread_join
#define thread_self pthread_self
#define thread_yield sched_yield
#define thread_exit pthread_exit
#endif
