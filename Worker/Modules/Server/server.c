/*********************************************
*
* @Autores: Alexandre Contreras, Armand López.
*
* @Finalidad: Implementar las funciones de inicialización y ejecución del servidor del worker.
*
* @Fecha de creación: 30 de octubre de 2024
*
* @Última modificación: 4 de enero de 2025
*
************************************************/

#include "server.h"

/*********************************************** 
* 
* @Finalidad: Inicializar una estructura `WorkerServer`, configurando sus campos, 
*             sockets, listas dinámicas y mutexes necesarios para su funcionamiento. 
* 
* @Parámetros: 
* in/out: server = Puntero a la estructura `WorkerServer` que será inicializada. 
* in: config = Puntero a la estructura `WorkerConfig` que contiene la configuración del worker. 
* 
* @Retorno: 
*           0 = Inicialización exitosa. 
*          -1 = Error durante la inicialización (e.g., fallo al crear el socket o asignar memoria). 
* 
************************************************/
int SRV_initWorkerServer(WorkerServer *server, WorkerConfig *config) {
    //inicialitzem estructures dinàmiques a null per si falla alguna de les següents operacions que al fer el free memory no s'intenti alliberar memòria que no s'ha demanat
    server->active_threads = NULL;
    server->clients = NULL; 

    //inicialitzem mutex's per si falla alguna de les següents operacions que el freeMemory no intenti destruir un mutex no inicialitzat
    pthread_mutex_init(&server->thread_list_mutex, NULL); 
    pthread_mutex_init(&server->clients_mutex, NULL);

    //inicialitzm socket d'escolta de flecks
    server->listen_socket = SOCKET_initListenSocket(config->worker_ip, config->worker_port, MAX_CLIENTS);
    if(server->listen_socket < 0) {
        return -1;
    }
    server->n_clients = 0;

    server->clients = (int*) malloc(sizeof(int));
    if(server->clients == NULL) {
        return -1; 
    }

    server->active_thread_count = 0;
    server->active_threads = (pthread_t*) malloc(sizeof(pthread_t));
    if(server->active_threads == NULL) {
        return -1; 
    }

    return 0; //si tot ha anat bé retornem 0
}

/*********************************************** 
* 
* @Finalidad: Ejecutar el servidor del worker, aceptando conexiones de flecks, 
*             gestionando threads para distorsión y manejando el cierre seguro del servidor. 
* 
* @Parámetros: 
* in: server = Puntero a la estructura `WorkerServer` que contiene la configuración del servidor. 
* in: distortions_folder_path = Ruta a la carpeta donde se procesarán las distorsiones. 
* in: exit_program = Puntero a una bandera `volatile int` que indica si el servidor debe finalizar. 
* in: exit_distortion = Puntero a una bandera `volatile int` que indica si las distorsiones deben interrumpirse. 
* in: sWorkerCountMutex = Puntero al semáforo utilizado para sincronizar el acceso al contador global de workers. 
* in: file_type = Tipo de archivo que procesa el worker (`'t'` para texto o `'m'` para multimedia). 
* in: print_mutex = Puntero al mutex utilizado para sincronizar los mensajes de impresión. 
* 
* @Retorno: Ninguno. 
* 
************************************************/
void SRV_runWorkerServer(WorkerServer* server, char* distortions_folder_path, volatile int* exit_program, volatile int* exit_distortion, semaphore* sWorkerCountMutex, char file_type, pthread_mutex_t* print_mutex) {
    int client_socket;

    while (!*exit_program) {
        // Acceptem connexions de flecks
        client_socket = SOCKET_safe_accept(server->listen_socket);
        if (client_socket < 0) {
            if (errno == EBADF || errno == EINVAL) {
                STRING_printF(print_mutex, STDOUT_FILENO, PURPLE, "\nListen socket closed, no more fleck connections will be accepted\n"); // S'entra en aquest cas quan gotham cau i tanquem el socket d'escolta
                break;
            } else {
                continue; // Timeout del mètode accept (no bloquejant)
            }
        }
        
        STRING_printF(print_mutex, STDOUT_FILENO, GREEN, "\nNew fleck connected\n");

        // Inicialitzem els arguments que passarem al thread de distorsió
        DistortionThreadArgsW* args = DIST_initDistortionArgs(server, distortions_folder_path, exit_distortion, sWorkerCountMutex, file_type, client_socket, print_mutex);
        if(!args) {
            close(client_socket);
            continue;
        }

        // Creem el thread de distorsió
        pthread_t distorsion_thread;
        if (pthread_create(&distorsion_thread, NULL, DIST_handleFileDistortion, (void *)args) != 0) {
            STRING_printF(print_mutex, STDOUT_FILENO, RED, "Failed to create distortion thread\n");
            close(client_socket);
            free(args); 
            continue;
        }

        // En cas que el thread de distorsió s'hagi llançat correctament, afegim el socket de client i el thread llançat a les llistes respectives de l'estructura de servidor 
        MC_addClient(server, client_socket);
        MC_addActiveThread(server, distorsion_thread);            
    }

    STRING_printF(print_mutex, STDOUT_FILENO, YELLOW, "Shutting down worker server...\n");
}

