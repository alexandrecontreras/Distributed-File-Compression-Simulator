/*********************************************** 
* 
* @Autores: Alexandre Contreras y Armand López
* @Propósito: Proveer funciones para la inicialización y gestión del servidor Worker, 
*             incluyendo la aceptación de conexiones, manejo de threads de distorsión y 
*             limpieza de recursos asociados al servidor.
* 
* @Fecha de creación: 30 de octubre de 2024
* @Última modificación: 4 de enero de 2025
* 
************************************************/

#ifndef _WORKER_SERVER_CUSTOM_H_
#define _WORKER_SERVER_CUSTOM_H_

//Llibreries del sistema
#include <stdlib.h>     // malloc, free, exit
#include <string.h>     // strcpy, strlen
#include <unistd.h>     // close, STDOUT_FILENO, write
#include <fcntl.h>      // O_CREAT, O_RDWR
#include <signal.h>     // signal, SIGINT
#include <pthread.h>    // pthread_create, pthread_mutex_init
#include <sys/select.h> // select, fd_set
#include <sys/socket.h> // accept, bind, listen
#include <netinet/in.h> // sockaddr_in, htons
#include <errno.h>      // errno, EBADF

//Llibreries pròpies
#include "../../../Libs/IO/io.h"                          // Per a les funcions d'entrada/sortida
#include "../../../Libs/Socket/socket.h"                  // Per a les funcions de creació de sockets
#include "../../../Libs/Semaphore/semaphore_v2.h"         // Per a les funcions de semàfors
#include "../../../Libs/String/string.h"                  // Per a les funcions de manipulació de strings
#include "../../../Libs/Socket/socket.h"                  // Per a la funció accept no bloquejant

//Moduls de Worker
#include "../Distortion/distortion.h"          // Per a les funcions de distorsió
#include "../MC/manage_client.h"               // Per a les funcions de gestió de clients

//.h estructures
#include "../../typeWorker.h"           // Per a les estructures de configuració de Worker

//Funcions

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
int SRV_initWorkerServer(WorkerServer *server, WorkerConfig *config);

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
void SRV_runWorkerServer(WorkerServer* server, char* distortions_folder_path, volatile int* exit_program, volatile int* exit_distortion, semaphore* sWorkerCountMutex, char file_type, pthread_mutex_t* print_mutex);
#endif // _WORKER_SERVER_CUSTOM_H_