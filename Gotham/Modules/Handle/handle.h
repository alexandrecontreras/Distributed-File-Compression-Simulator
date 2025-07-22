/*********************************************** 
* 
* @Autores: Alexandre Contreras y Armand López
* @Propósito: Definir funciones para manejar conexiones y tramas de los clientes conectados 
*             al servidor Gotham, permitiendo aceptar nuevas conexiones y procesar 
*             las acciones basadas en los datos recibidos. 
* @Fecha de creación: 24 de octubre de 2024
* @Última modificación: 4 de enero de 2025
* 
************************************************/

#ifndef _GOTHAM_HANDLE_H_
#define _GOTHAM_HANDLE_H_

//Constants del sistema
#define _GNU_SOURCE

//Llibreries del sistema
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/select.h>

//Llibreries pròpies
#include "../../../Libs/IO/io.h"                          // Per a les funcions d'entrada/sortida
#include "../../../Libs/Frame/frame.h"                    // Per a les funcions de creació de frames
#include "../../../Libs/String/string.h"                  // Per a les funcions de manipulació de strings
#include "../../../Libs/File/file.h"                      // Per a les funcions de manipulació de fitxers
#include "../../../Libs/LinkedList/fleckLinkedList.h"     // Per a les funcions de la llista enllaçada de Flecks
#include "../../../Libs/LinkedList/workerLinkedList.h"    // Per a les funcions de la llista enllaçada de Workers
#include "../../../Libs/Communication/communication.h"     // Per a les funcions de comunicació

//Moduls de Gotham
#include "../MC/manage_client.h"                        // Per a les funcions de gestió dels clients
#include "../Communication/communication.h"             // Per a les funcions de comunicació

//.h estructures
#include "../../typeGotham.h"

//Funcions
/*********************************************** 
* 
* @Finalidad: Aceptar una nueva conexión entrante desde un cliente (fleck o worker), 
*             agregar su socket al servidor y devolver el descriptor del socket aceptado. 
* 
* @Parámetros: 
* in: listen_socket = Descriptor del socket en modo escucha que recibe la conexión entrante. 
* in/out: server = Puntero a la estructura `GothamServer` que administra los clientes conectados. 
* in: is_fleck = Indicador booleano que especifica si el cliente es un fleck (`true`) o un worker (`false`). 
* 
* @Retorno: 
*           > 0 = Descriptor del socket del cliente aceptado. 
*           -1 = Error al aceptar la conexión. 
* 
************************************************/
int HANDLE_acceptNewConnection(int listen_socket, GothamServer *server, bool is_fleck);

/*********************************************** 
* 
* @Finalidad: Manejar una trama recibida desde un cliente (fleck o worker), procesando 
*             la acción correspondiente en función del tipo de trama. 
* 
* @Parámetros: 
* in/out: server = Puntero a la estructura `GothamServer` que contiene la lista de clientes y workers. 
* in: client_socket = Descriptor del socket del cliente que envió la trama. 
* in: client_type = Tipo de cliente (`'f'` para fleck o `'w'` para worker). 
* in: log_fd = Descriptor del archivo de log donde se registrarán eventos importantes. 
* 
* @Retorno: Ninguno. 
* 
************************************************/
void HANDLE_handleFrame(GothamServer *server, int client_socket, char client_type);
#endif // _GOTHAM_HANDLE_H_