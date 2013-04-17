#ifndef _TSP_JOB_H
#define _TSP_JOB_H
/* gestion de queues de jobs */

/* Structure pour la tête de queue */
typedef struct {
   struct Maillon *first ;
   struct Maillon *last ;
   int end;
} TSPqueue;

/* initialise la queue [q] */
extern void init_queue (TSPqueue *q) ;

/* ajoute un job à la queue [q]: chemin [p], [hops] villes parcourues, longueur [len] parcourue */
extern void add_job (TSPqueue *q, Path_t p, int hops, int len) ;

/* enlève un job de la queue [q], le stocke dans [p], [hops] et [len]. Peut retourner 0 si la queue est temporairement vide ; cela ne veut pas forcément dire qu'il n'y aura pas d'autres jobs ajoutés !! */
extern int get_job (TSPqueue *q, Path_t p, int *hops, int *len) ;

/* enregistre qu'il n'y aura plus de jobs ajoutés à la queue */
extern void no_more_jobs (TSPqueue *q) ;

/* retourne 1 si la queue est finie (i.e. no_more_jobs() a été appelé, et tous
 * les jobs ont été enlevés), 0 sinon */
extern int empty_queue (TSPqueue *q) ;
#endif
