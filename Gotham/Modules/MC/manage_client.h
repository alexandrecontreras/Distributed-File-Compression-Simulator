/*********************************************** 
* 
* @Autores: Alexandre Contreras y Armand López
* @Propósito: Proveer funciones para gestionar clientes (flecks y workers) conectados 
*             al servidor Gotham, incluyendo su adición, eliminación y mantenimiento de 
*             las listas enlazadas y descriptores de archivo. 
* @Fecha de creación: 25 de octubre de 2024
* @Última modificación: 4 de enero de 2025
* 
************************************************/

#ifndef _MANAGE_CLIENT_GOTHAM_H_
#define _MANAGE_CLIENT_GOTHAM_H_

//Constants del sistema
#define _GNU_SOURCE

//llibreries del sistema
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/select.h>

//llibreries pròpies
#include "../../../Libs/IO/io.h"                      // Per a les funcions d'entrada/sortida
#include "../../../Libs/LinkedList/fleckLinkedList.h" // Per a les funcions de la llista enllaçada de Flecks
#include "../../../Libs/LinkedList/workerLinkedList.h"// Per a les funcions de la llista enllaçada de Workers
#include "../../../Libs/String/string.h"              // Per a les funcions de manipulació de strings
#include "../../../Libs/Frame/frame.h"              // Per a les funcions de manipulació de trames

//Moduls de Gotham
#include "../Communication/communication.h"           // Per a les funcions de comunicació

//.h estructures
#include "../../typeGotham.h"

//Funcions

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
void MC_addClient(GothamServer *server, int client_socket, char client_type);

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
void MC_removeClient(GothamServer *server, int client_socket, char client_type);

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
void MC_addFleckToServer(GothamServer *server, int client_socket, char* username, char* ip_address, char* port_str);

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
int MC_addWorkerToServer(GothamServer *server, int client_socket, char* worker_type, char* ip_address, char* port_str);

#endif // _MANAGE_CLIENT_GOTHAM_H_