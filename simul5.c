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

int sum_QUEUE(int* QUEUE, int size)
{
	int res = 0;
	for(int i = 0; i < size; i++)
	{
		if(QUEUE[i] > 1)
			res += QUEUE[i] - 1;
	}
	return res;
}

void resetFreeServ(int* freeServ, int size)
{
	for(int i = 0; i < size; i++)
	{
		freeServ[i] = 0;
	}
}

void printFreeServ(int* freeServ, int size)
{
	for(int i = 0; i < size; i++)
	{
		printf("fS[%d] = %d\n", i, freeServ[i]);
	}
	printf("\n");
}

void printQUEUE(int* QUEUE, int size)
{
	for(int i = 0; i < size; i++)
	{
		printf("QUEUE[%d] = %d\n", i, QUEUE[i]);
	}
	printf("\n");
}

int getMinInQueue(int* QUEUE, int size)
{
	int NB_S_CANDIDAT = 1; // NBR de file de taille minimale
	int* freeServ = init_QUEUE(size); // 1 si plus petite file, 0 sinon
	int minVal = QUEUE[0]; //Valeur du minimum
	int minIndex = 0; // Index du minimum
  freeServ[0] = 1;

	for(int i = 1; i < size; i++)
	{
		if(QUEUE[i] < minVal) // On trouve une file plus petite que la minimale actuelle
		{
			minVal = QUEUE[i];
			minIndex = i;
			NB_S_CANDIDAT = 1;

			resetFreeServ(freeServ, size);

			freeServ[i] = 1;
		}
		else if(QUEUE[i] == minVal) // On trouve une autre file de même taille que la minimale
		{
			freeServ[i] = 1;
			NB_S_CANDIDAT++;
		}
	}

	if(NB_S_CANDIDAT == 1) // On a une seule file plus petite que les autres
	{
		free(freeServ);

		return minIndex;
	} else {
		// Si on a plusieurs files de taille minimale ==>

		int amountServersToSkip = getRandomInSet(NB_S_CANDIDAT); // Le nombre de serveurs qu'on va ignorer
		 																												 //on prend le amountServersToSkip + 1 ème serveur

		// On parcourt tous les Serveurs en incrémentant S_FREE_VISITED pour chaque Serveur libre vu
		// On place le client sur le k ième Serveur libre vu (histoire de conserver du random sans avoir à faire des centaines de tirages unif)
		int S_FREE_VISITED = 0;
		int realIndex = -1;
		while(realIndex < size && S_FREE_VISITED <= amountServersToSkip)
		{
			realIndex++;
			if(freeServ[realIndex] == 1)
			{
				S_FREE_VISITED++;
			}
		}


		free(freeServ);
		return realIndex;
	}
	free(freeServ);
	return -1;
}

