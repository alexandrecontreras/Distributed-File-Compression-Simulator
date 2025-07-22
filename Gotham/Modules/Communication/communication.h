/*********************************************** 
* 
* @Autores: Alexandre Contreras y Armand López
* @Propósito: Definir funciones para gestionar la comunicación entre Gotham y sus clientes, 
*             incluyendo el envío de respuestas de asignación, errores y resultados 
*             relacionados con solicitudes de distorsión. 
* @Fecha de creación: 21 de octubre de 2024
* @Última modificación: 4 de enero de 2025
* 
************************************************/

#ifndef _GOTHAM_COMMUNICATION_CUSTOM_H_
#define _GOTHAM_COMMUNICATION_CUSTOM_H_

//Llibreies del sistema
#include <stdlib.h>     // malloc, free
#include <string.h>     // strlen
#include <unistd.h>     // STDOUT_FILENO

//Llibreries pròpies
#include "../../../Libs/IO/io.h"                      // Per a les funcions d'entrada/sortida
#include "../../../Libs/Frame/frame.h"                     // Per a les funcions de creació de frames

//Funcions

/*********************************************** 
* 
* @Finalidad: Enviar una respuesta al cliente indicando que se ha asignado un nuevo 
*             worker principal mediante una trama de tipo `0x08`. 
* 
* @Parámetros: 
* in: client_socket = Descriptor del socket del cliente al que se enviará la respuesta. 
* 
* @Retorno: Ninguno. 
* 
************************************************/
void COMM_sendNewMainWorkerResponse (int client_socket);

/*********************************************** 
* 
* @Finalidad: Enviar una trama de error al cliente mediante una trama de tipo `0x09` 
*             sin datos asociados. 
* 
* @Parámetros: 
* in: client_socket = Descriptor del socket del cliente al que se enviará la trama de error. 
* 
* @Retorno: Ninguno. 
* 
************************************************/
void COMM_sendErrorFrame(int client_socket);

/*********************************************** 
* 
* @Finalidad: Enviar una respuesta de distorsión al cliente mediante una trama con el 
*             tipo y contenido especificados, indicando si la solicitud es válida o no. 
* 
* @Parámetros: 
* in: client_socket = Descriptor del socket del cliente al que se enviará la respuesta. 
* in: string = Cadena que contiene el mensaje o datos de la respuesta. 
* in: is_valid = Indicador de validez de la respuesta (1 para válida, 0 para inválida). 
* in: type = Tipo de trama que se enviará (e.g., código específico del protocolo). 
* 
* @Retorno: Ninguno. 
* 
************************************************/
void COMM_sendDistortResponse(int client_socket, char* string, int is_valid, int type);
#endif // _GOTHAM_COMMUNICATION_CUSTOM_H_