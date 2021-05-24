#include "stdio.h"
#include "stdlib.h"
#include "math.h"

int STOP = 0;

// ## 1. Les générateurs aléatoires
double unif() {
	return (double)random()/((double)RAND_MAX);
}


double expo (double L) {
	return -log(unif())/L;
}

// ## 2. Gestion de l'échéancier
struct evenement {
	int le_type;
	double la_date;
	struct evenement * suiv;
};
typedef struct evenement * ECHEANCIER;

int* init(int size, int offset)
{
	int* tab = malloc(size * sizeof(int));
	if(tab == NULL)
	{
		exit(1);
	}

	for(int i = 0; i < size; i++)
	{
		tab[i] = i + offset;
	}

	return tab;
}

int* init_QUEUE(int n)
{
	int *QUEUE = malloc(n * sizeof(int));
	if(QUEUE == NULL)
	{
		exit(1);
	}

	for(int i = 0; i < n; i++)
	{
		QUEUE[i] = 0;
	}

	return QUEUE;
}

// ## 3. La simulation
//#define AC 0
//#define DS 2
//#define FS 3
// ### EXO 3
//#define NB_EVENT_MAX 1e8
// ### EXO 4
#define NB_EVENT_MAX 1e9
#define DEBUG1
#define EPSILON 1e-4
struct evenement * nouveau_evenement (int le_type, double la_date) {
	struct evenement *e;
	e = malloc(sizeof(struct evenement));
	if (!e) exit(1);
	e->le_type = le_type;
	e->la_date = la_date;
	e->suiv = NULL;
	return e;
}

struct evenement * recup_premier_evenement (ECHEANCIER *E) {
	struct evenement *e = *E;
	*E = (*E)->suiv;
	return e;
}

ECHEANCIER inserer_evenement(struct evenement *e, ECHEANCIER E) {
	if ((E==NULL) || (e->la_date < E->la_date)) {
		e->suiv = E;
		return e;
	}
	E->suiv = inserer_evenement(e,E->suiv);
	return E;
}

// Retourne un entier entre 0 et size-1 qui correspond à choisir au hasard un élément dans un set de "size" éléments
int getRandomInSet(int size)
{
	double unifPull = unif();
	// double unifPull = 0.9; choisit au hasard par la team pour tester en MM1
	int k = 0;

	// sort du while quand on est sur le bon Serveur, k est son indice dans la liste des Serveurs libres
	while( !(unifPull >= (double)k/(double)size && unifPull <= (double)(k+1)/(double)size ) )
	{
		k = k + 1;
	}

	return k;
}

