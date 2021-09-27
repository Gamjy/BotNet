
typedef struct Cellule Cellule;
struct Cellule {
    char* port;
    char* ip;
    char* nom;
    Cellule* suiv; };

typedef Cellule* Liste;

void ajout_tete (Liste *l1, char* info);
void impression_liste(Liste l1);
void suppression_tete(Liste *l1);
void detruire_liste(Liste *l1);
bool appartient(Liste liste, char* nom);
void initialisation(Liste *pl);

