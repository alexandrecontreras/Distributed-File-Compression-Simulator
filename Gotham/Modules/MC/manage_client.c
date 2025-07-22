/*********************************************** 
* 
* @Autores: Alexandre Contreras, Armand López. 
* 
* @Finalidad: Implementar funciones para gestionar la conexión, desconexión y mantenimiento 
*             de clientes en el servidor Gotham. Esto incluye flecks y workers, la 
*             asignación de main workers, y la actualización dinámica de las listas 
*             enlazadas y descriptores de archivo. 
* 
* @Fecha de creación: 30 de octubre de 2024. 
* 
* @Última modificación: 4 de enero de 2025. 
* 
************************************************/

#include "manage_client.h"

/*********************************************** 
* 
* @Finalidad: Actualizar el descriptor de archivo máximo (`max_fd`) en el servidor Gotham, 
*             considerando los sockets en escucha y los sockets de los clientes conectados. 
* 
* @Parámetros: 
* in/out: server = Puntero a la estructura `GothamServer` que contiene los sockets en escucha 
*                  y los clientes conectados. 
* 
* @Retorno: Ninguno. 
* 
************************************************/
void MC_updateMaxFD(GothamServer *server) {
    server->max_fd = (server->fleck_listen_socket > server->worker_listen_socket) ? server->fleck_listen_socket : server->worker_listen_socket;

    //recorrem llista de sockets de clients
    for (int i = 0; i < server->n_clients; i++) {
        if (server->clients[i].socket_fd > server->max_fd) {
            server->max_fd = server->clients[i].socket_fd;
        }
    }
}

/*********************************************** 
* 
* @Finalidad: Agregar un nuevo cliente (fleck o worker) al servidor Gotham, 
*             redimensionando el array de clientes y actualizando el descriptor 
*             de archivo máximo (`max_fd`). 
* 
* @Parámetros: 
* in/out: server = Puntero a la estructura `GothamServer` que contiene la lista de clientes conectados. 
* in: client_socket = Descriptor del socket del cliente que se agregará. 
* in: client_type = Tipo de cliente (`'f'` para fleck o `'w'` para worker). 
* 
* @Retorno: Ninguno. 
* 
************************************************/
void MC_addClient(GothamServer *server, int client_socket, char client_type) {
    //redimensionem array de sockets de clients
    server->clients = realloc(server->clients, (server->n_clients + 1) * sizeof(Client));
    if (server->clients == NULL) return;

    server->clients[server->n_clients].socket_fd = client_socket;
    server->clients[server->n_clients].type = client_type;
    server->n_clients++;

    FD_SET(client_socket, &server->active_fds);
    MC_updateMaxFD(server);
}

/*********************************************** 
* 
* @Finalidad: Agregar un nuevo fleck al servidor Gotham, creando una estructura `Fleck` 
*             con la información proporcionada y añadiéndola a la lista enlazada de flecks. 
* 
* @Parámetros: 
* in/out: server = Puntero a la estructura `GothamServer` que contiene la lista enlazada de flecks conectados. 
* in: client_socket = Descriptor del socket del fleck que se conectó. 
* in: username = Nombre del usuario asociado al fleck. 
* in: ip_address = Dirección IP del fleck conectado. 
* in: port_str = Puerto del fleck conectado en formato de cadena. 
* 
* @Retorno: Ninguno. 
* 
************************************************/
void MC_addFleckToServer(GothamServer *server, int client_socket, char* username, char* ip_address, char* port_str) {
    Fleck new_fleck;

    if (username && ip_address && port_str) {
        new_fleck.username = strdup(username);
        new_fleck.ip = strdup(ip_address);
        new_fleck.port = atoi(port_str);  
        new_fleck.socket_fd = client_socket;
        FLECK_LINKEDLIST_add(&server->fleck_list, new_fleck);
        IO_printFormat(STDOUT_FILENO, GREEN "\nNew Fleck connected: %s\n" RESET, new_fleck.username);
    } 
}

