
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>
#include <string.h>
#include <sys/time.h>
#include <pthread.h>

#include "thread.h"
#include "61-Voy_types.h"
#include "61-Voy_job.h"


/* macro de mesure de temps, retourne une valeur en µsecondes */
#define TIME_DIFF(t1, t2) \
        ((t2.tv_sec - t1.tv_sec) * 1000000 + (t2.tv_usec - t1.tv_usec))



/* variables globales */

/* arguments du programme */
int    Argc ;
char   **Argv ;

/* dernier minimum trouvé */
int    minimum ;

/* liste des tâches */
TSPqueue     q ;

/* tableau des distances */
DTab_t    distance ;

/* nombre de villes */
int NrTowns ;

DTab_t dist;

pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;


#define MAXX	100
#define MAXY	100
typedef struct
		{
		 int x, y ;
		} coor_t ;

typedef coor_t coortab_t [MAXE] ;
coortab_t towns ;

/* initialisation du tableau des distances */
void genmap ()
{
int i, j ;
int dx, dy;

 NrTowns = atoi (Argv[1]) ;
 if (NrTowns > MAXE) {
  fprintf(stderr,"trop de villes, augmentez MAXE dans tsp-types.h");
  exit(1);
 }

 srand (atoi(Argv[2])) ;

 for (i=0; i<NrTowns; i++)
  {
   towns [i].x = rand () % MAXX ;
   towns [i].y = rand () % MAXY ;
  }

 for (i=0; i<NrTowns; i++)
  {
   for (j=0; j<NrTowns; j++)
    {
     /* Un peu réaliste */
     dx = towns [i].x - towns [j].x ;
     dy = towns [i].y - towns [j].y ;
     distance [i][j] = (int) sqrt ((double) ((dx * dx) + (dy * dy))) ;
    }
  }
}


/* impression tableau des distances, pour vérifier au besoin */
void PrintDistTab ()
{
 int i, j ;

 printf ("NrTowns = %d\n", NrTowns) ;

 for (i=0; i<NrTowns; i++)
  {
   printf ("distance [%1d]",i) ;
   for (j=0; j<NrTowns; j++)
    {
     printf (" [%2d:%2d] ", j, distance[i][j]) ;
    }
   printf (";\n\n") ;
  }
 printf ("done ...\n") ;

}

void printPath (Path_t path)
{
 char toprint[MAXY][MAXX+1];
 int i;
 int x, y;

 memset(toprint, ' ', sizeof(toprint));
 for (i = 0; i < NrTowns-1; i++)
 {
  int x1 = towns[path[i]].x;
  int y1 = towns[path[i]].y;
  int x2 = towns[path[i+1]].x;
  int y2 = towns[path[i+1]].y;

  if (abs(x2-x1) > abs(y2-y1))
   for (x = 1; x < abs(x2-x1); x++) {
    toprint[y1+x*(y2-y1)/abs(x2-x1)][x1+x*(x2-x1)/abs(x2-x1)] = '*';
   }
  else
   for (y = 1; y < abs(y2-y1); y++) {
    toprint[y1+y*(y2-y1)/abs(y2-y1)][x1+y*(x2-x1)/abs(y2-y1)] = '*';
   }
 }

 for (i = 0; i < NrTowns; i++)
  toprint[towns[i].y][towns[i].x] = '#';
 for (y = 0; y < MAXY; y++) {
  toprint[y][MAXX] = 0;
  printf("%s\n", toprint[y]);
 }
}

/* résolution du problème du voyageur de commerce */

int present (int city, int hops, Path_t path)
{
 unsigned int i ;

 for (i=0; i<hops; i++)
   if (path [i] == city) return 1 ;
 return 0 ;
}

void tsp (int hops, int len, Path_t path, int *cuts)
{
	int i ;
	int me, dist ;

	if (len >= minimum)
	{
		cuts[hops]++ ;
		return;
	}

	if (hops == NrTowns)
	{
		if (len +  distance[0][path[NrTowns-1]]< minimum){
			pthread_mutex_lock(&mut);
			if (len +  distance[0][path[NrTowns-1]]< minimum)
			{
				minimum = len +  distance[0][path[NrTowns-1]];
				printf ("found path len = %3d :", minimum) ;
				for (i=0; i < NrTowns; i++)
					 printf ("%2d ", path[i]) ;
				printf ("\n") ;
				printPath(path);
			}
			pthread_mutex_unlock(&mut);
		}
	}
	else
	{
	me = path [hops-1] ;

	for (i=0; i < NrTowns; i++)
	 {
	 if (!present (i, hops, path))
	   {
	   path [hops] = i ;
	   dist = distance[me][i] ;
	   tsp (hops+1, len+dist, path, cuts) ;
	   }
	 }

	}
}

void * tsp_init(void * p){
	int ncuts[MAXE] = {0};
	int me = *(int *) p;
	Path_t path;
	path[0] = 0;
	path[1] = me;
	tsp(2, distance[0][me], path, ncuts);
	return NULL;
}


void tsp_premier_niveau(){

	int i ;
	//~ int me, dist ;
	thread_t tid[MAXE];
	int id[MAXE];


	//~ if (len >= minimum)
	//~ {
		//~ cuts[hops]++ ;
	    //~ return;
	//~ }
	//~
	//~ if (hops == NrTowns){
		//~ if (len +  distance[0][path[NrTowns-1]]< minimum)
	    //~ {
			//~ minimum = len +  distance[0][path[NrTowns-1]];
			//~ printf ("found path len = %3d :", minimum) ;
			//~ for (i=0; i < NrTowns; i++)
				//~ printf ("%2d ", path[i]) ;
			//~ printf ("\n") ;
			//~ printPath(path);
	    //~ }
	  //~ }
	//~ else
	//~ {
	//~ me = path [hops-1] ;

	for (i=0; i < NrTowns; i++){
		id[i] = i+1;
		thread_create(&tid[i], tsp_init, (void *) &id[i]);

		//~ if (!present (i, hops, path))
		//~ {
		//~ path [hops] = i ;
		//~ dist = distance[me][i] ;
		//~ tsp (hops+1, len+dist, path, cuts) ;
		//~ }
		//~ }
	}
	void * pt;
	for (i=0; i < NrTowns; i++){
		thread_join(tid[i], &pt);
	}
}



int main (int argc, char **argv)
{
   unsigned long temps;
   int i;
   Path_t path;
   struct timeval t1, t2;

   int *cuts; /* Juste à des fins de statistiques pour savoir combien de fois on a pu optimiser */

   if (argc != 3)
     {
	fprintf (stderr, "Usage: %s  <ncities> <seed> \n",argv[0]) ;
	exit (1) ;
     }

   Argc = argc ;
   Argv = argv ;
   minimum = INT_MAX ;

   printf ("ncities = %3d\n", atoi(argv[1])) ;

   init_queue(&q);
   genmap () ;

   cuts = calloc(NrTowns, sizeof(long));

   gettimeofday(&t1,NULL);


   for(i=0;i<MAXE;i++)
     path[i] = -1 ;
   path [0] = 0;


   //tsp(1,0,path,cuts);
   tsp_premier_niveau();


   gettimeofday(&t2,NULL);

   temps = TIME_DIFF(t1,t2);
   printf("time = %ld.%03ldms (coupures :", temps/1000, temps%1000);
   for (i=0; i<NrTowns; i++)
     printf(" %d",cuts[i]);
     printf(")\n");
   return 0 ;
}
