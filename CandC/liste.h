
typedef struct Cellule Cellule;
struct Cellule {
    char* port;
    char* ip;
    char* nom;
    int socket;
    int connect;
    Cellule* suiv; };

typedef Cellule* Liste;

void ajout_tete (Liste *l1, char* info);
void impression_liste(Liste l1);
void suppression_tete(Liste *l1);
void detruire_liste(Liste *l1);
bool appartient(Liste liste, char* nom);
void initialisation(Liste *pl);
char* ip_boat(Liste liste, char* nom);
void pointeur_boat(Liste *pl1, Liste liste, char* nom);
void detruire_ifCONNECT(Liste *l1);
void suppression_ifCONNECT(Liste *l1);
