/*********************************************** 
* 
* @Autores: Alexandre Contreras y Armand López
* @Propósito: Proveer funciones para la monitorización de conexiones y manejo de desconexiones 
*             inesperadas en sistemas distribuidos.
* @Fecha de creación: 10 de octubre de 2024
* @Última modificación: 4 de enero de 2025
* 
************************************************/

#ifndef _MONITOR_CUSTOM_H_
#define _MONITOR_CUSTOM_H_

//Libreries del sistema
#include <stdlib.h>        // malloc(), free()
#include <string.h>        // Para manipulación de strings
#include <unistd.h>        // read(), close()
#include <sys/select.h>    // select(), fd_set
#include <sys/types.h>     // Tipos de datos para sockets
#include <sys/socket.h>    // recv(), MSG_PEEK
#include <netinet/in.h>    // sockaddr_in
#include <errno.h>         // errno
#include <pthread.h>       // pthread y manejo de threads

//Libreries pròpies
#include "../IO/io.h"      // Para IO_printStatic()

//.h de les estructures
#include "../Structure/typeMonitor.h"

//Funcions

/*********************************************** 
* 
* @Finalidad: Monitorear la conexión con Gotham, detectando desconexiones o problemas 
*             en el socket y actualizando las banderas de estado en consecuencia. 
* 
* @Parámetros: 
* in: args = Puntero a la estructura `MonitoringThreadArgs` que contiene los argumentos necesarios 
*            para monitorear la conexión (e.g., socket, banderas de estado). 
* 
* @Retorno: 
*           NULL = Finaliza el hilo de monitoreo tras detectar una desconexión o error crítico. 
* 
************************************************/
void* MONITOR_connectionMonitor(void* args);

/*********************************************** 
* 
* @Finalidad: Inicializar y asignar memoria para una estructura `MonitoringThreadArgs`, 
*             configurando los argumentos necesarios para el monitoreo de la conexión. 
* 
* @Parámetros: 
* in: gotham_socket = Descriptor del socket de Gotham que será monitoreado. 
* in: exit_program = Puntero a una bandera `volatile int` que indica si el programa debe finalizar. 
* in: gotham_alive = Puntero a una bandera que indica si la conexión con Gotham está activa. 
* 
* @Retorno: 
*           Puntero a la estructura `MonitoringThreadArgs` inicializada. 
*           Retorna NULL si ocurre un error al asignar memoria. 
* 
************************************************/
MonitoringThreadArgs* MONITOR_initMonitoringArgs(int gotham_socket, volatile int* exit_program, int* gotham_alive);

#endif // _MONITOR_CUSTOM_H_