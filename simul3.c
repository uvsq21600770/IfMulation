#include "math.h"
#include "Include/TAC.h"

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

int* initDS(int n)
{
	int *DS = malloc(n * sizeof(int));
	if(DS == NULL)
	{
		exit(1);
	}

	for(int i = 0; i < n; i++)
	{
		DS[i] = 1 + i;
	}

	return DS;
}

int* initFS(int n)
{
	int *FS = malloc(n * sizeof(int));
	if(FS == NULL)
	{
		exit(2);
	}

	for(int i = 0; i < n; i++)
	{
		FS[i] = 1 + i + n;
	}

	return FS;
}

int* initS_BUSY(int n)
{
	int *S_BUSY = malloc(n * sizeof(int));
	if(S_BUSY == NULL)
	{
		exit(1);
	}

	for(int i = 0; i < n; i++)
	{
		S_BUSY[i] = 0;
	}

	return S_BUSY;
}

// ## 3. La simulation
#define AC 0
//#define DS 2
//#define FS 3
// ### EXO 3
#define NB_EVENT_MAX 1e8
// ### EXO 4
//#define NB_EVENT_MAX 1e9
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

double simul_MMn (double lambda, double mu, int *converge, int n) {

	if(n == 0)
	{
		printf("Non, on ne peut pas avoir une MM0 et espérer avoir un système stable\n");
		exit(0);
	}

	list_TAC l_TAC = new_list_TAC();

	ECHEANCIER E = nouveau_evenement (AC,0.0);
	unsigned long int N = 0; // Nombre de clients dans la file
	double T = 0.0; // La date courante
	unsigned long int nb_event = 0;
	double S = 0.0;

	double totalWaitingTime = 0.0;
	int totalAmountClients = 0;
	double averageWaitingTime = 0.0;

	int* DS = initDS(n);
	int* FS = initFS(n);
	int* S_BUSY = initS_BUSY(n);
	int nb_S_FREE = n;


// for(int i = 0; i < n; i++)
// {
// 	printf("DS <%d> - FS <%d> - S_BUSY <%d>\n", DS[i], FS[i], S_BUSY[i]);
// }
// exit(0);
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
		if (e->le_type==AC) {
			T = e->la_date;
			S = S + N*(T-lastT);
			lastT = T;
			N++;
			double delay = T+expo(lambda);
// printf("[AC] T = %lf\n", T);
			E = inserer_evenement(nouveau_evenement(AC,delay),E);
			add_TAC(l_TAC, T);
			totalAmountClients++;
// if(N > n)
// printf("delay incoming at some point\n");
			if (N <= n) // On a au moins un PC de libre
			{
				// On pull une uniforme et on détermine le Serveur libre associé
				double unifPull = unif();
				// double unifPull = 0.9; choisit au hasard par la team pour tester en MM1
				int k = 0;

// if(STOP == 0)
// {
// 	printf("<%lf> - <%lf>\n", (double)k/(double)nb_S_FREE , (double)(k+1)/(double)nb_S_FREE);
// }

				// sort du while quand on est sur le bon Serveur, k est son indice dans la liste des Serveurs libres
				while( !(unifPull >= (double)k/(double)nb_S_FREE && unifPull <= (double)(k+1)/(double)nb_S_FREE ) )
				{
					k = k + 1;
				}

				// On parcourt tous les Serveurs en incrémentant S_FREE_VISITED pour chaque Serveur libre vu
				// On place le client sur le k ième Serveur libre vu (histoire de conserver du random sans avoir à faire des centaines de tirages unif)
				int S_FREE_VISITED = 0;
				int realIndex = 0; // Véritable indice du Serveur libre qu'on va attribuer
				while(S_FREE_VISITED != k && realIndex < n-1)
				{
					if(S_BUSY[realIndex] == 0)
					{
						S_FREE_VISITED++;
					}
					realIndex++;
				}

				// C'était laborieux
				E = inserer_evenement(nouveau_evenement(DS[realIndex],T),E);
				S_BUSY[realIndex] = 1;
				nb_S_FREE--;
//printf("AC - DS[%d] - free: %d\n", realIndex, nb_S_FREE);
			}
		}

		// ------ Début Service ------ //
		if (e->le_type >= DS[0] && e->le_type <= DS[n-1]) {
			T = e->la_date;
			int offsetDStoFS = e->le_type+n; // C'est pas vraiment plus parlant que de mettre direct e->... dans le call de fonction
//printf("DS[%d] (%d) into FS[%d] (%d) - off(%d)\n",e->le_type - 1 ,DS[e->le_type - 1], e->le_type - 1, FS[e->le_type - 1], offsetDStoFS);

			double arrivingTime = pop_TAC(l_TAC);
			totalWaitingTime += T - arrivingTime;
// printf("[DS %d] %lf -- %lf = %lf - %lf\n",e->le_type ,totalWaitingTime, T - arrivingTime, T, arrivingTime);

			E = inserer_evenement(nouveau_evenement(offsetDStoFS,T+expo(mu)),E);
		}

		// ------ Fin Service ------ //
		if (e->le_type >= FS[0] && e->le_type <= FS[n-1]) {
			T = e->la_date;
			S = S + N*(T-lastT);
			lastT = T;
			N--;
//printf("FS[%d] (%d) ", e->le_type - n - 1, FS[e->le_type - n - 1]);
			if (N <= n-1) // Tous les clients de la file sont sur un Serveur
			{
				S_BUSY[e->le_type - n - 1] = 0; // on offset pour retrouver le bon Serveur
				nb_S_FREE++;
//printf("no longer busy ");
			}
			else{
				E = inserer_evenement(nouveau_evenement(DS[e->le_type - n - 1],T),E);
//printf("FS - DS[%d] - free: %d\n", e->le_type - n, nb_S_FREE);
			}
//printf("\n");
		}
STOP++;
//if(STOP == 60) exit(100);

//printf("T= %lf\n", T);
		nb_event++;
		nb_e++;
		nbmoy = S/lastT;
		averageWaitingTime = totalWaitingTime / (double)totalAmountClients;
//printf("AWT: %lf\n", averageWaitingTime);
		if (averageWaitingTime>max) max = averageWaitingTime;
		if (averageWaitingTime<min) min = averageWaitingTime;
		if (nb_e>L_MANCHON) { // on a atteint la fin du manchon
			if (max-min <= averageWaitingTime*EPSILON) *converge = 1;
			else {	nb_e = 0; max = min = averageWaitingTime;
					L_MANCHON = 1e3*averageWaitingTime;
				}
		}
// if (nbmoy>max) max = nbmoy;
// if (nbmoy<min) min = nbmoy;
// if (nb_e>L_MANCHON) { // on a atteint la fin du manchon
// 	if (max-min < nbmoy*EPSILON) *converge = 1;
// 	else {	nb_e = 0; max = min = nbmoy;
// 			L_MANCHON = 1e3*nbmoy;
// 		}
// }
		free(e);
#ifdef DEBUG1
if (nb_event % 1000000==0) {
	//printf("%.3le %lu %.3le || %lf < %lf < %lf\r",T,N,(double)nb_event,min,S/lastT,max);
	printf("%.3le %lu %.3le || %lf < %lf < %lf\r",T,N,(double)nb_event,min,averageWaitingTime,max);
	fflush(stdout);
	}
#endif
	}
#ifdef DEBUG1
printf("%.3le %lu %.3le || %lf < %lf < %lf\r",T,N,(double)nb_event,min,averageWaitingTime,max);
printf("\n");
#endif

printf("TotalW: %lf, totalC: %d, average: %lf\n", totalWaitingTime, totalAmountClients, averageWaitingTime);

		// free tes putains de DS[] etc bordel
		free(DS);
		free(FS);
		free(S_BUSY);
		l_TAC = free_list_TAC(l_TAC);
		free(l_TAC);

		return averageWaitingTime;
}

int main () {
	FILE *F;
	F = fopen("mm1_3.data","w");
	srand(10);
	double lambda;
	double mu = 1.0;
	double nbmoy;
	int converge = 0;

// ### EXO 3
	for (lambda = 0.025 ; lambda < 10.1; lambda += 0.05) {
		nbmoy = simul_MMn (lambda,mu,&converge, 10);
		if (converge) fprintf(F,"%lf %lf\n",lambda/mu,nbmoy);
			else printf("Pas de convergence\n");
	}

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
