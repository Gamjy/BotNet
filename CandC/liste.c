#include<stdio.h>
#include<stdbool.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "liste.h"




void ajout_tete (Liste *l1, char* info) {
    char nom[6];
    char *buff = strtok(info, " *");
    buff = strtok(NULL, " *");
    strncpy(nom, buff, 6);

    if(appartient(*l1, nom)) return;

    Cellule *p = malloc(sizeof(Cellule));
    if (p == NULL) exit(EXIT_FAILURE);

    p-> port = malloc(6);
    p-> ip = malloc(18);
    p-> nom = malloc(8);
    if (p->port == NULL || p->ip == NULL || p->nom == NULL) exit(EXIT_FAILURE);

    /* SECURITE */
    memset( p->nom, 0, 8);
    memset( p->ip, 0, 18);
    memset( p->port, 0, 6);

    strncpy(p->nom, buff, 6);
    buff = strtok(NULL, " *");
    strcpy(p->ip, buff);
    buff = strtok(NULL, " *");
    strcpy(p->port, buff);

    p->connect = 0;
    p-> suiv = (*l1);
    (*l1) = p;
}


bool appartient(Liste liste, char* nom)
{
    Cellule *tmp= (liste);
    while(tmp!=NULL)
    {
	if (strncmp(nom,tmp->nom,6)==0) return 1;
	tmp = tmp->suiv;
    }
    return 0;
}



void pointeur_boat(Liste *pl1, Liste liste, char* nom)
{
    if(liste!=NULL)
    {
        if (strcmp(nom,liste->nom)==0) {
	    printf("adresse %p\n", liste);
	    *pl1 = liste;
            return; }
        else
            pointeur_boat(pl1, liste->suiv, nom);
    }
    *pl1 = NULL;
    return;
}




char* ip_boat(Liste liste, char* nom)
{
    if(liste!=NULL)
    {
        if (strcmp(nom,liste->nom)==0)
            return liste->ip;
        else
            return ip_boat(liste->suiv, nom);
    }
    return "";
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
    if ((*l1) == NULL) return;
    Cellule *tmp= (*l1);
    (*l1) = (*l1) -> suiv;
    if (tmp->connect != 0) {
        send( tmp->socket, "QUIT", 5, 0 );
	close(tmp->socket);
    }
    free(tmp->port);
    free(tmp->nom);
    free(tmp->ip);
    free(tmp);
}


void detruire_liste(Liste *l1)
{
    printf("DETRUCTION\n");
    if ((*l1) == NULL) return;
    while((*l1) -> suiv != NULL)
    {
        suppression_tete(l1);
    }
    suppression_tete(l1);
}




void suppression_ifCONNECT(Liste *l1) {
    if ((*l1) == NULL) return;
    Cellule *tmp= (*l1);

    if (tmp->connect != 0) {
	suppression_ifCONNECT(&(tmp->suiv));
	return;
    }
    (*l1) = (*l1) -> suiv;
    free(tmp->port);
    free(tmp->nom);
    free(tmp->ip);
    free(tmp);
    suppression_ifCONNECT(l1);
}


void detruire_ifCONNECT(Liste *l1)
{
    if ((*l1) == NULL) return;
    while((*l1) -> suiv != NULL)
    {
        suppression_ifCONNECT(l1);
    }
    suppression_ifCONNECT(l1);
}





void initialisation(Liste *pl)
{
  *pl = NULL;
}

