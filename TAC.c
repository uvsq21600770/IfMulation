#include "Include/TAC.h"
// Liste des temps d'arrivé des clients
// struct Tac {
// 	double date;
// 	struct Tac* next;
// };
// typedef struct Tac * TAC;
//
// // On va garder en mémoire le début, pour les pops et la fin pour les insert en O(1)
// struct list_Tac {
// 	TAC head;
// 	TAC tail;
// };
// typedef struct list_Tac* list_TAC;

list_TAC add_TAC(list_TAC l_TAC, double d)
{
	TAC elem = malloc(sizeof(struct Tac));
	if(!elem) exit(10);

	elem->date = d;
	elem->next = NULL;

	if(!l_TAC) exit(11);

	if(!l_TAC->head && !l_TAC->tail)
	{
		l_TAC->head = l_TAC->tail = elem;
	} else if(!l_TAC->head || !l_TAC->tail)
	{
		exit(12);
	} else {
		l_TAC->tail->next = elem;
		l_TAC->tail = elem;
	}

	return l_TAC;
}

// Pop la tête de la file pour récup la date
double pop_TAC(list_TAC l_TAC)
{
	double res = l_TAC->head->date;

	TAC h = NULL;
	TAC prev = NULL;

	if(!l_TAC) exit(13); // Liste non créée
	if(!l_TAC->head && !l_TAC->tail) return 0; // Liste vide lors d'un pop
	if(!l_TAC->head || !l_TAC->tail) exit(14); // Si l'un est NULL l'autre devrait l'être aussi

	h = l_TAC->head;
	prev = h->next;

	free(h);
	l_TAC->head = prev; // Décale la tête d'un cran

	if(!l_TAC->head) l_TAC->tail = l_TAC->head; // Cohérence tête / queue

	return res;
}

list_TAC free_list_TAC(list_TAC l_TAC)
{
	while(l_TAC->head)
	{
		pop_TAC(l_TAC);
	}
	return l_TAC;
}

list_TAC new_list_TAC()
{
	list_TAC l_TAC = malloc(sizeof(struct list_Tac));
	if(!l_TAC) exit(15);

	l_TAC->head = l_TAC->tail = NULL;

	return l_TAC;
}

list_TAC* new_tab_list_TAC(int size)
{
  list_TAC* l_TAC = malloc(size * sizeof(list_TAC*));
  if(!l_TAC) exit(16);

  for(int i = 0; i < size; i++)
  {
    l_TAC[i] = new_list_TAC();
  }

  return l_TAC;
}
