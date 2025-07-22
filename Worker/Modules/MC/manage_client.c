/************************************
*
* @Autores: Alexandre Contreras, Armand López.
*
* @Finalidad: Implementar las funciones de gestión de clientes del servidor.
*
* @Fecha de creación: 30 de octubre de 2024
*
* @Última modificación: 4 de enero de 2025
*
************************************************/


#include "manage_client.h"

/*********************************************** 
* 
* @Finalidad: Agregar un cliente a la lista de clientes del servidor, aumentando el tamaño 
*             dinámicamente y asegurando la sincronización mediante un mutex. 
* 
* @Parámetros: 
* in/out: server = Puntero a la estructura `WorkerServer` que contiene la lista de clientes. 
* in: client_socket = Descriptor del socket del cliente que se añadirá. 
* 
* @Retorno: Ninguno. 
* 
************************************************/
void MC_addClient(WorkerServer *server, int client_socket) {
    //bloquegem l'accés a la llista de clients
    pthread_mutex_lock(&server->clients_mutex);  

    //augmentem el nombre de clients
    server->clients = realloc(server->clients, (server->n_clients + 1) * sizeof(int));  
    if(server->clients == NULL) return;   
    server->clients[server->n_clients] = client_socket;  //afegim el client
    server->n_clients++;

    pthread_mutex_unlock(&server->clients_mutex);  //desbloquegem el mutex
}

/*********************************************** 
* 
* @Finalidad: Eliminar un cliente de la lista de clientes del servidor, cerrando su socket 
*             y redimensionando la lista dinámicamente, asegurando la sincronización con un mutex. 
* 
* @Parámetros: 
* in/out: server = Puntero a la estructura `WorkerServer` que contiene la lista de clientes. 
* in: client_socket = Descriptor del socket del cliente que se eliminará. 
* 
* @Retorno: Ninguno. 
* 
************************************************/
void MC_removeClient(WorkerServer *server, int client_socket) {
    pthread_mutex_lock(&server->clients_mutex);  //bloquegem l'accés a la llista

    //busquem el client i l'eliminem de la llista
    for (int i = 0; i < server->n_clients; i++) {
        if (server->clients[i] == client_socket) {
            //shiftem tots els elements per eliminar aquest client
            for (int j = i; j < server->n_clients - 1; j++) {
                server->clients[j] = server->clients[j + 1];
            }
            server->n_clients--;  

            //redimensionem la llista per ajustar la mida
            if (server->n_clients > 0) {
                server->clients = realloc(server->clients, server->n_clients * sizeof(int));
                if(server->clients == NULL) return;  
            } else {
                free(server->clients);
                server->clients = NULL;
            }
            
            //tanquem el socket del client si no l'hem tancat a sigint handler
            if(client_socket != -1) close(client_socket);
            break;
        }
    }

    pthread_mutex_unlock(&server->clients_mutex);  //desbloquegem el mutex
}

/*********************************************** 
* 
* @Finalidad: Agregar un hilo activo a la lista de hilos del servidor, redimensionando 
*             la lista dinámicamente y asegurando la sincronización mediante un mutex. 
* 
* @Parámetros: 
* in/out: server = Puntero a la estructura `WorkerServer` que contiene la lista de hilos activos. 
* in: thread_id = Identificador del hilo que se añadirá a la lista de hilos activos. 
* 
* @Retorno: Ninguno. 
* 
************************************************/
void MC_addActiveThread(WorkerServer *server, pthread_t thread_id) {
    pthread_mutex_lock(&server->thread_list_mutex);

    server->active_threads = (pthread_t*) realloc(server->active_threads, (server->active_thread_count + 1) * sizeof(pthread_t));
    if (server->active_threads == NULL) {
        IO_printStatic(1, "Error: Unable to resize active_threads array.\n");
        pthread_mutex_unlock(&server->thread_list_mutex);
        return;
    }
    server->active_threads[server->active_thread_count++] = thread_id;

    pthread_mutex_unlock(&server->thread_list_mutex);
}