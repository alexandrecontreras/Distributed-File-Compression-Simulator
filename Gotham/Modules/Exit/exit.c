/*********************************************** 
* 
* @Autores: Alexandre Contreras, Armand López. 
* 
* @Finalidad: Implementar funciones para la gestión y limpieza de memoria dinámica 
*             asociada al servidor Gotham, así como el cierre seguro de todas las 
*             conexiones activas y descriptores de archivo. 
* 
* @Fecha de creación: 30 de octubre de 2024. 
* 
* @Última modificación: 4 de enero de 2025. 
* 
************************************************/

#include "exit.h"

/*********************************************** 
* 
* @Finalidad: Liberar la memoria dinámica asociada a las configuraciones de Gotham y 
*             las estructuras del servidor Gotham, incluyendo listas enlazadas y clientes. 
* 
* @Parámetros: 
* in/out: gotham = Puntero a la estructura `GothamConfig` cuya memoria será liberada. 
* in/out: server = Puntero a la estructura `GothamServer` cuya memoria, listas y clientes 
*                  serán liberados. 
* 
* @Retorno: Ninguno. 
* 
************************************************/
void EXIT_freeMemory(GothamConfig* gotham, GothamServer* server) {
    free(gotham->fleck_ip);
    free(gotham->worker_ip);

    if(server != NULL) {
        FLECK_LINKEDLIST_destroy(&server->fleck_list);  
        WORKER_LINKEDLIST_destroy(&server->worker_list);

        if (server->clients != NULL) {
            free(server->clients);
            server->clients = NULL;  
        }
    }
}

/*********************************************** 
* 
* @Finalidad: Cerrar todas las conexiones activas en el servidor Gotham, incluyendo 
*             sockets de clientes, flecks y workers, y limpiar los descriptores de 
*             archivo correspondientes. 
* 
* @Parámetros: 
* in/out: server = Puntero a la estructura `GothamServer` que contiene los sockets y 
*                  listas de descriptores activos que se cerrarán y limpiarán. 
* 
* @Retorno: Ninguno. 
* 
************************************************/
void EXIT_closeAllConnections(GothamServer *server) {
    for (int i = 0; i < server->n_clients; i++) {
        close(server->clients[i].socket_fd); 
        FD_CLR(server->clients[i].socket_fd, &server->active_fds);  
    }

    if (server->fleck_listen_socket != -1) {
        close(server->fleck_listen_socket);
        FD_CLR(server->fleck_listen_socket, &server->active_fds);
        server->fleck_listen_socket = -1; 
    }
    if (server->worker_listen_socket != -1) {
        close(server->worker_listen_socket);
        FD_CLR(server->worker_listen_socket, &server->active_fds);
        server->worker_listen_socket = -1; 
    }

    server->max_fd = -1;
}