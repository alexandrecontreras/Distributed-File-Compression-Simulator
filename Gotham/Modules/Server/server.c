/*********************************************** 
* 
* @Autores: Alexandre Contreras, Armand López. 
* 
* @Finalidad: Implementar la inicialización y ejecución del servidor Gotham, 
*             incluyendo la configuración de los sockets en modo escucha, 
*             el manejo de conexiones entrantes de flecks y workers, y el 
*             procesamiento de tramas enviadas por los clientes conectados. 
* 
* @Fecha de creación: 30 de octubre de 2024. 
* 
* @Última modificación: 4 de enero de 2025. 
* 
************************************************/

#include "server.h"

/*********************************************** 
* 
* @Finalidad: Inicializar el servidor Gotham, incluyendo los sockets en modo escucha 
*             para flecks y workers, las listas enlazadas de clientes, y los conjuntos 
*             de descriptores de archivo. 
* 
* @Parámetros: 
* in/out: server = Puntero a la estructura `GothamServer` que se inicializará con los valores y estructuras necesarias. 
* in: config = Puntero a la estructura `GothamConfig` que contiene la configuración del servidor (IP, puertos, etc.). 
* 
* @Retorno: Ninguno. La función finaliza la ejecución del programa con `exit(EXIT_FAILURE)` si ocurre un error crítico. 
* 
************************************************/
int SRV_initGothamServer(GothamServer *server, GothamConfig *config) {
    //inicialitzem socket d'escolta de flecks
    server->fleck_listen_socket = SOCKET_initListenSocket(config->fleck_ip, config->fleck_port, MAX_CLIENTS);
    if (server->fleck_listen_socket < 0) {
        IO_printStatic(STDOUT_FILENO, "Error: Could not initialize Fleck listen socket\n");
        EXIT_freeMemory(config, NULL);
        return -1;
    }

    //inicialitzem socket d'escolta de workers
    server->worker_listen_socket = SOCKET_initListenSocket(config->worker_ip, config->worker_port, MAX_CLIENTS);
    if (server->worker_listen_socket < 0) {
        IO_printStatic(STDOUT_FILENO, "Error: Could not initialize Worker listen socket\n");
        close(server->fleck_listen_socket); 
        EXIT_freeMemory(config, NULL);
        return -1;
    }

    //inicialitzem les llistes de workers i flecks
    server->fleck_list = FLECK_LINKEDLIST_create();
    server->worker_list = WORKER_LINKEDLIST_create();

    //inicialitzem array dinàmic de socket descriptors
    server->clients = (Client*) malloc (sizeof(Client)); 
    if (server->clients == NULL) return -1; 

    server->n_clients = 0; 
    server->n_enigmas = 0; 
    server->n_harleys = 0; 

    //inicialitzem file descriptors i fd_set
    FD_ZERO(&server->active_fds);
    FD_SET(server->fleck_listen_socket, &server->active_fds);
    FD_SET(server->worker_listen_socket, &server->active_fds);
    server->max_fd = (server->fleck_listen_socket > server->worker_listen_socket) ? server->fleck_listen_socket : server->worker_listen_socket;

    return 0;
}

/*********************************************** 
* 
* @Finalidad: Ejecutar el servidor Gotham, gestionando conexiones entrantes de flecks y workers, 
*             y procesando las tramas recibidas de los clientes conectados. 
* 
* @Parámetros: 
* in/out: server = Puntero a la estructura `GothamServer` que contiene la información del servidor 
*                  y las conexiones activas. 
* in: exit_program = Puntero a una variable `volatile int` que indica si se debe finalizar la ejecución del servidor. 
* 
* @Retorno: Ninguno. 
* 
************************************************/
void SRV_runGothamServer(GothamServer *server, volatile int* exit_program) {
    while (!(*exit_program)) {
        fd_set read_fds = server->active_fds;
        int activity = select(server->max_fd + 1, &read_fds, NULL, NULL, NULL);

        if (activity < 0) {
            IO_printStatic(STDOUT_FILENO, "Error: Error en select.\n");
            break;
        }

        //comprovem connexions entrants de Flecks
        if (FD_ISSET(server->fleck_listen_socket, &read_fds)) {
            HANDLE_acceptNewConnection(server->fleck_listen_socket, server, true);
        }

        //comprovem connexions entrants de Workers
        if (FD_ISSET(server->worker_listen_socket, &read_fds)) {
            HANDLE_acceptNewConnection(server->worker_listen_socket, server, false);
        }

        for (int i = 0; i < server->n_clients; i++) {
            int client_socket = server->clients[i].socket_fd;
            if (client_socket != -1 && FD_ISSET(client_socket, &read_fds)) {
                HANDLE_handleFrame(server, client_socket, server->clients[i].type);
            }
        }
    }
}