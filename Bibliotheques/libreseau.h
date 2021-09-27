#ifndef libreseau_h
#define libreseau_h

#define MAX_UDP_MESSAGE 20
#define MAX_LIGNE 50
#define MAX_CONNEXIONS 5
#define MAX_TAMPON 30

char ligne[MAX_LIGNE];

/* Fonction d'initialisation d'une socket UDP */
int initialisationSocketUDP(char *service);



/** Utilisées dans Boat.c **/
/* Fonction qui crée une socket UDP */
int creationsocketUDP(char *hote, char *service, void **handle);

/* Fonction qui envoie un char *message de taille int taille en UDP */
void envoimessageUDP(int s, void *handle, unsigned char *message, int taille);


/* Fonction d'initialisation d'une socket TCP */
int initialisationServeurTCP(char *service,int connexions);


/** Utilisées dans le CandC.c **/
/* Fonction qui permet d'écouter et de traiter par la méthode 'traitement' des messages UDP */
int boucleServeurUDP(int s,int (*traitement)(unsigned char *,int, char*));

/* Fonction permettant de se connecter à un serveur TCP */
int connexionServeurTCP(char *hote,char *service);

/* Fonction d'initialisation d'une socket UDP */
/*int initialisationSocketUDP(char *service);*/

#endif //libreseau
