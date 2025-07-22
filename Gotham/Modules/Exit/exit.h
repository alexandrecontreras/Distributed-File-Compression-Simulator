/*********************************************** 
* 
* @Autores: Alexandre Contreras y Armand López
* @Propósito: Proveer funciones para la limpieza y liberación de recursos asociados 
*             al servidor Gotham, incluyendo configuraciones, listas enlazadas y conexiones activas. 
* @Fecha de creación: 23 de octubre de 2024
* @Última modificación: 4 de enero de 2025
* 
************************************************/

#ifndef _EXIT_GOTHAM_CUSTOM_H_
#define _EXIT_GOTHAM_CUSTOM_H_

//LLibreries del sistema
#include <stdlib.h>     // Para free
#include <unistd.h>     // Para close
#include <sys/select.h> // Para FD_CLR y fd_set

//Llibreries pròpies
#include "../../../Libs/LinkedList/fleckLinkedList.h" // Per a les llistes enllaçades de Fleck
#include "../../../Libs/LinkedList/workerLinkedList.h" // Per a les llistes enllaçades de Worker

//.h estructures
#include "../../typeGotham.h"     // Per a les estructures de configuració de Gotham

//Funcions

/*********************************************** 
* 
* @Finalidad: Liberar la memoria dinámica asociada a las configuraciones de Gotham y 
*             las estructuras del servidor Gotham, incluyendo listas enlazadas y clientes. 
* 
* @Parámetros: 
* in/out: gotham = Puntero a la estructura `GothamConfig` cuya memoria será liberada. 
* in/out: server = Puntero a la estructura `GothamServer` cuya memoria, listas y clientes 
*                  serán liberados. 
* 
* @Retorno: Ninguno. 
* 
************************************************/
void EXIT_freeMemory(GothamConfig* gotham, GothamServer* server);

/*********************************************** 
* 
* @Finalidad: Cerrar todas las conexiones activas en el servidor Gotham, incluyendo 
*             sockets de clientes, flecks y workers, y limpiar los descriptores de 
*             archivo correspondientes. 
* 
* @Parámetros: 
* in/out: server = Puntero a la estructura `GothamServer` que contiene los sockets y 
*                  listas de descriptores activos que se cerrarán y limpiarán. 
* 
* @Retorno: Ninguno. 
* 
************************************************/
void EXIT_closeAllConnections(GothamServer *server);

#endif // _EXIT_GOTHAM_CUSTOM_H_