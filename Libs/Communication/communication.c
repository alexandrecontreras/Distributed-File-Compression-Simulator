/*********************************************** 
* 
* @Autores: Alexandre Contreras, Armand López. 
* 
* @Finalidad: Proveer de funciones para la gestión de tramas en el sistema Mr J System, 
*             incluyendo el envío y recepción de archivos, verificación de integridad 
*             mediante MD5, y envío de respuestas para conexión y ACK. Estas funciones 
*             son utilizadas en la comunicación entre flecks, workers y Gotham. 
* 
* @Fecha de creación: 30 de octubre de 2024. 
* 
* @Última modificación: 4 de enero de 2025. 
* 
************************************************/

#include "communication.h"

/*********************************************** 
* 
* @Finalidad: Recibir y procesar una trama de reconocimiento (ACK) desde un socket, 
*             verificando posibles errores o desconexiones. 
* 
* @Parámetros: 
* in: socket = Descriptor del socket desde el cual se espera recibir la trama ACK. 
* 
* @Retorno: 
*           TRANSFER_SUCCESS = La trama ACK fue recibida y procesada correctamente. 
*           REMOTE_END_DISCONNECTION = El extremo remoto se desconectó. 
*           UNEXPECTED_ERROR = Error inesperado al recibir la trama. 
* 
************************************************/
int COMM_retrieveAckFrame(int socket) {
    int error = UNEXPECTED_ERROR;
    FrameResult ack_frame = FRAME_receiveFrame(socket);
    if (ack_frame.error_code != FRAME_SUCCESS) {
        if(ack_frame.error_code == FRAME_DISCONNECTED) {
            error = REMOTE_END_DISCONNECTION; 
        }
        if (ack_frame.frame) FRAME_destroyFrame(ack_frame.frame);
        return error; 
    }

    FRAME_destroyFrame(ack_frame.frame);
    return TRANSFER_SUCCESS;
}