/*********************************************** 
* 
* @Finalidad: Agregar un nuevo worker al servidor Gotham, creando una estructura `Worker` 
*             con la información proporcionada y añadiéndola a la lista enlazada de workers. 
*             Determina si el nuevo worker es el principal (main worker) para su tipo. 
* 
* @Parámetros: 
* in/out: server = Puntero a la estructura `GothamServer` que contiene la lista enlazada de workers conectados. 
* in: client_socket = Descriptor del socket del worker que se conectó. 
* in: worker_type = Tipo de worker conectado ("Text" o "Media"). 
* in: ip_address = Dirección IP del worker conectado. 
* in: port_str = Puerto del worker conectado en formato de cadena. 
* 
* @Retorno: 
*           1 = El worker agregado es el principal (main worker) para su tipo. 
*           0 = El worker agregado no es el principal. 
* 
************************************************/
int MC_addWorkerToServer(GothamServer *server, int client_socket, char* worker_type, char* ip_address, char* port_str) {
    Worker new_worker;

    if (worker_type && ip_address && port_str) {
        new_worker.worker_type = strdup(worker_type);
        new_worker.ip = strdup(ip_address);
        new_worker.port = atoi(port_str);  
        new_worker.socket_fd = client_socket;
        if (strcmp(worker_type, "Text") == 0) {
            new_worker.is_main = (server->n_enigmas == 0) ? 1 : 0;
            server->n_enigmas++;  
        } 
        else if (strcmp(worker_type, "Media") == 0) {
            new_worker.is_main = (server->n_harleys == 0) ? 1 : 0;
            server->n_harleys++; 
        } 
        WORKER_LINKEDLIST_add(&server->worker_list, new_worker);

        if (strcmp(worker_type, "Text") == 0) {
            IO_printStatic(STDOUT_FILENO, GREEN "\nNew Enigma connected.\n" RESET);
        }
        else {
            IO_printStatic(STDOUT_FILENO, GREEN "\nNew Harley connected.\n" RESET);
        }
    } 

    return new_worker.is_main;
}

/*********************************************** 
* 
* @Finalidad: Eliminar un cliente del servidor Gotham, cerrando su socket, eliminándolo 
*             del conjunto de descriptores activos (`FD_SET`) y ajustando el array de clientes. 
* 
* @Parámetros: 
* in/out: server = Puntero a la estructura `GothamServer` que contiene la lista de clientes conectados. 
* in: client_socket = Descriptor del socket del cliente que se eliminará. 
* 
* @Retorno: Ninguno. 
* 
************************************************/
void MC_removeClientSocket(GothamServer *server, int client_socket) {
    for (int i = 0; i < server->n_clients; i++) {
        if (server->clients[i].socket_fd == client_socket) {
            //tanquem el socket i l'eliminem del fd_set
            close(client_socket);
            FD_CLR(client_socket, &server->active_fds);

            //shiftem array declients
            for (int j = i; j < server->n_clients - 1; j++) {
                server->clients[j] = server->clients[j + 1];
            }

            //redimensionem l'array de clients
            if (server->n_clients > 1) {
                server->clients = realloc(server->clients, (server->n_clients - 1) * sizeof(Client));
                if (server->clients == NULL) return;
            } else {
                free(server->clients); //si estem eliminant l'últim client l'alliberem
                server->clients = NULL;
            }

            server->n_clients--;
            break;
        }
    }
}

/*********************************************** 
* 
* @Finalidad: Verificar si es posible asignar un nuevo main worker para un tipo específico 
*             de worker (`Text` o `Media`) en el servidor Gotham. 
* 
* @Parámetros: 
* in: server = Puntero a la estructura `GothamServer` que contiene el estado actual de los workers. 
* in: worker_type = Cadena que especifica el tipo de worker a verificar ("Text" o "Media"). 
* 
* @Retorno: 
*           0 = No es posible asignar un nuevo main worker (no hay workers disponibles para el tipo). 
*           1 = Es posible asignar un nuevo main worker. 
* 
************************************************/
int MC_canAssignNewMain(GothamServer *server, const char* worker_type) {
    if ((strcmp(worker_type, "Text") == 0 && server->n_enigmas == 0) || 
        (strcmp(worker_type, "Media") == 0 && server->n_harleys == 0)) {
        return 0; 
    }
    return 1; 
}