double simul_MMn (double lambda, double mu, int *converge, int n) {

	if(n <= 0)
	{
		printf("Non, on ne peut pas avoir une MM0 et espérer avoir un système stable\n");
		exit(0);
	}

	int* AC = init(n, 0);
	int* DS = init(n, n);
	int* FS = init(n, 2 * n);
	int* QUEUE = init_QUEUE(n);
//int nb_S_FREE = n;

	int randomFirstAC = getRandomInSet(n);
	ECHEANCIER E = nouveau_evenement (AC[randomFirstAC],0.0);
//(QUEUE[randomFirstAC])++;
	printf("F_AC: %d - QUEUE: %d\n", randomFirstAC, QUEUE[randomFirstAC]);
	unsigned long int N = 0; // Nombre de clients dans la file
	double T = 0.0; // La date courante
	unsigned long int nb_event = 0;
	double S = 0.0;

/*  for(int i = 0; i < n; i++)
 {
 	printf("AC <%d> - DS <%d> - FS <%d> - S_BUSY <%d>\n", AC[i], DS[i], FS[i], S_BUSY[i]);
}
 exit(0); */
	double lastT = 1e-6;
	unsigned long int nb_e = 0;
	double max = 0.0;
	double min = 0.0;
	double nbmoy = 0;
	double L_MANCHON = 1e3;
	*converge=0;
	printf("### SIMUL %.3lf %.3lf\n",lambda,mu);
	while ((nb_event < NB_EVENT_MAX) && (!(*converge))) {
		struct evenement *e = recup_premier_evenement (&E);

		// ------ Nouveau Client ------ //
		if (e->le_type >= AC[0] && e->le_type <= AC[n-1]) {
			T = e->la_date;
			S = S + N*(T-lastT);
			lastT = T;
			N++;
			(QUEUE[e->le_type])++;
			int randomAC = getRandomInSet(n);
//printf("AC[%d] - randomAC: %d, QUEUE: %d\n", e->le_type, randomAC, QUEUE[e->le_type]);
			E = inserer_evenement(nouveau_evenement(AC[randomAC],T+expo(lambda)),E);

			if (QUEUE[e->le_type] == 1) // Le PC de cette queue est libre
			{
				E = inserer_evenement(nouveau_evenement(DS[e->le_type],T),E);
			}
		}

		// ------ Début Service ------ //
		if (e->le_type >= DS[0] && e->le_type <= DS[n-1]) {
			T = e->la_date;
			int offsetDStoFS = e->le_type+n; // C'est pas vraiment plus parlant que de mettre direct e->... dans le call de fonction
//printf("DS[%d] - offset: %d\n", e->le_type, offsetDStoFS);
			E = inserer_evenement(nouveau_evenement(offsetDStoFS,T+expo(mu)),E);
		}

		// ------ Fin Service ------ //
		if (e->le_type >= FS[0] && e->le_type <= FS[n-1]) {
			T = e->la_date;
			S = S + N*(T-lastT);
			int offsetFStoAC = e->le_type - (2*n);
			lastT = T;
//printf("IN FS: Q_Bef = %d\n");
			(QUEUE[offsetFStoAC])--;
			N--;
//printf("IN FS: Q_Aft = %d\n");
//printf("FS[%d] - offset: %d, QUEUE: %d\n", e->le_type, offsetFStoAC, QUEUE[offsetFStoAC]);
			if (QUEUE[offsetFStoAC] > 0) // Il reste des clients dans la file
			{
				E = inserer_evenement(nouveau_evenement(DS[offsetFStoAC],T),E);
			}

		}
		STOP++;
//if(STOP == 1000) exit(10);

		nb_event++;
		nb_e++;
		nbmoy = S/lastT;
		if (nbmoy>max) max = nbmoy;
		if (nbmoy<min) min = nbmoy;
		if (nb_e>L_MANCHON) { // on a atteint la fin du manchon
			if (max-min < nbmoy*EPSILON) *converge = 1;
			else {	nb_e = 0; max = min = nbmoy;
					L_MANCHON = 1e3*nbmoy;
				}
		}
		free(e);
#ifdef DEBUG1
if (nb_event % 1000000==0) {
	printf("%.3le %lu %.3le || %lf < %lf < %lf\r",T,N,(double)nb_event,min,S/lastT,max);
	fflush(stdout);
	}
#endif
	}
#ifdef DEBUG1
printf("%.3le %lu %.3le || %lf < %lf < %lf\r",T,N,(double)nb_event,min,S/lastT,max);
printf("\n");
#endif

	// free tes putains de DS[] etc bordel
	free(DS);
	free(FS);
	free(QUEUE);


	return S/lastT;
}

int main () {
	FILE *F;
	F = fopen("mm1_3.data","w");
	double lambda;
	double mu = 1.0;
	double nbmoy;
	int converge = 0;

// ### EXO 3
	for (lambda = 0.025 ; lambda < 1.2; lambda += 0.05) {
		nbmoy = simul_MMn (lambda,mu,&converge, 10);
		if (converge) fprintf(F,"%lf %lf\n",lambda/mu,nbmoy);
			else printf("Pas de convergence\n");
	}

	//Pas loin de pas converger probablement
	nbmoy = simul_MMn (9.8,mu,&converge, 10);
	if (converge) fprintf(F,"%lf %lf\n",11/(10*mu),nbmoy);
		else printf("Pas de convergence\n");

	// Pas sensé converger
	nbmoy = simul_MMn (11,mu,&converge, 10);
	if (converge) fprintf(F,"%lf %lf\n",11/(10*mu),nbmoy);
		else printf("Pas de convergence\n");
// ### EXO 4
/*
	int i;
 	lambda = 0.1;
	for (i=0 ; i<10 ; i++) {
		printf("### lambda = %lf\n",lambda);
		nbmoy = simul_MM1 (lambda,mu,&converge);
		if (converge) fprintf(F,"%lf %lf\n",lambda/mu,nbmoy);
			else printf("Pas de convergence\n");
		lambda = lambda + (1.0-lambda)/2.0;
	}
*/
	fclose(F);
}
