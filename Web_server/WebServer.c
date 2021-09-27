/** Include files **/
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
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/socket.h>


#include "libreseau.h"
#include "libthread.h"
#include "libipc.h"


/** Some constants **/

#define WEB_DIR  "./www"
#define PAGE_NOTFOUND "error.html"
#define MAX_BUFFER 1024

#define CODE_OK  200
#define CODE_NOTFOUND 404



struct sigaction action;

int msgid1;
int idshm;
Mem_partage web_c;
Mem_partage *webc = &web_c;

char buffer[MAX_BUFFER];
char cmd[MAX_BUFFER];
char page[MAX_BUFFER];
char proto[MAX_BUFFER];
char path[MAX_BUFFER];
char type[MAX_BUFFER];
char buffer_contentlength[MAX_BUFFER];
char data_receive[MAX_BUFFER];
char endofline_delimiter[MAX_BUFFER];
char buffer_token2[MAX_BUFFER];
char equal_delimiter[MAX_BUFFER];
char boundarytemp[MAX_BUFFER];
char boundary[MAX_BUFFER];
char* buffer_boundary;
char filename[MAX_BUFFER];
char buffer_line_message[MAX_BUFFER];
char boatname[MAX_BUFFER];

int entete = 0;
int contentLength;
void* verrou;

void get_method(FILE** dialogue, int s, char *file);
void post_method(FILE** dialogue, int s);
void getname(char* ligne);
void getboundary(void);
void getcontentLength(void);


void LIST(void);
void UPLO(FILE** dialogue);
void PROC(FILE** dialogue, char *consigne, int needCHARGE);

typedef struct {
  char *port;
  char *ip_destination;
}parametres;

typedef struct {
        int socket;
}param_TCP;


void hand (int sig) {
  if (sig == SIGINT) {
    printf("\nFin du programme\n");
    msgctl(msgid1, IPC_RMID, 0);
    exit(-1);}
}





  int gestionClientTCP(void* parametres) {
      printf("\nGestion client lancé\n");
      param_TCP* p = parametres;
      int s = p->socket;
      entete=0;

      /* Obtient une structure de fichier */
      FILE *dialogue=fdopen(s,"a+");
      if(dialogue==NULL){ perror("gestionClient.fdopen"); exit(EXIT_FAILURE); }
      //printf("dialogue != NULL\n");

      while(1) {
         /* Recuperation du message reçu -> ligne */
         if(fgets(buffer,MAX_BUFFER,dialogue)==NULL) return 0; //exit(-1); 
         if(sscanf(buffer,"%s %s %s",cmd,page,proto)!=3) exit(-1);
         printf("cmd : %s\npage : %s\nproto : %s\n",cmd,page,proto);
         while(fgets(buffer,MAX_BUFFER,dialogue)!=NULL) {
            /* On récupère la ligne Content-Length */
            if (strncmp(buffer,"Content-Length:",15)==0) {
                getcontentLength();
            }

            /* Récupération du boundary */
            if(strncmp(buffer,"Content-Type: multipart",23)==0) {
                strcpy(boundary,buffer);
                getboundary();
            }

            if(strcmp(buffer,"\r\n")==0) break;
         }
         if(strcasecmp(cmd,"GET")==0) {
             get_method(&dialogue,s, "www/menuorigin.html");
         }

         else if(strcasecmp(cmd,"POST")==0) {
             post_method(&dialogue,s);
         }
         /* Termine la connexion */
      }
      fclose(dialogue);
      printf("\nfermeture dialogue\n");
      return 0;
  }




/* Crée une page HTML qui affiche le résultat d'une commande issue du webserver */
void receive() {
    FILE* fichier = NULL;
    fichier = fopen("www/reponse.html", "w");

    fputs("<!DOCTYPE html>\n<html>\n<body>\n<h1>RESULT</h1>\n<p1>", fichier);
    while(1) {
	while(webc->type != 0);
	fputs(webc->reponse, fichier);
	webc->type = 1;
	if(webc->finish == 1) break;
    }
    fputs("</p1>\n</body>\n</html>\n", fichier);
    fclose(fichier);
}