/*********************************************** 
* 
* @Finalidad: Asignar un nuevo main worker de un tipo específico (`Text` o `Media`) 
*             seleccionando aleatoriamente un worker disponible del servidor Gotham. 
* 
* @Parámetros: 
* in/out: server = Puntero a la estructura `GothamServer` que contiene la lista de workers conectados. 
* in: worker_type = Cadena que especifica el tipo de worker a asignar como main ("Text" o "Media"). 
* 
* @Retorno: Ninguno. 
* 
************************************************/
void MC_assignNewMainWorker(GothamServer *server, char* worker_type) {
    if (!MC_canAssignNewMain(server, worker_type)) {
        //gestionar l'error, potser mostrant un missatge o retornant una notificació, en el acas que no hi hagin mes workers d'aquell tipus
        IO_printFormat(STDOUT_FILENO, RED "No more workers of type %s available.\n" RESET, worker_type);
        return; 
    }

    int total_workers = server->n_enigmas + server->n_harleys;
    int random_index; 
    int valid = 0; 

    while (!valid) {
        random_index = rand() % total_workers; 
        WORKER_LINKEDLIST_gotoIndex(&server->worker_list, random_index);

        if (server->worker_list.error != WORKER_LIST_NO_ERROR) {
            //error en accedir a l'índex
            return;
        }

        WorkerElement* new_main_worker = WORKER_LINKEDLIST_getPointer(&server->worker_list);

        if (new_main_worker && !strcmp(new_main_worker->worker_type, worker_type)) {
            new_main_worker->is_main = 1;  
            valid = 1;
            COMM_sendNewMainWorkerResponse(new_main_worker->socket_fd);
        }
    }
}

/*********************************************** 
* 
* @Finalidad: Mostrar un mensaje en la salida estándar indicando la desconexión de un 
*             worker, diferenciando entre workers principales (main) y secundarios 
*             para los tipos `Text` y `Media`. 
* 
* @Parámetros: 
* in: type = Cadena que indica el tipo de worker desconectado ("Text" o "Media"). 
* in: is_main = Indicador que especifica si el worker desconectado era el principal (1) o no (0). 
* 
* @Retorno: Ninguno. 
* 
************************************************/
void MC_printWorkerDisconnection(char* type, int is_main) {
    if(!strcmp(type, "Text")) {
        if(is_main) {
            IO_printStatic(STDOUT_FILENO, CYAN "\nMain Enigma disconnected\n" RESET); 
        }
        else {
            IO_printStatic(STDOUT_FILENO, BLUE "\nEnigma disconnected\n" RESET); 
        }
    }
    else {
        if(is_main) {
            IO_printStatic(STDOUT_FILENO, CYAN "\nMain Harley disconnected\n" RESET); 
        }
        else {
            IO_printStatic(STDOUT_FILENO, BLUE "\nHarley disconnected\n" RESET); 
        }
    }
}

/*********************************************** 
* 
* @Finalidad: Eliminar un worker de la lista enlazada de workers en el servidor Gotham, 
*             actualizando el número de workers por tipo y reasignando el main worker 
*             si el worker eliminado era el principal. 
* 
* @Parámetros: 
* in/out: server = Puntero a la estructura `GothamServer` que contiene la lista de workers conectados. 
* in: client_socket = Descriptor del socket del worker que se debe eliminar. 
* 
* @Retorno: Ninguno. 
* 
************************************************/
void MC_removeWorkerFromList(GothamServer *server, int client_socket, int *port, char **ip, char **type) {
    WORKER_LINKEDLIST_goToHead(&server->worker_list);

    while (!WORKER_LINKEDLIST_isAtEnd(server->worker_list)) {
        Worker worker = WORKER_LINKEDLIST_get(&server->worker_list);
        int is_media = 1; 

        if (worker.socket_fd == client_socket) {
            if (strcmp(worker.worker_type, "Text") == 0) {
                server->n_enigmas--;
                is_media = 0;
                *port = worker.port;
                *ip = strdup(worker.ip);
                if (!*ip) {
                    IO_printStatic(STDOUT_FILENO, "Error: Could not create connection string\n");
                    return;
                }
                *type = strdup("Enigma");
                if (!*type) {
                    IO_printStatic(STDOUT_FILENO, "Error: Could not create connection string\n");
                    free(*ip);
                    return;
                }
            } else if (strcmp(worker.worker_type, "Media") == 0) {
                server->n_harleys--;
                *port = worker.port;
                *ip = strdup(worker.ip);
                if (!*ip) {
                    IO_printStatic(STDOUT_FILENO, "Error: Could not create connection string\n");
                    return;
                }
                *type = strdup("Harley");
                if (!*type) {
                    IO_printStatic(STDOUT_FILENO, "Error: Could not create connection string\n");
                    free(*ip);
                    return;
                }
            }
            MC_printWorkerDisconnection(worker.worker_type, worker.is_main); 
            WORKER_LINKEDLIST_remove(&server->worker_list);

            if (worker.is_main) {
                MC_assignNewMainWorker(server, is_media ? "Media" : "Text"); 
            }

            break;
        }
        WORKER_LINKEDLIST_next(&server->worker_list);
    }
}


