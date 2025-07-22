/*********************************************** 
* 
* @Autores: Alexandre Contreras y Armand López
* @Propósito: Definir constantes, estructuras y funciones relacionadas con la comunicación 
*             entre el proceso Fleck y el servidor Gotham o los workers. Proporciona funciones 
*             para gestionar conexiones, intercambiar metadatos y manejar desconexiones. 
* @Fecha de creación: 15 de octubre de 2024
* @Última modificación: 4 de enero de 2025
* 
************************************************/

#ifndef _FLECK_COMMUNICATION_CUSTOM_H_
#define _FLECK_COMMUNICATION_CUSTOM_H_

//Constant del sistema
#define _GNU_SOURCE

//Defines pròpies
#define CONNECTED_TO_SAME_WORKER 2
#define CONNECTED_TO_NEW_WORKER  1
#define FAILED_TO_CONNECT        0

#define RECONNECTION             1
#define CONNECTION               0

#define UNEXPECTED_ERROR        -1
#define REMOTE_END_DISCONNECTION 0
#define TRANSFER_SUCCESS         1
#define INTERRUPTED_BY_SIGINT    2

#define COMM_WORKER              0
#define COMM_GOTHAM              1

//Llibreries del sistema
#include <stdlib.h>    // malloc, free, asprintf
#include <stdio.h>     // asprintf
#include <string.h>    // strcmp, strlen
#include <unistd.h>    // STDOUT_FILENO
#include <arpa/inet.h> // inet_ntop, ntohs, struct sockaddr_in
#include <sys/socket.h> // getsocknameç
#include <pthread.h>


//Llibreries pròpies
#include "../../../Libs/IO/io.h"                  // Per a les funcions d'entrada/sortida
#include "../../../Libs/Frame/frame.h"           // Per a les funcions de manipulació de frames
#include "../../../Libs/Socket/socket.h"         // Per a les funcions de connexió per sockets
#include "../../../Libs/String/string.h"         // Per a les funcions de manipulació de strings

//.h estructures
#include "../../typeFleck.h"                          // Per a les estructures de configuracio de Fleck

//Funcions
/*********************************************** 
* 
* @Finalidad: Establecer conexión entre el proceso Fleck y el servidor Gotham, enviando 
*             una trama de conexión y verificando la respuesta del servidor. 
* 
* @Parámetros: 
* in: gotham_socket = Descriptor del socket conectado al servidor Gotham. 
* in: config = Puntero a la configuración del proceso Fleck que contiene el nombre de usuario. 
* 
* @Retorno: Retorna un entero que indica el estado de la operación:
*           0 = Conexión establecida correctamente con Gotham. 
*          -1 = Error durante el envío de la trama de conexión, recepción de respuesta, 
*               o verificación de la respuesta del servidor. 
* 
************************************************/
int COMM_connectToGotham(int gotham_socket, FleckConfig *config);

/*********************************************** 
* 
* @Finalidad: Solicitar al servidor Gotham la dirección IP y puerto de un worker según 
*             el tipo de tarea solicitada, procesar la respuesta, actualizar la información 
*             del worker principal y establecer la conexión con el worker. 
* 
* @Parámetros: 
* in: filename = Nombre del archivo que se va a procesar. 
* in: type = Tipo de distorsión solicitada (e.g., "media", "text"). 
* in/out: main_worker = Estructura `MainWorker` que contiene la información del worker principal 
*                       actual y su socket. Se actualiza si el worker cambia. 
* in: gotham_socket = Descriptor del socket conectado al servidor Gotham. 
* in: reconnecting_flag = Indicador de si esta solicitud es parte de un proceso de reconexión (1) 
*                         o una nueva conexión (0). 
* in: print_mutex = Mutex para garantizar la exclusión mutua al imprimir mensajes de estado o error. 
* 
* @Retorno: Retorna un entero que indica el estado de la operación:
*           CONNECTED_TO_NEW_WORKER = Conexión exitosa con un nuevo worker principal. 
*           CONNECTED_TO_SAME_WORKER = Conexión con el mismo worker al que estaba conectado 
*                                      previamente (durante una reconexión). 
*           FAILED_TO_CONNECT = Error al establecer la conexión con el worker. 
*           0 = Error al procesar la respuesta de Gotham o al solicitar el worker. 
* 
************************************************/
int COMM_requestWorkerAndEstablishConnection(char* filename, char* type, MainWorker* main_worker, int gotham_socket, int reconnecting_flag, pthread_mutex_t *print_mutex);

