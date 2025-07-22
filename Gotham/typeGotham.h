/*********************************************** 
* 
* @Autores: Alexandre Contreras y Armand López
* @Propósito: Definir las estructuras necesarias para la configuración y gestión del 
*             servidor Gotham, incluyendo la lista de clientes, workers y sus sockets 
*             asociados. 
* @Fecha de creación: 27 de octubre de 2024
* @Última modificación: 4 de enero de 2025
* 
************************************************/

#ifndef _TYPE_GOTHAM_CUSTOM_H_
#define _TYPE_GOTHAM_CUSTOM_H_

#include "../Libs/LinkedList/fleckLinkedList.h"
#include "../Libs/LinkedList/workerLinkedList.h"

//Constants pròpies
#define MAX_CLIENTS 10

typedef struct {
    int socket_fd; 
    char type; //'f' = fleck; 'w' = worker
} Client; 

typedef struct {
    int fleck_listen_socket;
    int worker_listen_socket; 
    int n_clients; 
    Client* clients; 
    int max_fd;               
    fd_set active_fds;
    int n_enigmas;
    int n_harleys;
    FleckLinkedList fleck_list;   
    WorkerLinkedList worker_list; 
    int fd_log;     
} GothamServer;

typedef struct {
    char* fleck_ip; 
    int fleck_port; 
    char * worker_ip; 
    int worker_port; 
} GothamConfig; 

#endif // _TYPE_GOTHAM_CUSTOM_H_