/*********************************************** 
* 
* @Finalidad: Eliminar un fleck de la lista enlazada de flecks en el servidor Gotham, 
*             identificándolo por su socket y mostrando un mensaje de desconexión. 
* 
* @Parámetros: 
* in/out: server = Puntero a la estructura `GothamServer` que contiene la lista de flecks conectados. 
* in: client_socket = Descriptor del socket del fleck que se debe eliminar. 
* 
* @Retorno: Ninguno. 
* 
************************************************/
void MC_removeFleckFromList(GothamServer *server, int client_socket, int *port, char **ip, char **type) {
    FLECK_LINKEDLIST_goToHead(&server->fleck_list);
    while (!FLECK_LINKEDLIST_isAtEnd(server->fleck_list)) {
        FleckElement fleck = FLECK_LINKEDLIST_get(&server->fleck_list);
        if (fleck.socket_fd == client_socket) {
            IO_printFormat(STDOUT_FILENO, BLUE "\nFleck disconnected: %s\n" RESET, fleck.username);
            *port = fleck.port;
            *ip = strdup(fleck.ip);
            if (!*ip) {
                IO_printStatic(STDOUT_FILENO, "Error: Could not create connection string\n");
                return;
            }
            *type = strdup("Fleck");
            if (!*type) {
                IO_printStatic(STDOUT_FILENO, "Error: Could not create connection string\n");
                free(*ip);
                return;
            }
            FLECK_LINKEDLIST_remove(&server->fleck_list);  
            //retyurn del asprint
            break;
        }
        FLECK_LINKEDLIST_next(&server->fleck_list);  
    }
}

/*********************************************** 
* 
* @Finalidad: Eliminar un cliente (fleck o worker) del servidor Gotham, cerrando su socket, 
*             eliminándolo de la lista correspondiente y actualizando el descriptor de archivo máximo. 
* 
* @Parámetros: 
* in/out: server = Puntero a la estructura `GothamServer` que contiene la información de los clientes conectados. 
* in: client_socket = Descriptor del socket del cliente que se debe eliminar. 
* in: client_type = Tipo de cliente (`'f'` para fleck o `'w'` para worker). 
* 
* @Retorno: Ninguno. 
* 
************************************************/
void MC_removeClient(GothamServer *server, int client_socket, char client_type) {
    char *data = NULL;
    int port = 0;
    char *ip = NULL;
    char *type = NULL;

    // Eliminar y cerrar socket del cliente
    MC_removeClientSocket(server, client_socket);

    // Eliminar fleck/worker de la lista respectiva
    if (client_type == 'f') {
        MC_removeFleckFromList(server, client_socket, &port, &ip, &type);
    } else if (client_type == 'w') {
        MC_removeWorkerFromList(server, client_socket, &port, &ip, &type);
    }

     // Escribir el log
    if (asprintf(&data, "%s disconnected: IP:%s:%d", type, ip, port) == -1) {
        IO_printStatic(STDOUT_FILENO, "Error: Creating log\n");
        free(ip);
        free(type);
        return;
    }
    Frame * frame = FRAME_createFrame(0x07, "", 0);
    writeLog(frame, server->fd_log, data);

    // Liberar memoria
    free(frame);
    free(ip);
    free(data);
    free(type);

    // Actualizar el descriptor máximo
    MC_updateMaxFD(server);
}