/*********************************************** 
* 
* @Finalidad: Enviar los metadatos de un archivo al worker, incluyendo información sobre 
*             el usuario, el nombre del archivo, el tamaño, el hash MD5 y el factor de distorsión. 
*             Procesa la respuesta del worker para verificar si acepta la solicitud. 
* 
* @Parámetros: 
* in: worker_socket = Descriptor del socket conectado al worker. 
* in: username = Nombre del usuario que solicita la distorsión. 
* in: filename = Nombre del archivo que se va a distorsionar. 
* in: file_size = Tamaño del archivo en bytes. 
* in: md5sum = Hash MD5 del archivo, utilizado para validar la integridad. 
* in: factor = Factor de distorsión solicitado. 
* in: print_mutex = Mutex para garantizar la exclusión mutua al imprimir mensajes de estado o error. 
* 
* @Retorno: Retorna un entero que indica el estado de la operación:
*           0 = El worker aceptó la solicitud y está listo para procesar el archivo. 
*          -1 = Error al enviar la solicitud o rechazo por parte del worker. 
* 
************************************************/
int COMM_sendFileMetadata(int worker_socket, const char* username, const char* filename, int file_size, const char* md5sum, const int factor, pthread_mutex_t *print_mutex);

/*********************************************** 
* 
* @Finalidad: Intentar reconectar con un nuevo worker principal tras una desconexión. 
*             Solicita al servidor Gotham los detalles del nuevo worker y establece 
*             la conexión si es posible. 
* 
* @Parámetros: 
* in: filename = Nombre del archivo que se procesará después de la reconexión. 
* in: worker_type = Tipo de worker solicitado (e.g., "media", "text"). 
* in/out: main_worker = Estructura `MainWorker` que contiene la información del worker principal 
*                       y su socket. Se actualiza si la reconexión es exitosa. 
* in: gotham_socket = Descriptor del socket conectado al servidor Gotham. 
* in: print_mutex = Mutex para garantizar la exclusión mutua al imprimir mensajes de estado o error. 
* 
* @Retorno: Retorna un entero que indica el estado de la reconexión:
*           1 = Reconexión exitosa con un nuevo worker principal. 
*           0 = Error al reconectar o reconexión con el mismo worker al que estaba conectado previamente. 
* 
************************************************/
int COMM_reconnectToWorker(char* filename, char* worker_type, MainWorker* main_worker, int gotham_socket, pthread_mutex_t *print_mutex); 

/*********************************************** 
* 
* @Finalidad: Recibir y procesar los metadatos del archivo distorsionado enviados por el worker. 
*             Configura la estructura de contexto de distorsión (`DistortionContext`) 
*             con los datos recibidos. 
* 
* @Parámetros: 
* in: worker_socket = Descriptor del socket conectado al worker. 
* out: distorted_file = Puntero a la estructura `DistortionContext` donde se almacenarán 
*                       los metadatos del archivo distorsionado. 
* in: print_mutex = Mutex para garantizar la exclusión mutua al imprimir mensajes de estado o error. 
* 
* @Retorno: Retorna un entero que indica el estado de la operación:
*           TRANSFER_SUCCESS = Metadatos recibidos y procesados con éxito. 
*           REMOTE_END_DISCONNECTION = El worker se desconectó inesperadamente. 
*           UNEXPECTED_ERROR = Error en la recepción o deserialización de la trama, 
*                              o tipo de trama incorrecto. 
* 
************************************************/
int COMM_retrieveFileMetadata(int worker_socket, DistortionContext* distorted_file, pthread_mutex_t *print_mutex);

/*********************************************** 
* 
* @Finalidad: Enviar una notificación a worker/gotham para desconectar al usuario especificado. 
* 
* @Parámetros: 
* in: server_socket = Descriptor del socket conectado al worker/gotham. 
* in: username = Nombre del usuario que se está desconectando.
* in: type = Entero que indica si la desconexión se realiza respecto a gotham o un worker. 
* 
* @Retorno: No retorna ningún valor. 
* 
************************************************/
void COMM_disconnectFromServer(int server_socket, char* username, int type);
#endif // _FLECK_COMMUNICATION_CUSTOM_H_