/* Crée une page HTML affichant la liste des boats trouvés après la commande LIST */
void receive_list() {

  FILE* fichier1 = NULL;
  FILE* fichier2 = NULL;

  fichier1 = fopen("www/menu.html","w");
  fichier2 = fopen("www/menusave.html","r");
  fputs("<!DOCTYPE html><html><head><link rel=\"icon\" href=\"data:,\"></head><body><style>body {background-color: powderblue;}h1   {color: blue;}p    {color: red;}</style><h1>Bienvenue sur le WebServer de Sameo</h1><h2>Commande LIST</h2>    <form method=\"post\"><input type=\"submit\" name=\"LIST\" value=\"LIST\"  />\n</form>",fichier1);

    fputs("<p>",fichier1);

    /* Ecriture des noms des boat dans le HTML */
    while(1) {
      while(webc->type != 0);
      fputs(webc->reponse, fichier1);
      fputs("<br>",fichier1);
      webc->type = 1;
      if(webc->finish == 1) break;
    }
    fputs("</p>\n", fichier1);

    /* Recupération taille */
    fseek(fichier2, 0, SEEK_END);
    int data_size = ftell(fichier2);
    rewind(fichier2);

    /* Copie du reste de la page HTML */
    char buf[1];
    memset(buf,0,1);
    for(int cpt = 0; cpt<data_size; cpt++) {
      fread(buf,1,1,fichier2);
      fwrite(buf,1,1,fichier1);
    }

    fclose(fichier1);
    fclose(fichier2);
}




/* Methode GET qui fourni au client le char file (.HTML)*/
void get_method(FILE** dialogue, int s, char *file) {

    int code=CODE_OK;
    struct stat fstat;
    strcpy(path,file);

    if(stat(path,&fstat)!=0 || !S_ISREG(fstat.st_mode)){
        sprintf(path,"%s/%s",WEB_DIR,PAGE_NOTFOUND);
        code=CODE_NOTFOUND;
     }

    fprintf(*dialogue,"HTTP/1.0 %d\r\n",code);
    fprintf(*dialogue,"Server: CWeb\r\n");
    fprintf(*dialogue,"Content-type: %s\r\n",type);
    fprintf(*dialogue,"Content-length: %ld\r\n",fstat.st_size);
    fprintf(*dialogue,"\r\n");
    fflush(*dialogue);
    int fd=open(path,O_RDONLY);

    if(fd>=0){
        int bytes;

        while((bytes=read(fd,buffer,MAX_BUFFER))>0) {
           write(s,buffer,bytes);
        }

        close(fd);
    }
return;
}



/* Méthode POST */
void post_method(FILE** dialogue, int s) {

    memset(boatname,0,MAX_BUFFER);
    strcpy(endofline_delimiter,"\r\n");

    if(contentLength==5){
      printf("Vous n'avez pas rentrer de chaines de caractères\n");
      get_method(dialogue,s, "www/menu.html");
      return;
    }
    fread(data_receive,1,5,*dialogue);

    /* Mutex (accé limité à la mémoire partagée) */
    P(verrou);
    if(strncmp(data_receive,"LIST",4)==0) {
        LIST();
	receive_list();
        get_method(dialogue,s,"www/menu.html");
        V(verrou);
        return;
    }
    else if((strncmp(data_receive,"STAT",4)==0)||(strncmp(data_receive,"QUIT",4)==0)) {
	char consigne[5];
	memset(consigne, 0, 5);
	strncpy(consigne, data_receive, 4);
	PROC(dialogue, consigne, 0);
        receive();
    }
    else if((strncmp(data_receive,"EXEC",4)==0)||(strncmp(data_receive,"DELE",4)==0)||(strncmp(data_receive,"RESU",4)==0)) {
        char consigne[5];
        memset(consigne, 0, 5);
        strncpy(consigne, data_receive, 4);
        PROC(dialogue, consigne, 1);
	receive();
    }
    else {
        UPLO(dialogue);
        printf("boatname %s\n", boatname);
	receive();
    }
    V(verrou);
    get_method(dialogue,s, "www/reponse.html");
    return;

}



/* Recupère le filename (charge) dans la méthode UPLOAD */
void getname(char* ligne) {

  strcpy(buffer_token2,ligne);
  char* equal_delimiter = "=";
  char* buffer_filename = strtok(buffer_token2,equal_delimiter);

  buffer_filename = strtok(NULL,equal_delimiter);
  buffer_filename = strtok(NULL,equal_delimiter);

  char* quotesdelimiter = "\"";
  strcpy(filename,"charge/");
  strcat(filename,strtok(buffer_filename,quotesdelimiter));

}



/* Recupère le boundary de l'entete HTML */
void getboundary(void) {

    buffer_boundary = strtok(boundary,"=");
    buffer_boundary= strtok(NULL,endofline_delimiter);
    strcpy(boundarytemp,"--");
    strcat(boundarytemp,buffer_boundary);
    strcat(boundarytemp,"--");
}



/* Recupère le contentLength de l'entete HTML */
void getcontentLength(void) {

  char* space_delimiter= strtok(buffer," ");
  while (space_delimiter != NULL) {
      strcpy(buffer_contentlength,space_delimiter);
      space_delimiter = strtok(NULL, "-");
  }
  contentLength=atoi(buffer_contentlength);
}



/** Différentes commandes **/
void LIST(void) {

  printf("LIST\n");
  memset(webc->consigne, 0, 5);
  strcpy(webc->consigne,"LIST");
  webc->type = 1;
}


