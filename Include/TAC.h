#include "stdio.h"
#include "stdlib.h"

// Liste des temps d'arrivé des clients
struct Tac {
	double date;
	struct Tac* next;
};
typedef struct Tac * TAC;

// On va garder en mémoire le début, pour les pops et la fin pour les insert en O(1)
struct list_Tac {
	TAC head;
	TAC tail;
};
typedef struct list_Tac* list_TAC;

list_TAC add_TAC(list_TAC l_TAC, double d);

// Pop la tête de la file pour récup la date
double pop_TAC(list_TAC l_TAC);

list_TAC free_list_TAC(list_TAC l_TAC);

list_TAC new_list_TAC();

list_TAC* new_tab_list_TAC(int size);
