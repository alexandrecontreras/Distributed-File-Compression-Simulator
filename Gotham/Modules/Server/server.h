#ifndef _GOTHAM_SERVER_CUSTOM_H_
#define _GOTHAM_SERVER_CUSTOM_H_

//Llibreries del sistema
#include <stdlib.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>

//LLibreries pròpies
#include "../../../Libs/IO/io.h"                          // Per a les funcions d'entrada/sortida
#include "../../../Libs/Socket/socket.h"                  // Per a les funcions de creació de sockets
#include "../../../Libs/LinkedList/fleckLinkedList.h"     // Per a les funcions de la llista enllaçada de Flecks
#include "../../../Libs/LinkedList/workerLinkedList.h"    // Per a les funcions de la llista enllaçada de Workers

//Moduls de Gotham
#include "../Exit/exit.h"                      // Per a les funcions de sortida del programa
#include "../Handle/handle.h"                  // Per a les funcions de gestió de les connexions

//.h estructures
#include "../../typeGotham.h"     // Per a les estructures de configuració de Gotham

//Funcions

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
int SRV_initGothamServer(GothamServer *server, GothamConfig *config);

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
void SRV_runGothamServer(GothamServer *server, volatile int* exit_program);
#endif // _GOTHAM_SERVER_CUSTOM_H_