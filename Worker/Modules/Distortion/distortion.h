/*********************************************** 
* 
* @Autores: Alexandre Contreras y Armand López
* @Propósito: Proveer funciones para inicializar y manejar procesos de distorsión 
*             en workers, incluyendo la configuración de argumentos para hilos 
*             y la ejecución completa del proceso de distorsión de archivos. 
* 
* @Fecha de creación: 27 de octubre de 2024
* @Última modificación: 4 de enero de 2025
* 
************************************************/

#ifndef _DISTORTION_WORKER_CUSTOM_H_
#define _DISTORTION_WORKER_CUSTOM_H_

//Constants del sistema
#define _GNU_SOURCE

//Llibreries del sistema
#include <stdlib.h>       // malloc, free, asprintf
#include <stdio.h>        // perror, sprintf
#include <unistd.h>       // close, fork, dup2, execlp, STDOUT_FILENO
#include <string.h>       // strcmp, strdup, strtok
#include <fcntl.h>        // open, O_WRONLY, O_TRUNC
#include <sys/types.h>    // pid_t
#include <sys/wait.h>     // wait

//Llibreries pròpies
#include "../../../Libs/IO/io.h"                          // Per a les funcions d'entrada/sortida
#include "../../../Libs/Semaphore/semaphore_v2.h"                   // Per a les funcions de semàfors
#include "../../../Libs/Communication/communication.h"                      // Per a les funcions de gestió de fitxers
#include "../../../Libs/File/file.h"                      // Per a les funcions de gestió de fitxers
#include "../../../Libs/Compress/so_compression.h"        // Per a les funcions de compressió
#include "../../../Libs/String/string.h"                  // Per a les funcions de manipulació de strings

//Moduls de Worker
#include "../Communication/communication.h"    // Per a les funcions de comunicació amb altres mòduls
#include "../Exit/exit.h"                      // Per a les funcions de sortida del programa
#include "../MC/manage_client.h"               // Per a les funcions de gestió de clients

//.h estructures
#include "../../typeWorker.h"           // Per a les estructures de configuració de Worker
#include "../../../Libs/Structure/typeDistort.h"

#define DISTORTION_SUCCESSFUL 1
#define DISTORTION_FAILED     0

//Funcions

/*********************************************** 
* 
* @Finalidad: Inicializar y asignar memoria para una estructura `DistortionThreadArgsW`, 
*             configurando los argumentos necesarios para manejar un hilo de distorsión. 
* 
* @Parámetros: 
* in: server = Puntero a la estructura `WorkerServer` asociada al servidor de workers. 
* in: distortions_folder_path = Ruta a la carpeta de distorsiones donde se procesará el archivo. 
* in: exit_distortion = Puntero a una bandera `volatile int` que indica si se debe interrumpir la distorsión. 
* in: sWorkerCountMutex = Puntero al semáforo que actúa como mutex para el contador global de workers. 
* in: file_type = Tipo de archivo que se procesará (e.g., `'t'` para texto o `'m'` para multimedia). 
* in: client_socket = Descriptor del socket del cliente asociado a la distorsión. 
* in: print_mutex = Puntero al mutex utilizado para sincronizar los mensajes de impresión. 
* 
* @Retorno: 
*           Puntero a la estructura `DistortionThreadArgsW` inicializada. 
*           Retorna NULL si ocurre un error al asignar memoria. 
* 
************************************************/
DistortionThreadArgsW* DIST_initDistortionArgs(WorkerServer* server, char* distortions_folder_path, volatile int* exit_distortion, semaphore* sWorkerCountMutex, char file_type, int client_socket, pthread_mutex_t* print_mutex);

/*********************************************** 
* 
* @Finalidad: Manejar el proceso completo de distorsión de un archivo en un hilo dedicado, 
*             gestionando las distintas etapas como recepción, verificación de integridad, 
*             distorsión, y envío del archivo distorsionado. 
* 
* @Parámetros: 
* in: args = Puntero a la estructura `DistortionThreadArgsW` que contiene los argumentos 
*            necesarios para gestionar el hilo de distorsión. 
* 
* @Retorno: 
*           NULL = El hilo finaliza su ejecución tras completar el proceso de distorsión 
*                  o ante un error crítico. 
* 
************************************************/
void* DIST_handleFileDistortion(void* args);
#endif // _DISTORTION_WORKER_CUSTOM_H_