#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <stdbool.h>

#include "libthread.h"
#include "libipc.h"
#include "liste.h"
#include "menu.h"

int msgid1;
int msgid2;
int nb_rep;
/* Messages IPC */
Msg_requete message1, reponse;

/* Gestion des signaux */
struct sigaction action;
void hand (int sig) {
	if (sig == SIGINT) {
		printf("\nFin du programme\n");
		exit(-1);}
}



void send(int start, int argc, char *argv[])
{
    /* Envoi de la consigne */
    for (int i=start; i<argc; i++) {
	memset(message1.texte, 0, TAILLE_MSG);
        strcpy(message1.texte, argv[i]);
	printf("envoi : %s\n", message1.texte);
        msgsnd(msgid1, &message1, TAILLE_MSG, message1.type);
        //printf("Envoi: %s\n", message1.texte);
    }
    return;
}



void send_chargename(int indice, char *argv[])
{
    int nb_byte = strlen(argv[indice]);
    int nb_envoi = nb_byte/20;
    int cpt = 0;
    message1.t_chemin = nb_byte;
    while( cpt <= nb_envoi) {
        memset(message1.texte, 0, TAILLE_MSG);
        strncpy(message1.texte, argv[indice], 20);
        msgsnd(msgid1, &message1, TAILLE_MSG, message1.type);
        argv[indice] += 20;
        cpt ++;
    }
}


/* Reçoit en IPC et affiche le résultat d'un protocole */
void printREP(int nbBoat)
{
    int cpt = 0;
    while(cpt < nbBoat) {
                msgrcv(msgid2, &reponse, TAILLE_MSG, 0, 0);
                nbBoat = reponse.nbBoat;
                printf("Boat_%d : %s\n", cpt+1, reponse.texte);
		cpt ++;
    }
    return;
}



int main(int argc, char *argv[])
{
    /* Gestion des signaux */
    action.sa_handler = hand;
    sigaction(SIGINT, &action, NULL);

    reponse.type = 3;

    if (argc < 2) {
	menu();
	return 0;
    }

    /* Création Files de message IPC */
    if ((msgid1 = msgget(CLE_MSG, IPC_CREAT|0666)) == -1) {
        perror("msgget1");
        exit(1);
    }
    if ((msgid2 = msgget(CLE_MEM, IPC_CREAT|0666)) == -1) {
        perror("msgget2");
        exit(1);
    }
    if (argc == 2) {

	/* Envoie commande list */
        if (strcasecmp(argv[1],"LIST")==0) {
            message1.type = 5;
	    msgsnd(msgid1, &message1, TAILLE_MSG, message1.type);

	    /* Reception reponse */
	    int cpt = 0;
	    printf(" ----LISTE DES BOATS LOCALISES----\n\n");
    	    printf("| IDENTIFIANT  |        IP        |\n");
    	    printf("|______________|__________________|\n");
	    while(reponse.type != 1) {
		msgrcv(msgid2, &reponse, TAILLE_MSG, 0, 0);
		if (reponse.type != 1 && cpt%2 == 0) {
		    printf("| %s       |", reponse.texte);
		    cpt++; }
		else if (reponse.type != 1 && cpt%2 == 1) {
                    printf(" %s |\n", reponse.texte);
	            cpt++; }
	    }
	    if (cpt == 0) printf("AUCUN BOAT POUR LE MOMENT\n");
	return 0;
        }
    return 0;
    }
    else {
        if (strcasecmp(argv[1],"STAT")==0) {
	    printf("Stat\n");
	    message1.type = 1;
	    message1.nbBoat = argc - 2;
	    send(2, argc, argv);

	    printREP(message1.nbBoat);
        }
        else if (strcasecmp(argv[1],"QUIT")==0) {
	    printf("Quit\n");
            message1.type = 2;
	    message1.nbBoat = argc - 2;

	    send(2, argc, argv);
	    printREP(message1.nbBoat);
        }
        if (strcasecmp(argv[1],"UPLOAD")==0 && argc > 3) {
	    printf("Upload\n");
	    message1.type = 3;
	    message1.nbBoat = argc - 3;

	    send_chargename(2, argv);
	    send(3, argc, argv);
	    printREP(message1.nbBoat);
        }
        else if (strcasecmp(argv[1],"EXECUTE")==0 && argc > 3) {
	    printf("Execute\n");
            message1.type = 6;
	    message1.nbBoat = argc - 3;

	    send_chargename(2, argv);
      	    send(3, argc, argv);
	    printREP(message1.nbBoat);
	   }
	   else if (strcasecmp(argv[1],"DELETE")==0 && argc > 3) {
            printf("Delete\n");
	    message1.type = 4;
            message1.nbBoat = argc - 3;

	    send_chargename(2, argv);
            send(3, argc, argv);
            printREP(message1.nbBoat);
        }
        else if (strcasecmp(argv[1],"RESULT")==0 && argc == 4) {
            printf("Result\n");
            message1.type = 7;
            message1.nbBoat = argc - 3;

            send_chargename(2, argv);
            send(3, argc, argv);
            int cpt = 0;
            msgrcv(msgid2, &reponse, TAILLE_MSG, 0, 0);
            printf("%s", reponse.texte);
            while(cpt < reponse.nbOctet-1) {
                cpt ++;
                msgrcv(msgid2, &reponse, TAILLE_MSG, 0, 0);
                printf("%s", reponse.texte);
            }
        }
    }

    return 0;
}


