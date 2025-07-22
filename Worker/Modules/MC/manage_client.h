/*********************************************** 
* 
* @Autores: Alexandre Contreras y Armand López
* @Propósito: Proveer funciones para la gestión de clientes y hilos activos en el servidor Worker, 
*             incluyendo la adición y eliminación de clientes y hilos de manera segura.
* 
* @Fecha de creación: 30 de octubre de 2024
* @Última modificación: 4 de enero de 2025
* 
************************************************/

#ifndef _MANAGE_CLIENT_WORKER_H_
#define _MANAGE_CLIENT_WORKER_H_

//Llibreries del sistema
#include <stdlib.h>       // malloc, realloc, free
#include <pthread.h>      // pthread_mutex_lock, pthread_mutex_unlock, pthread_t
#include <unistd.h>       // close

//Llibreries pròpies
#include "../../../Libs/IO/io.h"                          // Per a les funcions d'entrada/sortida

//.h estructures
#include "../../typeWorker.h"           // Per a les estructures de configuració de Worker

//Funcions

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
void MC_addClient(WorkerServer *server, int client_socket);

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
void MC_removeClient(WorkerServer *server, int client_socket);

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
void MC_addActiveThread(WorkerServer *server, pthread_t thread_id);
#endif // _MANAGE_CLIENT_WORKER_H_