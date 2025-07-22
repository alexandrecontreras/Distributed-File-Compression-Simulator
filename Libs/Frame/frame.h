/*********************************************** 
* 
* @Autores: Alexandre Contreras y Armand López
* @Propósito: Proveer funciones para la creación, manipulación, envío y recepción de 
*             tramas (`Frame`) en un sistema de comunicación basado en sockets. 
*             Incluye funcionalidades para gestionar logs con información de timestamp. 
* @Fecha de creación: 14 de octubre de 2024
* @Última modificación: 4 de enero de 2025
* 
************************************************/

#ifndef _FRAME_CUSTOM_H_
#define _FRAME_CUSTOM_H_

//Constants del sistema
#define _GNU_SOURCE

//Libreries del sistema
#include <stdio.h> // Necesario para perror()
#include <stdlib.h>    // malloc, free
#include <string.h>    // memset, memcpy
#include <unistd.h>       // read()
#include <stdint.h>    // uint8_t, uint16_t, uint32_t
#include <time.h>      // time
#include <errno.h>        // errno, códigos de error como ECONNRESET, EBADF

//Constants
#define FRAME_SIZE 256
#define DATA_SIZE (FRAME_SIZE - 9) 

//Tipus propis
typedef struct {
    uint8_t type;             // Tipus de trama (1 byte)
    uint16_t data_length;     // Longitut de dades (2 bytes)
    uint8_t data[DATA_SIZE];  // Dades (ajustades al tamany restant)
    uint16_t checksum;        // Checksum (2 bytes)
    int32_t timestamp;        // Timestamp (4 bytes)
} Frame;

typedef enum {
    FRAME_SUCCESS = 0,
    FRAME_RECV_ERROR = -1,
    FRAME_DISCONNECTED = -2,
    FRAME_PENDING = -3 
} FrameErrorCode;

typedef struct {
    Frame *frame;
    FrameErrorCode error_code;
} FrameResult;

//Funcions

/*********************************************** 
* 
* @Finalidad: Crear y inicializar una nueva estructura `Frame`, asignando memoria dinámica 
*             y configurando sus campos según los parámetros proporcionados. 
* 
* @Parámetros: 
* in: type = Tipo de la trama, representado como un entero. 
* in: data = Puntero a los datos que se incluirán en la trama (puede ser NULL). 
* in: dataLength = Longitud de los datos proporcionados. Si excede `DATA_SIZE`, 
*                  se truncará al tamaño máximo permitido. 
* 
* @Retorno: 
*           Puntero a la estructura `Frame` creada e inicializada. 
*           Retorna NULL si ocurre un error al asignar memoria. 
* 
************************************************/
Frame *FRAME_createFrame(int type, const char *data, size_t dataLength);

/*********************************************** 
* 
* @Finalidad: Liberar la memoria asignada dinámicamente para una estructura `Frame`. 
* 
* @Parámetros: 
* in: frame = Puntero a la estructura `Frame` que se desea liberar. 
* 
* @Retorno: Ninguno. 
* 
************************************************/
void FRAME_destroyFrame(Frame *frame);

/*********************************************** 
* 
* @Finalidad: Enviar una estructura `Frame` a través de un socket, serializándola previamente 
*             y escribiendo su representación completa en el socket. 
* 
* @Parámetros: 
* in: socket = Descriptor del socket donde se enviará la trama. 
* in: frame = Puntero a la estructura `Frame` que se desea enviar. 
* 
* @Retorno: 
*           0 = La trama fue enviada con éxito. 
*          -1 = Error al enviar la trama (e.g., trama nula o fallo en la escritura). 
* 
************************************************/
int FRAME_sendFrame(int socket, Frame *frame);

/*********************************************** 
* 
* @Finalidad: Recibir una trama (`Frame`) a través de un socket, deserializarla y verificar 
*             su integridad mediante el cálculo de su checksum. 
* 
* @Parámetros: 
* in: socket = Descriptor del socket desde el cual se recibirá la trama. 
* 
* @Retorno: 
*           Una estructura `FrameResult` que contiene: 
*           - `frame`: Puntero a la trama recibida (NULL si hubo un error). 
*           - `error_code`: Código que indica el resultado de la operación:
*               - `FRAME_SUCCESS`: La trama se recibió correctamente. 
*               - `FRAME_DISCONNECTED`: El extremo remoto cerró la conexión. 
*               - `FRAME_PENDING`: Error debido a un descriptor de archivo inválido. 
*               - `FRAME_RECV_ERROR`: Error durante la recepción o problemas con el checksum. 
* 
************************************************/
FrameResult FRAME_receiveFrame(int socket);

/*********************************************** 
* 
* @Finalidad: Escribir entradas en un archivo de log, incluyendo un timestamp legible 
*             y un mensaje especificado. 
* 
* @Parámetros: 
* in: frame = Puntero a la estructura `Frame` que contiene el timestamp para la entrada del log. 
* in: log_fd = Descriptor de archivo del archivo de log donde se escribirán las entradas. 
* in: message = Mensaje que se incluirá en la entrada del log. Si el mensaje es `"X"`, 
*               se escribe una señal de parada en el log. 
* 
* @Retorno: Ninguno. 
* 
************************************************/
void writeLog(Frame *frame, int fd_log, char* message);
#endif // _FRAME_CUSTOM_H_