/*********************************************** 
* 
* @Finalidad: Crear y enviar una trama de reconocimiento (ACK) a través de un socket especificado. 
* 
* @Parámetros: 
* in: socket = Descriptor del socket a través del cual se enviará la trama ACK. 
* 
* @Retorno: 
*           TRANSFER_SUCCESS = La trama ACK fue creada y enviada correctamente. 
*           UNEXPECTED_ERROR = Error al crear o enviar la trama ACK. 
* 
************************************************/
int COMM_sendAckFrame(int socket) {
    Frame *ack_frame = FRAME_createFrame(0x12, NULL, 0);
    if (!ack_frame || FRAME_sendFrame(socket, ack_frame) < 0) { // Si ack_frame és NULL, el primer operand avalua CERT i no es fa el sendFrame
        if (ack_frame) FRAME_destroyFrame(ack_frame);
        return UNEXPECTED_ERROR;
    }
    FRAME_destroyFrame(ack_frame);
    return TRANSFER_SUCCESS;
}

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
int COMM_sendFile(char* file_path, char* filename, int n_packets, int* n_processed_packets, int worker_socket, volatile int* exit_distortion, int process, pthread_mutex_t *print_mutex) {
    int ack_result = TRANSFER_SUCCESS; 

    int fd = open(file_path, O_RDONLY);
    if (fd < 0) {
        return UNEXPECTED_ERROR;
    }

    // Ens posicionem al lloc correcte segons l'últim paquet enviat 
    off_t offset = *n_processed_packets * DATA_SIZE;
    if (lseek(fd, offset, SEEK_SET) < 0) {
        close(fd);
        return UNEXPECTED_ERROR;
    }

    char buffer[DATA_SIZE];
    int bytes_read = 0;

    // Mentre el nombre de paquets enviats sigui menor al nombre de paquets totals enviem paquets al worker
    while (*n_processed_packets < n_packets && !*(exit_distortion)) {
        bytes_read = read(fd, buffer, DATA_SIZE);
        if (bytes_read < 0) {
            STRING_printF(print_mutex, STDOUT_FILENO, RED, "ERROR: failed to read file %s\n", filename);
            close(fd);
            return UNEXPECTED_ERROR;
        } else if (bytes_read == 0) {
            break; // Fi del fitxer (no hauriem d'arribar si la segmentació del fitxer en paquets és correcta)
        }

        // Crear i enviar la trama al worker
        Frame *packet_frame = FRAME_createFrame(0x05, buffer, bytes_read); 
        if (!packet_frame || FRAME_sendFrame(worker_socket, packet_frame) < 0) { // Si packet_frame és NULL el primer operand avalua CERT i per tant no s'arriba a enviar la trama
            if (packet_frame) FRAME_destroyFrame(packet_frame);
            close(fd);
            return UNEXPECTED_ERROR;
        }

        FRAME_destroyFrame(packet_frame); 

        // Esperar heartbeat del worker a mode d'ACK
        ack_result = COMM_retrieveAckFrame(worker_socket);
        if(ack_result == REMOTE_END_DISCONNECTION || ack_result == UNEXPECTED_ERROR) {
            if(ack_result == REMOTE_END_DISCONNECTION) STRING_printF(print_mutex, STDOUT_FILENO, RED, "%s crashed while receiving file %s\n", process == FLECK ? "Worker" : "Fleck", filename);
            close(fd);
            return ack_result; // Retornem WORKER DOWN si el worker ha caigut i UNEXPECTED_ERROR si ha ahgut un error en deserialitzar la trama
        }

        // resposta del worker és correcta, actualitzem l'últim paquet enviat
        (*n_processed_packets)++;
    }

    // Tanquem file descriptor del fitxer que hem llegit
    close(fd);

    if(*exit_distortion) {
        STRING_printF(print_mutex, STDOUT_FILENO, RED, "Exiting send method because of sigint\n");
        return INTERRUPTED_BY_SIGINT; 
    } else {
        STRING_printF(print_mutex, STDOUT_FILENO, GREEN, "Successfully sent distorted file to %s\n", process == FLECK ? "Worker" : "Fleck");
        return TRANSFER_SUCCESS; 
    }
}

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
int COMM_receiveFile(char* file_path, char* filename, int n_packets, int* n_processed_packets, int worker_socket, volatile int* exit_distortion, int process, pthread_mutex_t *print_mutex) {
    int unexpected_error = 1;

    // Obrim el fitxer en mode escriptura (per worker ens interessa flag d'append pero per fleck no ja que volem sobreescriure el contingut del fitxer original)
    int fd = open(file_path, O_WRONLY | O_CREAT, 0666);
    if (fd < 0) {
        STRING_printF(print_mutex, STDOUT_FILENO, MAGENTA, "ERROR: failed to open file %s\n", filename);
        return UNEXPECTED_ERROR;
    }

    // Ens posicionem al lloc correcte segons l'últim paquet rebut
    off_t offset = *n_processed_packets * DATA_SIZE;
    if (lseek(fd, offset, SEEK_SET) < 0) {
        close(fd);
        return UNEXPECTED_ERROR;
    }

    // Mentre no haguem rebut tots els paquets, continuem processant
    while (*n_processed_packets < n_packets && !*(exit_distortion)) {
        // Rebem la trama del worker
        FrameResult result = FRAME_receiveFrame(worker_socket);
        if (result.error_code != FRAME_SUCCESS) {
            if (result.error_code == FRAME_DISCONNECTED) {
                STRING_printF(print_mutex, STDOUT_FILENO, RED, "%s disconnected while sending file %s\n", process == FLECK ? "Worker" : "Fleck", filename);
                unexpected_error = 0;
            }
            if (result.frame) FRAME_destroyFrame(result.frame);
            close(fd);
            return unexpected_error ? UNEXPECTED_ERROR : REMOTE_END_DISCONNECTION;
        }

        Frame *packet_frame = result.frame;

        // Si el tipus de trama rebut no és correcte abortem amb codi d'error
        if (packet_frame->type != 0x05) {
            FRAME_destroyFrame(packet_frame);
            close(fd);
            return UNEXPECTED_ERROR;
        }

        // Escrivim les dades del paquet al fitxer
        if (write(fd, packet_frame->data, packet_frame->data_length) < 0) {
            FRAME_destroyFrame(packet_frame);
            close(fd);
            return UNEXPECTED_ERROR;
        }

        FRAME_destroyFrame(packet_frame);

        // Confirmem la recepció al worker enviant-li un heartbeat a mode d'ACK
        if(COMM_sendAckFrame(worker_socket) != TRANSFER_SUCCESS) {
            close(fd);
            return UNEXPECTED_ERROR; 
        }

        // Actualitzem el nombre de paquets processats
        (*n_processed_packets)++;
    }

    // Tanquem el fitxer i alliberem recursos
    close(fd);
    
    if(*exit_distortion) {
        STRING_printF(print_mutex, STDOUT_FILENO, RED, "Exiting receive method because of sigint\n");
        return INTERRUPTED_BY_SIGINT; 
    } else {
        STRING_printF(print_mutex, STDOUT_FILENO, GREEN, "Successfully received %s's file\n", process == FLECK ? "Worker" : "Fleck");
        return TRANSFER_SUCCESS; 
    }    
}

