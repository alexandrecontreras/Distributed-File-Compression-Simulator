/*********************************************** 
* 
* @Autores: Alexandre Contreras y Armand López
* @Propósito: Proveer las funciones necesarias para la comunicación entre el worker, Gotham y Flecks, 
*             permitiendo el manejo de conexiones, recepción y envío de metadatos, y gestión de desconexiones. 
* 
* @Fecha de creación: 28 de octubre de 2024
* @Última modificación: 4 de enero de 2025
* 
************************************************/

#ifndef _WORKER_COMMUNICATION_CUSTOM_H_
#define _WORKER_COMMUNICATION_CUSTOM_H_

//Constants del sistema
#define _GNU_SOURCE     // Permet utilitzar asprintf

//Llibreries del sistema
#include <stdlib.h>     // malloc, free, asprintf, atoi
#include <string.h>     // strcmp, strdup, strtok, strlen
#include <unistd.h>     // close, STDOUT_FILENO
#include <stdio.h>      // perror, sprintf
#include <errno.h>      // errno

//Llibreries pròpies
#include "../../../Libs/IO/io.h"                          // Per a les funcions d'entrada/sortida
#include "../../../Libs/Frame/frame.h"                    // Per a les funcions de creació i destrucció de trames
#include "../../../Libs/Communication/communication.h"    // Per a les funcions de comunicació
#include "../../../Libs/String/string.h"                  // Per a les funcions de manipulació de strings

//Moduls de Worker
#include "../Context/context.h"               // Per a les funcions de context

//.h estructures
#include "../../typeWorker.h"           // Per a les estructures de configuració de Worker
#include "../../../Libs/Structure/typeDistort.h"       // Per a les estructures de context de distorsió

#define COMM_ASSIGNED_MAIN_WORKER 0  
#define COMM_GOTHAM_CRASHED       1   
#define COMM_SIGINT_RECEIVED      2   
#define COMM_PENDING              3   


#define UNEXPECTED_ERROR        -1
#define REMOTE_END_DISCONNECTION 0
#define TRANSFER_SUCCESS         1

//Funcions

/*********************************************** 
* 
* @Finalidad: Establecer conexión con el servidor Gotham, enviando una trama de conexión 
*             y verificando la respuesta del servidor. 
* 
* @Parámetros: 
* in: gotham_socket = Descriptor del socket conectado al servidor Gotham. 
* in: config = Puntero a la estructura `WorkerConfig` que contiene la configuración del worker. 
* 
* @Retorno: 
*           0 = Conexión establecida exitosamente. 
*          -1 = Error al enviar la trama de conexión, al recibir la respuesta, o si Gotham 
*               devuelve un estado negativo. 
* 
************************************************/
int COMM_connectToGotham(int gotham_socket, WorkerConfig *config);

/*********************************************** 
* 
* @Finalidad: Recibir y procesar los metadatos de un archivo enviados por un fleck, 
*             validarlos, inicializar el contexto de distorsión, y gestionar el progreso 
*             asociado a la distorsión. 
* 
* @Parámetros: 
* in: fleck_socket = Descriptor del socket del fleck desde el cual se recibirán los metadatos. 
* in/out: distortion_context = Puntero a la estructura `DistortionContext` donde se almacenarán 
*                              los metadatos recibidos y procesados. 
* in: distortions_folder_path = Ruta a la carpeta donde se almacenarán los archivos distorsionados. 
* out: shm_id = Puntero al identificador de memoria compartida para gestionar el progreso. 
* 
* @Retorno: 
*           1 = Los metadatos fueron recibidos y procesados correctamente. 
*           0 = Error en la recepción, validación de atributos, o inicialización del contexto. 
* 
************************************************/
int COMM_retrieveFileMetadata(int fleck_socket, DistortionContext* distortion_context, char* distortions_folder_path, int* shm_id);

/*********************************************** 
* 
* @Finalidad: Enviar los metadatos de un archivo distorsionado a un fleck mediante un socket. 
*             Los metadatos incluyen el tamaño del archivo y su hash MD5. 
* 
* @Parámetros: 
* in: context = Estructura `DistortionContext` que contiene los metadatos del archivo distorsionado. 
* in: fleck_socket = Descriptor del socket del fleck al que se enviarán los metadatos. 
* in: print_mutex = Puntero al mutex utilizado para sincronizar los mensajes de impresión. 
* 
* @Retorno: 
*           TRANSFER_SUCCESS = Los metadatos fueron enviados exitosamente. 
*           UNEXPECTED_ERROR = Error al enviar los metadatos. 
* 
************************************************/
int COMM_sendFleckFileMetadata(DistortionContext context, int fleck_socket, pthread_mutex_t* print_mutex);

/*********************************************** 
* 
* @Finalidad: Esperar la asignación como worker principal mediante la recepción de tramas 
*             de Gotham, gestionando posibles interrupciones por SIGINT o desconexiones. 
* 
* @Parámetros: 
* in: gotham_socket = Descriptor del socket conectado a Gotham desde el cual se recibirán las tramas. 
* in: exit_program = Puntero a una bandera `volatile int` que indica si el programa debe finalizar. 
* 
* @Retorno: 
*           COMM_ASSIGNED_MAIN_WORKER = Se recibió una trama que asigna el rol de worker principal. 
*           COMM_GOTHAM_CRASHED = Gotham se desconectó o el socket fue cerrado. 
*           COMM_SIGINT_RECEIVED = El proceso fue interrumpido por una señal SIGINT. 
*           COMM_PENDING = La asignación sigue pendiente y el bucle terminó sin una trama válida. 
* 
************************************************/
int COMM_waitForMainWorkerAssignment(int gotham_socket, volatile int* exit_program);

/*********************************************** 
* 
* @Finalidad: Manejar la desconexión de un fleck, recibiendo y procesando la trama 
*             de desconexión enviada por el fleck. 
* 
* @Parámetros: 
* in: fleck_socket = Descriptor del socket del fleck que se desconecta. 
* in: print_mutex = Mutex para sincronizar los mensajes de impresión. 
* 
* @Retorno: Ninguno. 
* 
************************************************/
void COMM_handleFleckDisconnection(int fleck_socket, pthread_mutex_t* print_mutex);
#endif // _WORKER_COMMUNICATION_CUSTOM_H_