#ifndef __THREAD_H__
#define __THREAD_H__



typedef enum { READY, SLEEPING, DEAD} STATE;

typedef struct thread_t_
{
  /* state of the thread */
  STATE state;
  /* context of the thread*/
  ucontext_t context;
  /* contains pointers to next and previous node */
  TAILQ_ENTRY(thread_t_) entries;
  /* return value */
  void *retval;
  /* boolean */
  int already_done;
} thread_t;

typedef struct Threads
{
  int isInitialized;
  thread_t mainThread;
  TAILQ_HEAD(, thread_t_) list;
} Threads;



//*********************************//
//***********INTERFACE*************//
//*********************************//


/* identifiant de thread
 * NB: pourra être un entier au lieu d'un pointeur si ca vous arrange,
 *     mais attention aux inconvénient des tableaux de threads
 *     (consommation mémoire, cout d'allocation, ...).
 */
//typedef void * thread_t;

/* recuperer l'identifiant du thread courant.
 */
extern thread_t thread_self(void);

/* creer un nouveau thread qui va exécuter la fonction func avec l'argument funcarg.
 * renvoie 0 en cas de succès, -1 en cas d'erreur.
 */
extern int thread_create(thread_t *newthread, void *(*func)(void *), void *funcarg);

/* passer la main à un autre thread.
 */
extern int thread_yield(void);

/* attendre la fin d'exécution d'un thread.
 * la valeur renvoyée par le thread est placée dans *retval.
 * si retval est NULL, la valeur de retour est ignorée.
 */
extern int thread_join(thread_t thread, void **retval);

/* terminer le thread courant en renvoyant la valeur de retour retval.
 * cette fonction ne retourne jamais.
 *
 * L'attribut noreturn aide le compilateur à optimiser le code de
 * l'application (élimination de code mort). Attention à ne pas mettre
 * cet attribut dans votre interface tant que votre thread_exit()
 * n'est pas correctement implémenté (il ne doit jamais retourner).
 */
extern void thread_exit(void *retval) __attribute__ ((__noreturn__));

#endif /* __THREAD_H__ */
