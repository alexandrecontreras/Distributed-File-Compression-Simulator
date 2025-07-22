/*********************************************** 
* 
* @Autores: Alexandre Contreras, Armand López. 
* 
* @Finalidad: Proveer funciones para la creación, serialización, 
*             deserialización, envío y recepción de tramas (`Frame`) 
*             utilizadas en el sistema, incluyendo la gestión de logs 
*             para auditoría y validación de mensajes. 
* 
* @Fecha de creación: 10 de noviembre de 2024. 
* 
* @Última modificación: 4 de enero de 2025. 
* 
************************************************/

#include "frame.h"

/*********************************************** 
* 
* @Finalidad: Calcular el checksum de una trama (`Frame`) utilizando los campos de 
*             la estructura, para validar su integridad. 
* 
* @Parámetros: 
* in: frame = Puntero a la estructura `Frame` de la cual se calculará el checksum. 
* 
* @Retorno: 
*           Valor `uint16_t` que representa el checksum calculado, obtenido como el 
*           módulo de la suma acumulada de los campos de la trama. 
* 
************************************************/
uint16_t FRAME_calculateChecksum(const Frame *frame) {
    uint32_t sum = 0;
    sum += frame->type;
    sum += frame->data_length;
    for (int i = 0; i < DATA_SIZE; i++) {
        sum += frame->data[i];
    }
    sum += frame->timestamp & 0xFFFF;
    sum += (frame->timestamp >> 16) & 0xFFFF;

    return (uint16_t)(sum % 65536); //mòdul 2^16
}

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
Frame *FRAME_createFrame(int type, const char *data, size_t dataLength) {
    Frame *frame = (Frame *)malloc(sizeof(Frame));
    if (!frame) return NULL;

    uint8_t frameType = (uint8_t)type;
    uint16_t frameDataLength = (uint16_t)(dataLength > DATA_SIZE ? DATA_SIZE : dataLength);

    frame->type = frameType;
    frame->data_length = frameDataLength;

    memset(frame->data, 0, DATA_SIZE); //inicialitzem tot el camp de dades a 0 (padding)
    if (data) {
        memcpy(frame->data, data, frame->data_length);
    }
    
    frame->timestamp = (int32_t)time(NULL);      //generem timestamp
    frame->checksum = FRAME_calculateChecksum(frame);  //calculem checksum
    return frame;
}

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
void FRAME_destroyFrame(Frame *frame) {
    if (frame) {
        free(frame);
    }
}

/*********************************************** 
* 
* @Finalidad: Serializar una estructura `Frame` en un buffer de bytes, convirtiendo 
*             sus campos en un formato adecuado para la transmisión o almacenamiento. 
* 
* @Parámetros: 
* in: frame = Puntero a la estructura `Frame` que se desea serializar. 
* out: buffer = Puntero al buffer donde se almacenará la representación serializada 
*               de la trama. El buffer debe tener suficiente espacio para almacenar 
*               todos los campos de la estructura `Frame`. 
* 
* @Retorno: Ninguno. 
* 
************************************************/
void FRAME_serializeFrame(const Frame *frame, uint8_t *buffer) {
    int offset = 0;
    
    //serialitzem camp type
    buffer[offset] = frame->type;
    offset += sizeof(frame->type);
    
    //serialitzem camp data length
    buffer[offset] = (frame->data_length >> 8) & 0xFF; //byte alt (8MSB)
    buffer[offset + 1] = frame->data_length & 0xFF;     //byte baix (8LSB)
    offset += sizeof(frame->data_length);
    
    //serialitzem dades
    memcpy(buffer + offset, frame->data, sizeof(frame->data));
    offset += sizeof(frame->data);
    
    //serialitzem checksum
    buffer[offset] = (frame->checksum >> 8) & 0xFF; //byte alt (8MSB)
    buffer[offset + 1] = frame->checksum & 0xFF;    //byte baix (8LSB)
    offset += sizeof(frame->checksum);
    
    //serialitzem timestamp
    buffer[offset] = (frame->timestamp >> 24) & 0xFF; //byte alt (8MSB)
    buffer[offset + 1] = (frame->timestamp >> 16) & 0xFF;
    buffer[offset + 2] = (frame->timestamp >> 8) & 0xFF;
    buffer[offset + 3] = frame->timestamp & 0xFF;      //byte baix (8LSB)
}

