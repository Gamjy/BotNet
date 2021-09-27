#ifndef libipc_h
#define libipc_h


#define CLE_MSG (key_t)1000
#define CLE_MEM (key_t)500
#define TAILLE_TXT 20
#define TAILLE_MSG 50


typedef struct Cellule Cellule;


/* Structure d'un message IPC */
typedef struct {
  long type;
  int nbBoat;
  int nbOctet;
  int t_chemin;
  char texte[TAILLE_MSG];
} Msg_requete;


/* Structure d'une mémoire partagée */
typedef struct {
  long type;
  int finish;
  char boat[7];
  char consigne[5];
  char charge[TAILLE_MSG];
  char reponse[TAILLE_MSG];
} Mem_partage;


#endif