void PROC(FILE** dialogue, char *consigne, int needCHARGE) {

  printf("%s\n", consigne);
  memset(webc->boat, 0, 7);
  memset(webc->consigne, 0, 5);
  memset(webc->charge, 0, TAILLE_MSG);

  strcpy(webc->consigne, consigne);
  char* buffer_proc = malloc(MAX_BUFFER);
  memset(buffer_proc,0,MAX_BUFFER);
  fread(buffer_proc,1,12+contentLength-17,*dialogue);
  char* motread=strtok(buffer_proc,"&");

  if (needCHARGE == 1) {
      strcpy(webc->charge, motread);
      printf("charge %s\n", webc->charge);
      strcpy(buffer_proc,strtok(NULL,"="));
      strcpy(buffer_proc,strtok(NULL,"\n"));
      strcpy(webc->boat, buffer_proc);
      printf("bot : %s\n", webc->boat);
      webc->type = 1;
  }
  else {
      strcpy(webc->boat, motread);
      printf("charge %s\n", webc->boat);
      webc->type = 1;
  }
  free(buffer_proc);
}




void UPLO(FILE** dialogue) {

  memset(webc->boat, 0, 7);
  memset(webc->consigne, 0, 5);
  memset(webc->charge, 0, TAILLE_MSG);
  printf("UPLOAD\n");
  FILE* fichier = NULL;

  while(strcmp(fgets(buffer,MAX_BUFFER,*dialogue),"\r\n")!=0){
      entete+=strlen(buffer);
      printf("buffer while1 :%s:\n",buffer);
  }
  fgets(buffer,MAX_BUFFER,*dialogue);
  entete+=strlen(buffer);
  strncpy(boatname,buffer,strlen(buffer)-1);

  while(fgets(buffer,MAX_BUFFER,*dialogue)!=NULL){
      entete+=strlen(buffer);
      if(strncmp(buffer,"Content-Disposition:",20)==0) {
          getname(buffer);
          printf("strlen buffer name vide %ld\n", strlen(filename));
          if(strcmp(filename,"\r\n")==0) {
              fclose(*dialogue);
              return;
          }
          fichier = fopen(filename,"wb+");
      }
      if(strcmp(buffer,"\r\n")==0) break;
  }

  /* Recupération et écriture du fichier dans le dossier Web_server/charge */
  for (unsigned int compteur = 0; compteur<contentLength-entete-strlen(boundarytemp)-11; compteur++) {
      fread(data_receive,1,1,*dialogue);
      fwrite(data_receive,1,1,fichier);
  }
  fgets(buffer,MAX_BUFFER,*dialogue);
  fclose(fichier);

  memset(webc->consigne,0,5);
  memset(webc->boat,0,7);
  memset(webc->charge,0,TAILLE_MSG);

  printf("boatna :%s:\n",boatname);
  strncpy(webc->boat,boatname,6);
  strcpy(webc->charge,"../Web_server/");
  strcat(webc->charge,filename);
  printf("boatname :%s:\n", webc->boat);
  printf("webc->consigne :%s:\nwebc->charge :%s:\n", webc->consigne, webc->charge);
  webc->type = 1;
}




int boucleServeurTCP(int ecoute, int (*traitement)(void*))
{
    int communication;
    while(1)
    {
  /* Attente d'une connexion */
        if((communication=accept(ecoute,NULL,NULL))<0) {printf("!:\n"); return -1;}

        param_TCP param_thread;
        param_thread.socket = communication;
        creer_process(traitement, &param_thread , sizeof(param_thread));
        printf("Lancement Thread communication:%d:\n",communication);
    }
}





/* Fonction utilisée dans le thread TCP */
/* Lancement d'une socket TCP et mise en écoute des clients */
void TCP_f(void *p_param_TCP) {
    printf("Thread TCP\n");

    /* Récupération des arguments : */
    parametres *param = p_param_TCP;
    char *port = param -> port;

    /* Initialisation socket TCP: */
    int s_TCP = initialisationServeurTCP(port,MAX_CONNEXIONS);
    printf("Initialisation serveur TCP réussie\n");

    /* boucle de communication */
    boucleServeurTCP(s_TCP, gestionClientTCP);

    //free(param -> port);

    close(s_TCP);

    return;
}
void (*p_TCP)(void *) = &TCP_f;





/** Main procedure **/

int main(void){

idshm = shmget (CLE_MEM, 200 , IPC_CREAT | 0666);
webc = shmat(idshm , NULL , 0);
verrou = init_verrou();
parametres *p_param_TCP = malloc(sizeof(parametres));
p_param_TCP->port = malloc(5);
strcpy(p_param_TCP->port,"8080");

int taille = 5+sizeof(parametres);

creer_process(p_TCP, p_param_TCP, taille);
while(1);

return 0;

}
