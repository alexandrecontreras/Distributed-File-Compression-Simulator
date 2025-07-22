/*********************************************** 
* 
* @Autores: Alexandre Contreras y Armand López
* @Propósito: Proveer funciones para la inicialización, manejo y cierre de sockets en sistemas distribuidos.
* @Fecha de creación: 15 de octubre de 2024
* @Última modificación: 4 de enero de 2025
* 
************************************************/

#ifndef _SOCKET_CUSTOM_H_
#define _SOCKET_CUSTOM_H_

//Libreries del sistema
#include <unistd.h>       // close
#include <string.h>       // memset
#include <sys/socket.h>   // socket, bind, listen, connect
#include <netinet/in.h>   // struct sockaddr_in, htons
#include <arpa/inet.h>    // inet_addr, inet_pton

//Libreries pròpies
#include "../IO/io.h"

//Funcions

/*********************************************** 
* 
* @Finalidad: Inicializar un socket en modo escucha, enlazándolo a una dirección IP y un puerto específicos, 
*             y configurarlo para aceptar conexiones entrantes. 
* 
* @Parámetros: 
* in: ip = Dirección IP donde el socket escuchará las conexiones. 
* in: port = Puerto donde el socket escuchará las conexiones. 
* in: max_clients = Número máximo de clientes en cola de espera. 
* 
* @Retorno: 
*           Descriptor del socket en modo escucha si la operación es exitosa. 
*          -1 = Error al crear, enlazar o configurar el socket en modo escucha. 
* 
************************************************/
int SOCKET_initListenSocket(const char* ip, int port, int max_clients);

/*********************************************** 
* 
* @Finalidad: Crear e inicializar un socket cliente para conectarse a un servidor 
*             en una dirección IP y puerto específicos. 
* 
* @Parámetros: 
* in: ip = Dirección IP del servidor al que se conectará el cliente. 
* in: port = Puerto del servidor al que se conectará el cliente. 
* 
* @Retorno: 
*           Descriptor del socket del cliente si la conexión es exitosa. 
*          -1 = Error al crear, configurar o establecer la conexión con el servidor. 
* 
************************************************/
int SOCKET_initClientSocket(const char* ip, int port);

/*********************************************** 
* 
* @Finalidad: Cerrar un socket abierto y establecer su descriptor a `-1` para indicar 
*             que ya no está activo. 
* 
* @Parámetros: 
* in/out: socket = Puntero al descriptor del socket que se cerrará. 
* 
* @Retorno: Ninguno. 
* 
************************************************/
void SOCKET_closeSocket(int* socket); 

/*********************************************** 
* 
* @Finalidad: Aceptar una conexión de cliente de forma segura en un socket de escucha, 
*             utilizando la función `select` para manejar posibles bloqueos y timeouts. 
* 
* @Parámetros: 
* in: listen_socket = Descriptor del socket en modo escucha donde se aceptará la conexión. 
* 
* @Retorno: 
*           > 0 = Descriptor del socket del cliente aceptado. 
*          -1 = Error al aceptar la conexión, ya sea por timeout o fallo en `select`. 
* 
************************************************/
int SOCKET_safe_accept(int listen_socket);

#endif // _SOCKET_CUSTOM_H_