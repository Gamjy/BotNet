#ifndef libthread_h
#define libthread_h

/* Structure Data à passer en paramètre de f_inter */
typedef struct {
        void (*p_fonction)(void *);
        void *p_param;
        }Data;

/* Fonction tampon permettant de lancer la fonction fournie par le void *data */
void *f_inter(void *data);

/* Permet de lancer un thread pour la fonction passée en paramètre */
void creer_process(void (*p_fonction), void *p_param, int taille_parametres);

void* init_verrou(void);

int P(void* mutex);

int V(void* mutex);


#endif