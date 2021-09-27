#include <sys/socket.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <netinet/tcp.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <poll.h>
#include "../libreseau.h"


/* Fonction d'initialisation d'une socket UDP */
int initialisationSocketUDP(char *service)
{
    struct addrinfo precisions,*resultat,*origine;
    int statut;
    int s;

    /* Construction de la structure adresse */
    memset(&precisions,0,sizeof precisions);
    precisions.ai_family=AF_UNSPEC;
    precisions.ai_socktype=SOCK_DGRAM;
    precisions.ai_flags=AI_PASSIVE;
    statut=getaddrinfo(NULL,service,&precisions,&origine);
    if(statut<0){ perror("initialisationSocketUDP.getaddrinfo"); exit(EXIT_FAILURE); }
    struct addrinfo *p;
    for(p=origine,resultat=origine;p!=NULL;p=p->ai_next)
    if(p->ai_family==AF_INET6){ resultat=p; break; }

    /* Creation d'une socket */
    s=socket(resultat->ai_family,resultat->ai_socktype,resultat->ai_protocol);
    if(s<0){ perror("initialisationSocketUDP.socket"); exit(EXIT_FAILURE); }

    /* Options utiles */
    int vrai=1;
    if(setsockopt(s,SOL_SOCKET,SO_REUSEADDR,&vrai,sizeof(vrai))<0){
        perror("initialisationServeurUDPgenerique.setsockopt (REUSEADDR)");
        exit(-1);
    }

    /* Specification de l'adresse de la socket */
    statut=bind(s,resultat->ai_addr,resultat->ai_addrlen);
    if(statut<0) {perror("initialisationServeurUDP.bind"); exit(-1);}

    /* Liberation de la structure d'informations */
    freeaddrinfo(origine);
    return s;
}







/* Fonction qui crée une socket UDP */
int creationsocketUDP(char *hote,char *service, void **handle)
{
    struct addrinfo precisions,*resultat,*origine;
    int statut;
    int s;

    /* Creation de l'adresse de socket */
    memset(&precisions,0,sizeof precisions);
    precisions.ai_family=AF_UNSPEC;
    precisions.ai_socktype=SOCK_DGRAM;
    statut=getaddrinfo(hote,service,&precisions,&origine);
    if(statut<0){ perror("messageUDPgenerique.getaddrinfo"); exit(EXIT_FAILURE); }
    struct addrinfo *p;
    for(p=origine,resultat=origine;p!=NULL;p=p->ai_next)
    if(p->ai_family==AF_INET6){ resultat=p; break; }

    *handle = malloc(sizeof(struct addrinfo));
    *((struct addrinfo *)*handle) = *resultat;

    /* Creation d'une socket */
    s=socket(resultat->ai_family,resultat->ai_socktype,resultat->ai_protocol);
    if(s<0){ perror("messageUDPgenerique.socket"); exit(EXIT_FAILURE); }

    /* Option sur la socket */
    int vrai=1;
    if(setsockopt(s,SOL_SOCKET,SO_BROADCAST,&vrai,sizeof(vrai))<0){
        perror("initialisationServeurUDPgenerique.setsockopt (BROADCAST)");
        exit(-1);
    }

    /* Liberation de la structure d'informations */
    freeaddrinfo(origine);
    return s;
}




/* Fonction qui envoie un char *message de taille int taille en UDP */
void envoimessageUDP(int s, void *handle, unsigned char *message, int taille)
{
    struct addrinfo *resultat = handle;
    int nboctets=sendto(s,message,taille,0,resultat->ai_addr,resultat->ai_addrlen);
    if(nboctets<0){ perror("messageUDPgenerique.sento"); exit(EXIT_FAILURE); }
}



/* Fonction d'initialisation d'une socket TCP */
int initialisationServeurTCP(char *service,int connexions)
{
    struct addrinfo precisions,*resultat,*origine;
    int statut;
    int s;

    /* Construction de la structure adresse */
    memset(&precisions,0,sizeof precisions);
    precisions.ai_family=AF_UNSPEC;
    precisions.ai_socktype=SOCK_STREAM;
    precisions.ai_flags=AI_PASSIVE;
    statut=getaddrinfo(NULL,service,&precisions,&origine);
    if(statut<0){ perror("initialisationServeur.getaddrinfo"); exit(EXIT_FAILURE); }
    struct addrinfo *p;
    for(p=origine,resultat=origine;p!=NULL;p=p->ai_next)
    if(p->ai_family==AF_INET6){ resultat=p; break; }

    /* Creation d'une socket */
    s=socket(resultat->ai_family,resultat->ai_socktype,resultat->ai_protocol);
    if(s<0){ perror("initialisationServeur.socket"); exit(EXIT_FAILURE); }

    /* Options utiles */
    int vrai=1;
    if(setsockopt(s,SOL_SOCKET,SO_REUSEADDR,&vrai,sizeof(vrai))<0){
        perror("initialisationServeur.setsockopt (REUSEADDR)");
        exit(EXIT_FAILURE);
    }
    if(setsockopt(s,IPPROTO_TCP,TCP_NODELAY,&vrai,sizeof(vrai))<0){
        perror("initialisationServeur.setsockopt (NODELAY)");
        exit(EXIT_FAILURE);
    }

    /* Specification de l'adresse de la socket */
    statut=bind(s,resultat->ai_addr,resultat->ai_addrlen);
    if(statut<0) return -1;

    /* Liberation de la structure d'informations */
    freeaddrinfo(origine);

    /* Taille de la queue d'attente */
    statut=listen(s,connexions);
    if(statut<0) return -1;

    return s;
}





/* Fonction permettant de se connecter à un serveur TCP */
int connexionServeurTCP(char *hote,char *service)
{
    struct addrinfo precisions,*resultat,*origine;
    int statut;
    int s;

    /* Creation de l'adresse de socket */
    memset(&precisions,0,sizeof precisions);
    precisions.ai_family=AF_UNSPEC;
    precisions.ai_socktype=SOCK_STREAM;
    statut=getaddrinfo(hote,service,&precisions,&origine);
    if(statut<0){ perror("connexionServeur.getaddrinfo"); exit(EXIT_FAILURE); }
    struct addrinfo *p;
    for(p=origine,resultat=origine;p!=NULL;p=p->ai_next)
    if(p->ai_family==AF_INET6){ resultat=p; break; }

    /* Creation d'une socket */
    s=socket(resultat->ai_family,resultat->ai_socktype,resultat->ai_protocol);
    if(s<0){ perror("connexionServeur.socket"); exit(EXIT_FAILURE); }

    /* Connection de la socket a l'hote */
    if(connect(s,resultat->ai_addr,resultat->ai_addrlen)<0) return -1;

    /* Liberation de la structure d'informations */
    freeaddrinfo(origine);

    return s;
}