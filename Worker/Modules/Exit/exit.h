/*********************************************** 
* 
* @Autores: Alexandre Contreras y Armand López
* @Propósito: Proveer funciones para la gestión y limpieza de recursos en el contexto de workers, 
*             incluyendo la liberación de memoria, la limpieza de archivos de distorsión y 
*             el manejo de memoria compartida.
* 
* @Fecha de creación: 30 de octubre de 2024
* @Última modificación: 4 de enero de 2025
* 
************************************************/

#ifndef _EXIT_WORKER_CUSTOM_H_
#define _EXIT_WORKER_CUSTOM_H_

//Llibreries del sistema
#include <stdlib.h>      // malloc, free
#include <stdio.h>       // perror
#include <string.h>      // strcmp
#include <unistd.h>      // close, STDOUT_FILENO
#include <pthread.h>     // pthread_mutex_destroy, pthread_join, pthread_mutex_lock, pthread_mutex_unlock
#include <sys/ipc.h>     // ftok
#include <sys/shm.h>     // shmget, shmat, shmdt, shmctl

//Llibreries pròpies
#include "../../../Libs/IO/io.h"                          // Per a les funcions d'entrada/sortida
#include "../../../Libs/Semaphore/semaphore_v2.h"                   // Per a les funcions de semàfors
#include "../../../Libs/File/file.h"                      // Per a les funcions de manipulació de fitxers
#include "../../../Libs/Dir/dir.h"                      // Per a les funcions de manipulació de fitxers
#include "../../../Libs/Compress/so_compression.h"

//.h estructuctures
#include "../../typeWorker.h"           // Per a les estructures de configuració de Worker
#include "../../../Libs/Structure/typeDistort.h"       // Per a les estructures de context de distorsió

//Funcions

/*********************************************** 
* 
* @Finalidad: Realizar la limpieza de los recursos asociados al worker principal, 
*             cerrando el socket de escucha y esperando la finalización de los hilos 
*             activos de distorsión. 
* 
* @Parámetros: 
* in/out: server = Puntero a la estructura `WorkerServer` que contiene los recursos a limpiar. 
* 
* @Retorno: Ninguno. 
* 
************************************************/
void EXIT_cleanupMainWorker(WorkerServer* server);

/*********************************************** 
* 
* @Finalidad: Liberar la memoria dinámica asociada a las estructuras `WorkerConfig` y 
*             `WorkerServer`, incluyendo sus campos internos y mutexes. 
* 
* @Parámetros: 
* in/out: worker = Doble puntero a la estructura `WorkerConfig` que se liberará. 
* in/out: server = Doble puntero a la estructura `WorkerServer` que se liberará. 
* 
* @Retorno: Ninguno. 
* 
************************************************/
void EXIT_freeMemory(WorkerConfig** worker, WorkerServer** server);

/*********************************************** 
* 
* @Finalidad: Gestionar los archivos generados durante el proceso de distorsión, moviéndolos 
*             al directorio global en caso de una señal SIGINT o eliminándolos si la 
*             distorsión se completó o fue abortada. 
* 
* @Parámetros: 
* in: context = Estructura `DistortionContext` que contiene la información del archivo a procesar. 
* in: sigint_flag = Bandera que indica si la limpieza fue causada por una señal SIGINT. 
* in: shm_id = Identificador de la memoria compartida asociada al proceso de distorsión. 
* in: sWorkerCountMutex = Puntero al semáforo utilizado para sincronizar el acceso al contador de workers. 
* in: file_type = Tipo de archivo (`'t'` para texto o `'m'` para multimedia). 
* 
* @Retorno: Ninguno. 
* 
************************************************/
void EXIT_cleanupDistortionFiles(DistortionContext distortion_context, int sigint_flag, int shm_id, semaphore* sWorkerCountMutex, char file_type);

/*********************************************** 
* 
* @Finalidad: Gestionar y limpiar la memoria compartida asociada al proceso de distorsión, 
*             desasociándola o eliminándola según el estado del proceso y la señal recibida. 
* 
* @Parámetros: 
* in: distortion_context = Estructura `DistortionContext` que contiene el estado actual del proceso de distorsión. 
* in: shm_id = Identificador de la memoria compartida utilizada para almacenar el progreso de la distorsión. 
* in: exit_distortion = Bandera que indica si el proceso de distorsión fue interrumpido. 
* in: sWorkerCountMutex = Puntero al semáforo utilizado para sincronizar el acceso al contador global de workers. 
* in: file_type = Tipo de archivo procesado (`'t'` para texto o `'m'` para multimedia). 
* 
* @Retorno: Ninguno. 
* 
************************************************/
void EXIT_cleanupSharedMemory(DistortionContext distortion_context, int shm_id, volatile int exit_distortion, semaphore* sWorkerCountMutex, char file_type);

/*********************************************** 
* 
* @Finalidad: Liberar la memoria asociada a una estructura `DistortionContext`, 
*             incluyendo los punteros internos que almacenan información del archivo 
*             y metadatos relacionados con la distorsión. 
* 
* @Parámetros: 
* in/out: context = Puntero a la estructura `DistortionContext` cuya memoria será liberada. 
* 
* @Retorno: Ninguno. 
* 
************************************************/
void EXIT_cleanupDistortionContext(DistortionContext* distortion_context);

#endif // _EXIT_WORKER_CUSTOM_H_