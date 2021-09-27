#include<stdio.h>
#include<stdbool.h>
#include<stdlib.h>
#include<string.h>

#include "liste.h"




void ajout_tete (Liste *l1, char* info) {
    Cellule *p = malloc(sizeof(Cellule));
    if (p == NULL) exit(EXIT_FAILURE);
    /* Extraction des informations */
/*    char *buff = malloc(strlen(info)+1);
    if (buff == NULL) exit(EXIT_FAILURE);
    memset( &buff, 0, strlen(info)+1);
    strcpy(buff, info);
*/
    p-> port = malloc(5);
    p-> ip = malloc(16);
    p-> nom = malloc(7);
    if (p->port == NULL || p->ip == NULL || p->nom == NULL) exit(EXIT_FAILURE);

    /* SECURITE */
    memset( p->nom, 0, 7);
    memset( p->ip, 0, 16);
    memset( p->port, 0, 5);

    char *buff = strtok(info, " *");
    buff = strtok(NULL, " *");
    strncpy(p->nom, buff, 6);
    buff = strtok(NULL, " *");
    strcpy(p->ip, buff);
    buff = strtok(NULL, " *");
    strcpy(p->port, buff);

    p-> suiv = (*l1);
    (*l1) = p;
}


bool appartient(Liste liste, char* nom)
{
    if(liste!=NULL)
    {
	if (strcmp(nom,liste->nom)==0)
            return 1;
	else
	    return appartient(liste->suiv, nom);
    }
    return 0;
}


void impression_liste(Liste l1){
//    system("clear");
    if (l1 == NULL) {
	printf("AUCUN BOAT DETECTE POUR LE MOMENT...\n");
	return; }

    printf(" --------LISTE DES BOATS LOCALISES---------\n\n");
    printf("| IDENTIFIANT |  PORT   |        IP        |\n");
    printf("|_____________|_________|__________________|\n");
    int k = 1;
    Cellule *p = l1;
    while(p -> suiv != NULL){
        printf("| %s      | %s    | %s |\n", p->nom, p->port, p->ip);
        p = p->suiv;
        k++;}
    printf("| %s      | %s    | %s |\n", p->nom, p->port, p->ip);

    printf("\nLa liste contient %d éléments\n",k);
}


void suppression_tete(Liste *l1) {
    if ((*l1) == NULL) exit(EXIT_FAILURE);

    if ((*l1) -> suiv != NULL) {
	Cellule *tmp= (*l1);
    	(*l1) = (*l1) -> suiv;
        free(tmp->port);
        free(tmp->nom);
        free(tmp->ip);
        free(tmp);
     }
}



void detruire_liste(Liste *l1)
{
    if ((*l1) == NULL) exit(EXIT_FAILURE);
    while((*l1) -> suiv != NULL)
    {
        suppression_tete(l1);
    }
    (*l1) = (*l1) -> suiv;
    free(*l1);
}




void initialisation(Liste *pl)
{
  *pl = NULL;
}

