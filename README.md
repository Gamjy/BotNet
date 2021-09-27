# Projet PSR

Réalisé par Samuel BENAYED et Théo EVRARD

------------------------------------------


L'objectif de ce projet est de mettre en place une architecture de type botnet afin de contrôler et déployer à distance des tâches sur différentes machines.




## Présentation de l'architecture

Le projet est constitué de 4 parties: un programme admin, un Command and Control, un Bot et un Webserver

le C&C fait l'intermédiaire entre les 3 autres parties du projet.


### C&C

Le C&C envoie des ordres et des charges utiles aux bots via le protocole TCP.
Il communique avec l'admin via file de message ipc et avec le Webserver par mémoire partagée

### Bot

Le bot reçoit les ordres du C&C en TCP et renvoie les informations via protocole UDP sur le port 4242.

### Admin

L'admin permet d'envoyer des ordres au C&C via un terminal de commande

### Webserver 

Le Webserver permet de récupérer les ordres de l'utilisateur depuis une page internet et de transmettre les ordres et les charges utiles au C&C.



## Compilation

Pour compiler l'intégralité du projet il suffit de se positionner à la racine et de lancer la commande make.
Une cible make clean est implémentée pour supprimer tous les fichiers objets.

Le makefile à la racine appelle les makefiles présents dans les sous-répertoires.

La compilation est faite avec les flags -Wall -Wextra -Werror


## Utilisation

#### CandC

Pour lancer le programme C&C il suffit de se positionner dans le dossier C&C après la compilation et de lancer ./CandC


#### Bot

Pour lancer le programme du boat il suffit de se positionner dans le dossier Boat et de lancer ./boat


#### Admin

L'utilisation de l'admin est plus particulière : 

commande LIST : ./admin list

commande STAT : ./admin stat [nomdubot1] [nomdubot2] ...

commande QUIT : ./admin quit [nomdubot1] [nomdubot2] ...

commande UPLOAD : ./admin upload [nomdelacharge] [nomdubot1] [nomdubot2] ...

commande EXECUTE : ./admin execute [nomdelacharge] [nomdubot1] [nomdubot2] ...

commande RESULT : ./admin result [idresult] [nomdubot]

commande delete : ./delete [nomdelacharge] [nomdubot1] [nomdubot2] ...


Nous avons également implémenté l'argument all pour l'admin :

./admin STAT all envoie un STAT à tous les bots connectés.

Cet argument all est utilisable sur toutes les commandes sauf result.


#### Webserver

Pour lancer le Webserver il suffit de se positionner dans le dossier Webserver et de lancer ./Webserver

Il faut ensuite se connecter sur un navigateur à l'adresse localhost:8080.

Notre site HTML peut être sensible à certaines maltraitances.


## Bilan

Ce projet nous a permis de mettre en pratique de nombreuses notions de Système et Réseaux : Threads/Mutex, communication TCP et UDP, IPC, mémoire partagée.
Nous avons aussi découvert le protocole HTTP.

