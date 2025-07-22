/*********************************************** 
* 
* @Autores: Alexandre Contreras, Armand López. 
* 
* @Finalidad: Implementar funciones para la comunicación con clientes en el sistema 
*             de distorsión, permitiendo el envío de respuestas de asignación, 
*             errores y resultados de distorsión a través de tramas. 
* 
* @Fecha de creación: 25 de octubre de 2024. 
* 
* @Última modificación: 4 de enero de 2025. 
* 
************************************************/

#include "communication.h"

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
void COMM_sendNewMainWorkerResponse (int client_socket) {
    Frame *response_frame;

    response_frame = FRAME_createFrame(0x08, "", 0);

    if(FRAME_sendFrame(client_socket, response_frame) < 0) {
        IO_printStatic(STDOUT_FILENO, RED "Failed assign new main worker\n" RESET);
    }

    IO_printStatic(STDOUT_FILENO, YELLOW "Assigning new main worker...\n" RESET);
    FRAME_destroyFrame(response_frame);
}

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
void COMM_sendErrorFrame(int client_socket) {
    // TYPE: 0x09, DATA: Buit
    Frame *error_frame = FRAME_createFrame(0x09, NULL, 0);  

    if (FRAME_sendFrame(client_socket, error_frame) < 0) {
        IO_printStatic(STDOUT_FILENO, "Failed to send error frame to client.\n");
    }

    //destruïm la trama d'error després d'enviar-la
    FRAME_destroyFrame(error_frame);
}

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
void COMM_sendDistortResponse(int client_socket, char* string, int is_valid, int type) {
    Frame *response_frame;

    if (is_valid) {
        response_frame = FRAME_createFrame(type, string, strlen(string));

    } else {
        response_frame = FRAME_createFrame(type, string, strlen(string));
    }

    if(FRAME_sendFrame(client_socket, response_frame) < 0) {
        IO_printStatic(STDOUT_FILENO, RED "Failed to send distord response frame to fleck.\n" RESET);
    }

    FRAME_destroyFrame(response_frame);
}