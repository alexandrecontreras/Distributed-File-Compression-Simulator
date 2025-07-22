/*********************************************** 
* 
* @Autores: Alexandre Contreras y Armand López
* @Propósito: Proveer funciones para la comunicación entre los procesos (Fleck, Workers y Gotham), 
*             incluyendo el envío y recepción de archivos, verificación de integridad mediante MD5, 
*             y gestión de respuestas de conexión. 
* @Fecha de creación: 28 de octubre de 2024
* @Última modificación: 4 de enero de 2025
* 
************************************************/

#ifndef _COMMUNICATION_CUSTOM_H_
#define _COMMUNICATION_CUSTOM_H_

// Constants del sistema
#define _GNU_SOURCE     // Permet utilitzar asprintf

//Llibreries del sistema
#include <fcntl.h>    // Para open, O_RDONLY, O_WRONLY, etc.
#include <unistd.h>   // Para read, write, close, lseek, usleep
#include <stdlib.h>   // Para malloc, free, atoi
#include <string.h>   // Para strcmp, memcpy
#include <errno.h>    // Para manejar errores con errno
#include <stdint.h>   // Para tipos como uint8_t

//Llibreries pròpies
#include "../IO/io.h"
#include "../Frame/frame.h"
#include "../File/file.h"	
#include "../String/string.h"

#define FLECK  1
#define WORKER 2

#define UNEXPECTED_ERROR        -1
#define REMOTE_END_DISCONNECTION 0
#define TRANSFER_SUCCESS         1
#define INTERRUPTED_BY_SIGINT    2

//Funcions

/*********************************************** 
* 
* @Finalidad: Enviar un archivo al worker o fleck en paquetes, utilizando un socket especificado. 
*             Maneja la confirmación (ACK) después de cada paquete y permite la reanudación 
*             en caso de interrupción. 
* 
* @Parámetros: 
* in: file_path = Ruta completa del archivo a enviar. 
* in: filename = Nombre del archivo que se está enviando. 
* in: n_packets = Número total de paquetes en que está dividido el archivo. 
* in/out: n_processed_packets = Puntero al número de paquetes procesados hasta el momento. 
* in: worker_socket = Descriptor del socket utilizado para enviar los paquetes. 
* in: exit_distortion = Bandera que indica si se debe interrumpir el proceso de envío. 
* in: process = Indica si se está comunicando con un worker o un fleck (e.g., `FLECK`). 
* in: print_mutex = Mutex para sincronizar los mensajes de impresión. 
* 
* @Retorno: 
*           TRANSFER_SUCCESS = El archivo fue enviado con éxito. 
*           REMOTE_END_DISCONNECTION = El extremo remoto (worker o fleck) se desconectó. 
*           UNEXPECTED_ERROR = Ocurrió un error durante la lectura o envío del archivo. 
*           INTERRUPTED_BY_SIGINT = El envío fue interrumpido por una señal SIGINT. 
* 
************************************************/
int COMM_sendFile(char* file_path, char* filename, int n_packets, int* n_processed_packets, int worker_socket, volatile int* exit_distortion, int process, pthread_mutex_t *print_mutex);

/*********************************************** 
* 
* @Finalidad: Recibir un archivo desde un worker o fleck en paquetes a través de un socket, 
*             confirmando la recepción de cada paquete y escribiendo los datos en un archivo local. 
* 
* @Parámetros: 
* in: file_path = Ruta completa del archivo donde se escribirán los datos recibidos. 
* in: filename = Nombre del archivo que se está recibiendo. 
* in: n_packets = Número total de paquetes esperados. 
* in/out: n_processed_packets = Puntero al número de paquetes procesados hasta el momento. 
* in: worker_socket = Descriptor del socket utilizado para recibir los paquetes. 
* in: exit_distortion = Bandera que indica si se debe interrumpir el proceso de recepción. 
* in: process = Indica si se está comunicando con un worker o un fleck (e.g., `FLECK`). 
* in: print_mutex = Mutex para sincronizar los mensajes de impresión. 
* 
* @Retorno: 
*           TRANSFER_SUCCESS = El archivo fue recibido con éxito. 
*           REMOTE_END_DISCONNECTION = El extremo remoto (worker o fleck) se desconectó durante la transmisión. 
*           UNEXPECTED_ERROR = Ocurrió un error durante la recepción o escritura del archivo. 
*           INTERRUPTED_BY_SIGINT = La recepción fue interrumpida por una señal SIGINT. 
* 
************************************************/
int COMM_receiveFile(char* file_path, char* filename, int n_packets, int* n_processed_packets, int worker_socket, volatile int* exit_distortion, int process, pthread_mutex_t *print_mutex);

/*********************************************** 
* 
* @Finalidad: Verificar la integridad de un archivo mediante la comparación de su hash MD5 
*             con el esperado, y enviar el resultado al worker a través de un socket. 
* 
* @Parámetros: 
* in: file_path = Ruta completa del archivo cuya integridad se verificará. 
* in: md5sum = Hash MD5 esperado del archivo. 
* in: worker_socket = Descriptor del socket utilizado para enviar el resultado de la verificación. 
* in: print_mutex = Mutex para sincronizar los mensajes de impresión. 
* 
* @Retorno: 
*           TRANSFER_SUCCESS = La integridad del archivo fue verificada exitosamente (MD5 coincide). 
*           UNEXPECTED_ERROR = Error en la verificación o el MD5 no coincide. 
* 
************************************************/
int COMM_verifyFileIntegrity(char* file_path, char* md5sum, int worker_socket, pthread_mutex_t *print_mutex);

/*********************************************** 
* 
* @Finalidad: Recibir y procesar la verificación MD5 desde un worker o fleck a través de un socket, 
*             evaluando si el archivo fue reensamblado correctamente. 
* 
* @Parámetros: 
* in: worker_socket = Descriptor del socket utilizado para recibir la verificación MD5. 
* in: process = Indica si la comunicación es con un worker o un fleck (e.g., `FLECK`). 
* in: print_mutex = Mutex para sincronizar los mensajes de impresión. 
* 
* @Retorno: 
*           TRANSFER_SUCCESS = El archivo fue reensamblado correctamente (MD5 coincide). 
*           UNEXPECTED_ERROR = Error en la verificación o el reensamblado falló. 
*           REMOTE_END_DISCONNECTION = El extremo remoto (worker o fleck) se desconectó durante la transmisión. 
* 
************************************************/
int COMM_retrieveMD5Check(int worker_socket, int process, pthread_mutex_t *print_mutex);

/*********************************************** 
* 
* @Finalidad: Crear y enviar una respuesta de conexión a un cliente (fleck o worker) a través de un socket. 
*             La respuesta puede indicar éxito o error, dependiendo de la validez de la solicitud. 
* 
* @Parámetros: 
* in: client_socket = Descriptor del socket del cliente al que se enviará la respuesta. 
* in: string_err = Cadena que contiene el mensaje de error en caso de respuesta no válida. 
*                  Si es válida, esta cadena puede ser NULL. 
* in: is_valid = Indicador de validez de la respuesta (1 para válida, 0 para no válida). 
* in: type = Tipo de cliente (`0x01` para fleck o `0x02` para worker). 
* 
* @Retorno: Ninguno. 
* 
************************************************/
void COMM_sendConnectionResponse(int client_socket, char* string_err, int is_valid, int type);  //Usada tant per Gotham com els Workers

#endif // _COMMUNICATION_CUSTOM_H_