/*********************************************** 
* 
* @Finalidad: Crear y enviar una trama de comprobación MD5 a través de un socket, 
*             utilizando el tipo de trama y contenido especificado. 
* 
* @Parámetros: 
* in: client_socket = Descriptor del socket al que se enviará la trama. 
* in: type = Tipo de trama que se enviará (por ejemplo, para indicar éxito o fallo en la verificación MD5). 
* in: string = Cadena que contiene el contenido de la trama (e.g., resultado de la verificación MD5). 
* 
* @Retorno: 
*           0 = La trama fue enviada con éxito. 
*          -1 = Error al enviar la trama. 
* 
************************************************/
int COMM_sendMD5CheckFrame (int client_socket, int type, char* string) {
    int send_success = 0;
    Frame* check_frame = FRAME_createFrame(type, string, strlen(string)); 
    if(FRAME_sendFrame(client_socket, check_frame) < 0) send_success = -1;
    FRAME_destroyFrame(check_frame);
    return send_success; 
}

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
int COMM_verifyFileIntegrity(char* file_path, char* md5sum, int worker_socket, pthread_mutex_t *print_mutex) {
    int md5_match = FILE_compareMD5(md5sum, file_path);
 
    if(md5_match) {
        // MD5 coincideix
        if(COMM_sendMD5CheckFrame(worker_socket, 0x06, "CHECK_OK") < 0) {
            STRING_printF(print_mutex, STDOUT_FILENO, RED, "ERROR: failed to send check frame\n");
            return UNEXPECTED_ERROR;
        }
        STRING_printF(print_mutex, STDOUT_FILENO, GREEN, "Reassembled file matches the expected md5\n");
    } else {
        // MD5 no coincideix
        if(COMM_sendMD5CheckFrame(worker_socket, 0x06, "CHECK_KO") < 0) {
            STRING_printF(print_mutex, STDOUT_FILENO, RED, "ERROR: failed to send check frame\n");
            return UNEXPECTED_ERROR;
        }
        STRING_printF(print_mutex, STDOUT_FILENO, RED, "ERROR: md5 mismatch between the original and reassembled file\n");
    }

    return md5_match ? TRANSFER_SUCCESS : UNEXPECTED_ERROR; 
}

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
int COMM_retrieveMD5Check(int worker_socket, int process, pthread_mutex_t *print_mutex) {
    int unexpected_error = 1; 

    // Rebem la trama del worker
    FrameResult result = FRAME_receiveFrame(worker_socket);

    // Si hi ha error en deserialitzar la trama retornem codi d'error (si caigués el worker en aquest punt es consideraria error)
    if (result.error_code != FRAME_SUCCESS) {
        if (result.error_code == FRAME_DISCONNECTED) {
            STRING_printF(print_mutex, STDOUT_FILENO, RED, "%s disconnected while sending MD5 check\n", process == FLECK ? "Worker" : "Fleck");
            unexpected_error = 0;
        }
        if (result.frame) FRAME_destroyFrame(result.frame);
        return unexpected_error ? UNEXPECTED_ERROR : REMOTE_END_DISCONNECTION;
    }

    Frame *response_frame = result.frame;
    if (response_frame->type == 0x06) {
        if (strncmp((char *)response_frame->data, "CHECK_OK", 8) == 0) {
            STRING_printF(print_mutex, STDOUT_FILENO, GREEN, "%s successfully reassembled the file!\n", process == FLECK ? "Worker" : "Fleck");
            FRAME_destroyFrame(response_frame);
            return TRANSFER_SUCCESS;  // CHECK_OK
        }
        
        if (strncmp((char *)response_frame->data, "CHECK_KO", 8) == 0) {
            STRING_printF(print_mutex, STDOUT_FILENO, RED, "Error: %s failed to reassmble the file\n", process == FLECK ? "Worker" : "Fleck");
            FRAME_destroyFrame(response_frame);
            return UNEXPECTED_ERROR;  // CHECK_KO
        }

        FRAME_destroyFrame(response_frame);
        return UNEXPECTED_ERROR;  
    }

    FRAME_destroyFrame(response_frame);
    return UNEXPECTED_ERROR;
}

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
void COMM_sendConnectionResponse(int client_socket, char* string_err, int is_valid, int type) {
    Frame *response_frame;

    if (is_valid) {
        response_frame = FRAME_createFrame(type, "", 0);
    } else {
        response_frame = FRAME_createFrame(type, string_err, strlen(string_err));
    }

    if(FRAME_sendFrame(client_socket, response_frame) < 0) {
        if (type == 0x01) {
            IO_printStatic(STDOUT_FILENO, RED "Failed to send connnection response frame to fleck.\n" RESET);
        } else {
            IO_printStatic(STDOUT_FILENO, RED "Failed to send connnection response frame to worker.\n" RESET);
        }
    }

    FRAME_destroyFrame(response_frame);
}