/*********************************************** 
* 
* @Finalidad: Deserializar un buffer de bytes en una estructura `Frame`, reconstruyendo 
*             los campos de la trama a partir de su representación serializada. 
* 
* @Parámetros: 
* in: buffer = Puntero al buffer que contiene los datos serializados de la trama. 
* out: frame = Puntero a la estructura `Frame` donde se almacenarán los datos deserializados. 
* 
* @Retorno: Ninguno. 
* 
************************************************/
void FRAME_deserializeFrame(const uint8_t *buffer, Frame *frame) {
    int offset = 0;

    //deserialitzem camp type
    frame->type = buffer[offset];
    offset += sizeof(frame->type);
    
    //deserialitzem camp data length
    frame->data_length = (buffer[offset] << 8) | buffer[offset + 1]; //shiftem el byte alt i el sumem amb el byte baix
    offset += sizeof(frame->data_length);
    
    //deserialitzem dades
    memcpy(frame->data, buffer + offset, sizeof(frame->data));
    offset += sizeof(frame->data);
    
    //deserialitzem checksum
    frame->checksum = (buffer[offset] << 8) | buffer[offset + 1];
    offset += sizeof(frame->checksum);
    
    //deseralitzem timestamp
    frame->timestamp = (buffer[offset] << 24) | (buffer[offset + 1] << 16) | //shiftem bytes i fem suma lògica
                       (buffer[offset + 2] << 8) | buffer[offset + 3];
}

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
int FRAME_sendFrame(int socket, Frame *frame) {
    if (!frame) return -1;

    //calculem el checksum de la trama
    frame->checksum = FRAME_calculateChecksum(frame);

    //serialitzem la trama en un buffer de 256 bytes
    uint8_t buffer[FRAME_SIZE];
    FRAME_serializeFrame(frame, buffer);

    //enviem trama en una sola escriptura de 256 bytes
    if (write(socket, buffer, FRAME_SIZE) != FRAME_SIZE) {
        perror("Failed to send frame: ");
        return -1;
    }

    return 0; 
}

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
FrameResult FRAME_receiveFrame(int socket) {
    uint8_t buffer[FRAME_SIZE];
    FrameResult result = {NULL, FRAME_SUCCESS};

    ssize_t bytes_received = read(socket, buffer, FRAME_SIZE); 
    if (bytes_received == 0) {
        //La connexió s'ha tancat pel costat remot
        result.error_code = FRAME_DISCONNECTED;
        return result;
    } else if (bytes_received < 0) {
        if (errno == ECONNRESET) {
            // La connexió ha estat reiniciada pel costat remot
            result.error_code = FRAME_DISCONNECTED;
        } else if (errno == EBADF) {
            // Descriptor de fitxer invàlid. Si el socket s'ha tancat abans de fer la lectura
            result.error_code = FRAME_PENDING; //error: bad file descriptor, entrarem a aquesta condició quan es tanqui 
        } else {
            // Qualsevol altre problema amb la lectura
            result.error_code = FRAME_RECV_ERROR;
        }
        return result;
    }

    //creem i deserialitzem la trama
    result.frame = (Frame *)malloc(sizeof(Frame));
    if (!result.frame) {
        result.error_code = FRAME_RECV_ERROR;
        return result;
    }

    FRAME_deserializeFrame(buffer, result.frame);

    //comprovem el checksum
    if (result.frame->checksum != FRAME_calculateChecksum(result.frame)) {
        free(result.frame);
        // Error en el checksum
        result.error_code = FRAME_RECV_ERROR;
    }

    return result;
}

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
void writeLog(Frame *frame, int log_fd, char* message) {
    if (!frame) {
        return;
    }

    if (strcmp(message, "X") == 0) {
        // Escribir el mensaje de parada
        asprintf(&message, "X\n");
        write(log_fd, message, strlen(message));
        free(message);
        return;
    }

    // Convertir el timestamp a formato de tiempo legible
    time_t raw_time = (time_t)frame->timestamp;
    struct tm *time_info = localtime(&raw_time);

    // Crear la cadena con el formato [YYYY-MM-DD HH:MM:SS]
    char *log_entry = NULL;
    if (time_info) {
        asprintf(&log_entry, "[%04d-%02d-%02d %02d:%02d:%02d] %s\n",
                 time_info->tm_year + 1900,
                 time_info->tm_mon + 1,
                 time_info->tm_mday,
                 time_info->tm_hour,
                 time_info->tm_min,
                 time_info->tm_sec, 
                 message);
    } else {
        asprintf(&log_entry, "[Timestamp inválido: %d]\n", frame->timestamp);
    }

    // Escribir la entrada byte a byte
    if (log_entry) {
        for (size_t i = 0; i < strlen(log_entry); i++) {
            if (write(log_fd, &log_entry[i], 1) == -1) {
                perror("Error writing to log file");
                break;
            }
        }
    }

    free(log_entry); // Liberar memoria dinámica
}