double simul_MMn (double lambda, double mu, int *converge, int n) {

	if(n <= 0)
	{
		printf("Non, on ne peut pas avoir une MM0 et espérer avoir un système stable\n");
		exit(0);
	}

	list_TAC* l_TAC = new_tab_list_TAC(n);
	int* AC = init(n, 0);
	int* DS = init(n, n);
	int* FS = init(n, 2 * n);
	int* QUEUE = init_QUEUE(n);



	// Toutes les queues sont vides, on peut donc juste faire un tirage uniforme
	int randomFirstAC = getRandomInSet(n);
	ECHEANCIER E = nouveau_evenement (AC[randomFirstAC],0.0);


	unsigned long int N = 0; // Nombre de clients dans la file
	double T = 0.0; // La date courante
	unsigned long int nb_event = 0;


	double totalWaitingTime = 0.0;
	int totalAmountClients = 1;
	double averageWaitingTime = 0.0;


	unsigned long int nb_e = 0;
	double max = 0.0;
	double min = 0.0;

	double L_MANCHON = 2e4;
	*converge=0;
	printf("### SIMUL %.3lf %.3lf\n",lambda,mu);
	while ((nb_event < NB_EVENT_MAX) && (!(*converge))) {
		struct evenement *e = recup_premier_evenement (&E);

		// ------ Nouveau Client ------ //
		if (e->le_type >= AC[0] && e->le_type <= AC[n-1]) {
			T = e->la_date;

			N++;
			(QUEUE[e->le_type])++;

			int shortestQueueAC = getMinInQueue(QUEUE, n);

			double delay = T+expo(lambda);
			E = inserer_evenement(nouveau_evenement(AC[shortestQueueAC],delay),E);
			add_TAC(l_TAC[AC[e->le_type]], T); // on pourrait directement mettre e->le_type pour l'instant
			totalAmountClients++;

			if (QUEUE[e->le_type] == 1) // Le PC de cette queue est libre
			{
				E = inserer_evenement(nouveau_evenement(DS[e->le_type],T),E);
			}
		}

		// ------ Début Service ------ //
		if (e->le_type >= DS[0] && e->le_type <= DS[n-1]) {
			T = e->la_date;
			int offsetDStoFS = e->le_type+n; // C'est pas vraiment plus parlant que de mettre direct e->... dans le call de fonction
			int offsetDStoAC = e->le_type-n;

			double arrivingTime = pop_TAC(l_TAC[offsetDStoAC]);
			totalWaitingTime += T - arrivingTime;

			E = inserer_evenement(nouveau_evenement(offsetDStoFS,T+expo(mu)),E);
		}

		// ------ Fin Service ------ //
		if (e->le_type >= FS[0] && e->le_type <= FS[n-1]) {
			T = e->la_date;

			int offsetFStoAC = e->le_type - (2*n);

			(QUEUE[offsetFStoAC])--;
			N--;

			if (QUEUE[offsetFStoAC] > 0) // Il reste des clients dans la file
			{
				E = inserer_evenement(nouveau_evenement(DS[offsetFStoAC],T),E);
			}

		}


		nb_event++;
		nb_e++;
		averageWaitingTime = totalWaitingTime / (double)(totalAmountClients - sum_QUEUE(QUEUE, n));

		if (averageWaitingTime>max) max = averageWaitingTime;
		if (averageWaitingTime<min) min = averageWaitingTime;
		if (nb_e>L_MANCHON) { // on a atteint la fin du manchon
			if (max-min <= averageWaitingTime*EPSILON) *converge = 1;
			else {	nb_e = 0; max = min = averageWaitingTime;
					L_MANCHON = 2e4*averageWaitingTime;
				}
		}
		free(e);
#ifdef DEBUG1
if (nb_event % 1000000==0) {
	printf("%.3le %lu %.3le || %lf < %lf < %lf\r",T,N,(double)nb_event,min,averageWaitingTime,max);
	fflush(stdout);
	}
#endif
	}
#ifdef DEBUG1
printf("%.3le %lu %.3le || %lf < %lf < %lf\r",T,N,(double)nb_event,min,averageWaitingTime,max);
printf("\n");
#endif

printf("TotalW: %lf, totalC: %d, average: %lf - sum: %d\n", totalWaitingTime, totalAmountClients, averageWaitingTime, sum_QUEUE(QUEUE, n));

	free(DS);
	free(FS);
	free(QUEUE);
	for(int i = 0; i < n; i++)
	{
		l_TAC[i] = free_list_TAC(l_TAC[i]);
		free(l_TAC[i]);
	}
	free(l_TAC);

	return averageWaitingTime;
}

int main () {
	FILE *F;
	F = fopen("mm1_5.data","w");
	double lambda;
	double mu = 1.0;
	double nbmoy;
	int converge = 0;


	for (lambda = 1.0; lambda < 9.0; lambda += 0.5) {
	nbmoy = simul_MMn (lambda,mu,&converge, 10);
	if (converge) fprintf(F,"%lf %lf\n",lambda/mu,nbmoy);
		else printf("Pas de convergence\n");

		printf("\n");
	}

	for (lambda = 9.0; lambda < 9.9; lambda += 0.1) {
	nbmoy = simul_MMn (lambda,mu,&converge, 10);
	if (converge) fprintf(F,"%lf %lf\n",lambda/mu,nbmoy);
		else printf("Pas de convergence\n");

		printf("\n");
	}

	for (lambda = 9.9; lambda < 10.0; lambda += 0.025) {
	nbmoy = simul_MMn (lambda,mu,&converge, 10);
	if (converge) fprintf(F,"%lf %lf\n",lambda/mu,nbmoy);
		else printf("Pas de convergence\n");

		printf("\n");
	}
	
	// Si vous voulez vérifier sur 10.0+
	/*for (lambda = 10.0; lambda < 10.05; lambda += 0.025) {
	nbmoy = simul_MMn (lambda,mu,&converge, 10);
	if (converge) fprintf(F,"%lf %lf\n",lambda/mu,nbmoy);
		else printf("Pas de convergence\n");

		printf("\n");
	}*/

	fclose(F);
}
