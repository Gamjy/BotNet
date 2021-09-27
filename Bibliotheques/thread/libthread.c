#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../libthread.h"


/* Fonction qui déréférence *data afin de lancer une autre fonction et ses paramètres correspondants */
void *f_inter(void *data)
{
	/* Récupération du pointeur de structure Data */
	Data *p_data = data;

	/* On déréférence afin de récupérer la fonction et ses paramètres */
	void (*fonction)(void *) = p_data -> p_fonction;
	void *p_param = p_data -> p_param;
	/*Lancement de la fonction */
	fonction(p_param);
	printf("adresse p_param %p\n", p_param);
	free(p_param);
	free(data);
	pthread_exit(NULL);
}


/* Lancement d'une fonction dans un thread dédié par l'intermédiaire d'une fonction tampon 'f_inter' */
void creer_process(void (*fonction), void *p_param, int taille_parametres)
{
	/* Création du tid */
	pthread_t tid;

	/* Création et remplissage de la structure de type Data */
	Data *p_data = malloc(sizeof(Data));
	if (p_data == NULL) exit(0);
	void *param = p_param;

	p_data -> p_fonction = fonction;
	p_data->p_param = malloc(taille_parametres);
	memcpy(p_data->p_param,param,taille_parametres);

	/* Création et lancement du thread */
	pthread_create(&tid, NULL, f_inter,(void *)p_data);
	pthread_detach(tid);
}


void* init_verrou(void) {
	pthread_mutex_t *mutex = malloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(mutex,NULL);
	return (void*)mutex;
}

int P(void* mutex) {

	int prendre = pthread_mutex_lock((pthread_mutex_t*)mutex);
	if(prendre==0) 
		return 0;
	else 
		return 1;

}

int V(void* mutex) {

	int rendre = pthread_mutex_unlock((pthread_mutex_t*)mutex);
	if(rendre==0) 
		return 0;
	else 
		return 1;
}