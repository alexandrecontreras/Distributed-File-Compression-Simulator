/*********************************************** 
* 
* @Autores: Alexandre Contreras, Armand López. 
* 
* @Finalidad: Implementar funciones para gestionar la comunicación entre procesos Fleck, 
*             Gotham y los workers, incluyendo la conexión, el intercambio de tramas y 
*             la gestión de respuestas en el sistema de distorsión. 
* 
* @Fecha de creación: 20 de octubre de 2024. 
* 
* @Última modificación: 4 de enero de 2025. 
* 
************************************************/

#include "communication.h"

/*********************************************** 
* 
* @Finalidad: Obtener la dirección IP y el puerto local asociados a un socket específico. 
*             Convierte la IP a formato de cadena y devuelve el puerto como un entero. 
* 
* @Parámetros: 
* in: socket_fd = Descriptor de archivo del socket del cual se desea obtener la IP y el puerto. 
* out: ip_buffer = Buffer donde se almacenará la IP en formato de cadena. 
* in: ip_buffer_size = Tamaño máximo del buffer de la IP, en bytes. 
* out: port = Puntero donde se almacenará el número de puerto local asociado al socket. 
* 
* @Retorno: Retorna un entero que indica el estado de la operación:
*           0 = Operación exitosa. La IP y el puerto se han extraído correctamente. 
*          -1 = Error al obtener la IP y el puerto o al convertir la IP a cadena. 
* 
************************************************/
int COMM_getFleckIpAndPort(int socket_fd, char *ip_buffer, size_t ip_buffer_size, int *port) {
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);

    //obtenim la IP i el port locals associats al socket 
    if (getsockname(socket_fd, (struct sockaddr *)&addr, &addr_len) == -1) {
        //IO_printStatic(STDOUT_FILENO, "Error: Failed to obtain the local IP and port.\n");
        return -1;
    }

    //convertim l'adreça IP a format de cadena
    if (inet_ntop(AF_INET, &addr.sin_addr, ip_buffer, ip_buffer_size) == NULL) {
        IO_printStatic(STDOUT_FILENO, "Error: Failed to convert IP to string.\n");
        return -1;
    }

    //assignem el port local
    *port = ntohs(addr.sin_port);

    return 0;
}

/*********************************************** 
* 
* @Finalidad: Enviar una trama de conexión desde el proceso Fleck al servidor Gotham. 
*             La trama incluye el nombre de usuario, la dirección IP y el puerto local 
*             del proceso Fleck. 
* 
* @Parámetros: 
* in: gotham_socket = Descriptor del socket conectado al servidor Gotham. 
* in: config = Puntero a la configuración del proceso Fleck que contiene el nombre de usuario. 
* 
* @Retorno: Retorna un entero que indica el estado de la operación:
*           0 = Operación exitosa. La trama de conexión se ha enviado correctamente. 
*          -1 = Error al obtener la IP y el puerto, crear la trama o enviarla. 
* 
************************************************/
int COMM_sendConnectionFrame(int gotham_socket, FleckConfig *config) {
    char *data;
    char local_ip[INET_ADDRSTRLEN];
    int local_port; 

    //obtenim la IP i el port locals del fleck
    if (COMM_getFleckIpAndPort(gotham_socket, local_ip, sizeof(local_ip), &local_port) == -1) {
        //IO_printStatic(STDOUT_FILENO, "Error: Failed to obtain the local IP and port.\n");
        return -1;
    }

    //creem la cadena amb el username, IP i port del fleck
    if (asprintf(&data, "%s&%s&%d", config->username, local_ip, local_port) == -1) {
        IO_printStatic(STDOUT_FILENO, "Error: The connection string has not been created.\n");
        return -1;  
    }
    
    //creem la trama de connexió
    Frame *connect_frame = FRAME_createFrame(0x01, data, strlen(data));
    
    //enviem la trama de connexió a Gotham
    int result = FRAME_sendFrame(gotham_socket, connect_frame);

    //destruïm la trama 
    FRAME_destroyFrame(connect_frame);

    //alliberem la memòria assignada per asprintf
    free(data);
    
    return result; 
}

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
int COMM_connectToGotham(int gotham_socket, FleckConfig *config) {
    //enviem trama de connexió (username, IP i port)
    if (COMM_sendConnectionFrame(gotham_socket, config) < 0) {
        //IO_printStatic(STDOUT_FILENO, RED "Unable to send connection frame to Gotham.\n" RESET);       
        return -1;  
    }

    //rebem resposta de gotham (OK o KO)
    FrameResult result = FRAME_receiveFrame(gotham_socket);
    if (result.error_code != FRAME_SUCCESS) {
        //no mirem frame_disconnected ja que ja mirem caiguda al keep-alive
        return -1;
    }

    Frame *frame = result.frame;    
    //verifiquem la resposta de Gotham
    if (frame->type == 0x01) {
        if (frame->data_length == 0) {
            IO_printFormat(STDOUT_FILENO, GREEN "%s connected to Mr. J System. Let the chaos begin!:)\n" RESET, config->username); //gotham ha retornat OK
            FRAME_destroyFrame(frame);

            return 0;  
        }
        if (frame->data_length > 0 && strcmp((char *)frame->data, "CON_KO") == 0) {
            FRAME_destroyFrame(frame); //gotham ha retornat KO
            return -1;  
        }
    }

    //gotham no ha rebut la trama correctament i ha enviat errorFrame
    if (frame->type == 0x09) {
        //todo: gestionar reenviament de trama?
        FRAME_destroyFrame(frame);  
        return -1; 
    }

    //destruïm trama
    FRAME_destroyFrame(frame);
    return 0;  
}

/*********************************************** 
* 
* @Finalidad: Enviar una solicitud de trabajo al servidor Gotham, especificando si la 
*             solicitud es una reconexión o una nueva petición de trabajo. 
* 
* @Parámetros: 
* in: data = Cadena de datos que contiene la información necesaria para la solicitud. 
*           Este puntero se libera internamente tras enviar la solicitud. 
* in: gotham_socket = Descriptor del socket conectado al servidor Gotham. 
* in: reconnecting = Indicador de si la solicitud corresponde a una reconexión (1) 
*                    o a una nueva petición de trabajo (0). 
* 
* @Retorno: Retorna un entero que indica el estado de la operación:
*           0 = Solicitud enviada correctamente. 
*          -1 = Error al crear o enviar la trama al servidor Gotham. 
* 
************************************************/
int COMM_sendWorkerRequestToGotham(char *data, int gotham_socket, int reconnecting) {
    Frame *request_frame = FRAME_createFrame(reconnecting? 0x11 : 0x10, data, strlen(data));
    int result = FRAME_sendFrame(gotham_socket, request_frame);

    FRAME_destroyFrame(request_frame);
    free(data);
    return result;
}

/*********************************************** 
* 
* @Finalidad: Procesar la respuesta recibida del servidor Gotham, extrayendo la dirección 
*             IP y el puerto del worker asignado o manejando errores relacionados con 
*             la disponibilidad de workers. 
* 
* @Parámetros: 
* in: result_frame = Puntero a la estructura `FrameResult` que contiene la trama de respuesta 
*                    del servidor Gotham y el código de error asociado. 
* out: data_buffer = Puntero al buffer donde se almacenará temporalmente la respuesta recibida. 
*                    La memoria asignada se debe liberar externamente cuando ya no se use. 
* out: worker_ip = Puntero donde se almacenará la dirección IP del worker asignado. 
* out: worker_port = Puntero donde se almacenará el puerto del worker asignado. 
* in: type = Tipo de archivo o tarea solicitada (e.g., "media", "text"). 
* in: reconnecting = Indicador de si la solicitud era una reconexión (1) o una nueva petición (0). 
* in: print_mutex = Mutex para garantizar la exclusión mutua al imprimir mensajes de error. 
* 
* @Retorno: Retorna un entero que indica el estado de la operación:
*           0 = Operación exitosa. Se ha obtenido la dirección IP y el puerto del worker. 
*          -1 = Error en la recepción de la respuesta, en la asignación de memoria, o si no hay 
*               workers disponibles o soportan el tipo de archivo solicitado. 
* 
************************************************/
int COMM_processGothamResponse(FrameResult *result_frame, char **data_buffer, char **worker_ip, int *worker_port, const char* type, int reconnecting, pthread_mutex_t *print_mutex) {
    if (result_frame->error_code != FRAME_SUCCESS) {
        STRING_printF(print_mutex, STDOUT_FILENO, RED, "Error: Failed to receive response from Gotham.\n");
        return -1;
    }

    Frame *response_frame = result_frame->frame;

    //si el tipus de resposta és 0x10 pot ser: resposta satisfactoria amb ip i port del worker, distort_ko o media_ko
    if (response_frame->type == (reconnecting ? 0x11 : 0x10)) {
        if (strncmp((char *)response_frame->data, "DISTORT_KO", 11) == 0) {
            STRING_printF(print_mutex, STDOUT_FILENO, RED, "Error: No workers available for the requested %s type.\n", type);
            FRAME_destroyFrame(response_frame);
            return -1;  //no hi ha workers disponibles
        }
        
        if (strncmp((char *)response_frame->data, "MEDIA_KO", 8) == 0) {
            STRING_printF(print_mutex, STDOUT_FILENO, RED, "Error: No worker supports the requested file type.\n");
            FRAME_destroyFrame(response_frame);
            return -1;  //el tipus de fitxer a distorsionar no és suportat pels workers
        }

        //si arribem aquí significa que hem rebut la ip i port del worker
        *data_buffer = strdup((char *)response_frame->data);  //copiem les dades de la trama a buffer auxiliar
        if (!*data_buffer) {
            STRING_printF(print_mutex, STDOUT_FILENO, RED, "Error: Could not allocate memory for response.\n");
            FRAME_destroyFrame(response_frame);
            return -1;
        }

        //extreiem la IP i el port del worker
        *worker_ip = strtok(*data_buffer, "&"); 
        char *worker_port_str = strtok(NULL, "&");
        *worker_port = atoi(worker_port_str); 

        FRAME_destroyFrame(response_frame);
        return 0;  
    }

    // Si trama rebuda no es correspon amb la 0x10 retornem codi d'error
    FRAME_destroyFrame(response_frame); 
    return -1;  
}

/*********************************************** 
* 
* @Finalidad: Solicitar un worker al servidor Gotham para procesar un archivo o tarea 
*             específica, y procesar la respuesta para obtener la dirección IP y puerto 
*             del worker asignado. 
* 
* @Parámetros: 
* out: data_buffer = Puntero al buffer donde se almacenará temporalmente la respuesta recibida. 
*                    La memoria asignada se debe liberar externamente cuando ya no se use. 
* out: worker_ip = Puntero donde se almacenará la dirección IP del worker asignado. 
* out: worker_port = Puntero donde se almacenará el puerto del worker asignado. 
* in: media_type = Tipo de medio solicitado (e.g., "media", "text"). 
* in: filename = Nombre del archivo a procesar. 
* in: gotham_socket = Descriptor del socket conectado al servidor Gotham. 
* in: reconnecting = Indicador de si la solicitud es una reconexión (1) o una nueva petición (0). 
* in: print_mutex = Mutex para garantizar la exclusión mutua al imprimir mensajes de error. 
* 
* @Retorno: Retorna un entero que indica el estado de la operación:
*           0 = Operación exitosa. Se ha recibido y procesado correctamente la respuesta. 
*          -1 = Error durante la creación de la trama, envío de la solicitud o procesamiento 
*               de la respuesta del servidor Gotham. 
* 
************************************************/
int COMM_requestWorkerAndProcessResponse(char** data_buffer, char** worker_ip, int* worker_port, const char* media_type, const char* filename, int gotham_socket, int reconnecting, pthread_mutex_t *print_mutex) {
    char *data = NULL;

    // Creem camp de dades de la trama
    if (asprintf(&data, "%s&%s", media_type, filename) == -1) {
        STRING_printF(print_mutex, STDOUT_FILENO, RED, "Error: Could not allocate memory for the request.\n");
        return -1;
    }

    // Enviem petició a Gotham
    if (COMM_sendWorkerRequestToGotham(data, gotham_socket, reconnecting) < 0) {
        return -1;
    }

    // Rebem i processem resposta de Gotham
    FrameResult result_frame = FRAME_receiveFrame(gotham_socket);
    int response_result = COMM_processGothamResponse(&result_frame, data_buffer, worker_ip, worker_port, media_type, reconnecting, print_mutex);

    return response_result;
}

/*********************************************** 
* 
* @Finalidad: Actualizar la estructura `MainWorker` con la dirección IP y el puerto de 
*             un nuevo worker principal. Si el worker es el mismo que el ya registrado, 
*             no realiza cambios significativos. 
* 
* @Parámetros: 
* in/out: main_worker = Puntero a la estructura `MainWorker` que contiene la IP y el 
*                       puerto del worker principal actual. 
* in: worker_ip = Cadena que contiene la dirección IP del nuevo worker principal. 
* in: worker_port = Puerto del nuevo worker principal. 
* 
* @Retorno: Retorna un entero que indica el estado de la operación:
*           0 = La estructura se ha actualizado con un nuevo worker principal. 
*           1 = No se realizaron cambios porque el worker actual ya está registrado. 
* 
************************************************/
int COMM_updateMainWorker(MainWorker* main_worker, char* worker_ip, int worker_port) {
    // Si venim d'una primera connexió de fleck a worker, simplement actualitzem l'estructua de main_worker
    if(main_worker->ip == NULL) goto assign_ip_and_port;
    // Si la connexió amb un worker no es tracta de la primera establerta, mirem si la connexió s'estableix amb el mateix worker previ. En aquest cas no cal actualitzar l'estructura.
    if(!strcmp(main_worker->ip, worker_ip) && main_worker->port == worker_port) return 1; 

    freePointer((void**)&main_worker->ip);   // Alliberem la cadena prèvia corresponent a la ip de l'antic worker principal

assign_ip_and_port:    
    main_worker->ip = strdup(worker_ip);    // Assignem a l'estructura la nova ip del worker principal
    main_worker->port = worker_port; 
    return 0; 
}

/*********************************************** 
* 
* @Finalidad: Establecer una conexión con un worker dado su dirección IP y puerto, 
*             devolviendo un socket para la comunicación. 
* 
* @Parámetros: 
* in: worker_ip = Cadena que contiene la dirección IP del worker al que se desea conectar. 
* in: worker_port = Puerto del worker al que se desea conectar. 
* in: print_mutex = Mutex para garantizar la exclusión mutua al imprimir mensajes de error. 
* 
* @Retorno: Retorna un entero que representa el descriptor de socket de la conexión:
*           >= 0 = Descriptor de socket válido si la conexión fue exitosa. 
*           -1 = Error al intentar establecer la conexión. 
* 
************************************************/

int COMM_connectToWorker(const char* worker_ip, int worker_port, pthread_mutex_t *print_mutex) {
    int worker_socket = SOCKET_initClientSocket(worker_ip, worker_port);
    if (worker_socket < 0) {
        STRING_printF(print_mutex, STDOUT_FILENO, RED, "Error: Failed to connect to worker with IP: %s, Port: %d\n", worker_ip, worker_port);
        return -1;
    }
    return worker_socket;
}

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
int COMM_requestWorkerAndEstablishConnection(char* filename, char* type, MainWorker* main_worker, int gotham_socket, int reconnecting_flag, pthread_mutex_t *print_mutex) {
    char* data_buffer = NULL;
    char* worker_ip = NULL; 
    int worker_port = -1;
    int connected_to_same_worker = 0; // Flag per indicar si la connexió s'estableix amb el mateix worker principal al que estavem connectats prèviament. Això ens interessa per quan fleck faci una distorsió i rebi un frame_disconnected de part del worker, quan faci la reconnexió sàpiga si la desconnexió ha sigut per una caiguda (!connected_to_same_worker) o per una fallida d'alguna fase del procés de distorsió (connected_to_same_worker).
    
    // Demanem a gotham la ip i port del worker principal segons el tipus de distorsió sol·licitat i processem la resposta
    int result = COMM_requestWorkerAndProcessResponse(&data_buffer, &worker_ip, &worker_port, type, filename, gotham_socket, reconnecting_flag, print_mutex); 
    if (result < 0) {
        freePointer((void**)&data_buffer); 
        return 0; 
    }
    
    STRING_printF(print_mutex, STDOUT_FILENO, YELLOW, "Retrieved main worker details. Establishing connection...\n");

    // Comprovem si fleck s'està intentant connectar al mateix worker al que estava connectat prèviament. Si no és el cas, actualitzem l'estructura the worker principal
    connected_to_same_worker = COMM_updateMainWorker(main_worker, worker_ip, worker_port); 
    // Si el worker retornat per gotham és el worker al que estavem connectats abortem connexió (hem fet reconnect per fallida del worker, no por caiguda)
    if(reconnecting_flag && connected_to_same_worker) { 
        freePointer((void**)&data_buffer); 
        return CONNECTED_TO_SAME_WORKER; 
    }

    SOCKET_closeSocket(&main_worker->socket);
    main_worker->socket = COMM_connectToWorker(worker_ip, worker_port, print_mutex);
    if (main_worker->socket < 0) {
        STRING_printF(print_mutex, STDOUT_FILENO, RED, "Error: Failed to connect to worker\n");
        freePointer((void**)&data_buffer); 
        return FAILED_TO_CONNECT;
    }

    STRING_printF(print_mutex, STDOUT_FILENO, GREEN, "Connection successful\n");

    freePointer((void**)&data_buffer); 

    return connected_to_same_worker? CONNECTED_TO_SAME_WORKER : CONNECTED_TO_NEW_WORKER; 
}

/*********************************************** 
* 
* @Finalidad: Procesar la respuesta del worker tras intentar establecer conexión para 
*             el envío de un archivo de distorsión. Verifica el tipo de trama y su contenido 
*             para determinar el estado de la conexión. 
* 
* @Parámetros: 
* in: worker_socket = Descriptor del socket conectado al worker. 
* in: print_mutex = Mutex para garantizar la exclusión mutua al imprimir mensajes de estado o error. 
* 
* @Retorno: Retorna un entero que indica el estado de la operación:
*           0 = Conexión establecida con éxito. El worker está listo para recibir el archivo. 
*          -1 = Error en la respuesta del worker o rechazo de la conexión (CON_KO). 
* 
************************************************/
int COMM_processDistortionResponse(int worker_socket, pthread_mutex_t *print_mutex) {
    FrameResult result_frame = FRAME_receiveFrame(worker_socket);
    if (result_frame.error_code != FRAME_SUCCESS) {
        STRING_printF(print_mutex, STDOUT_FILENO, RED, "Error: The worker's response is not available.\n");
        return -1;
    }

    Frame *response_frame = result_frame.frame;

    if (response_frame->type == 0x03 && response_frame->data_length == 0) {
        STRING_printF(print_mutex, STDOUT_FILENO, YELLOW, "Connection established with the worker. Ready to send the file.\n");
        FRAME_destroyFrame(response_frame);
        return 0;  
    }

    if (response_frame->type == 0x03 && response_frame->data_length > 0) {
        STRING_printF(print_mutex, STDOUT_FILENO, RED, "Error: The worker refused the connection request (CON_KO).\n");
        FRAME_destroyFrame(response_frame);
        return -1;
    }

    // Si el tipus no es correspon amb 0x03 retornem error
    FRAME_destroyFrame(response_frame);
    return -1;
}

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

int COMM_sendFileMetadata(int worker_socket, const char* username, const char* filename, int file_size, const char* md5sum, const int factor, pthread_mutex_t *print_mutex) {
    char *data = NULL;

    if(asprintf(&data, "%s&%s&%d&%s&%d", username, filename, file_size, md5sum, factor) < 0) return -1;

    // Creem i enviem trama de metadades al worker (petició de distorsió)
    Frame *metadata_frame = FRAME_createFrame(0x03, data, strlen(data));
    if(FRAME_sendFrame(worker_socket, metadata_frame) < 0) {
        STRING_printF(print_mutex, STDOUT_FILENO, RED, "Error: Failed to send warp request to worker\n");
        FRAME_destroyFrame(metadata_frame);
        free(data);
        return -1; 
    }

    // Processem la resposta del worker (CON_OK / CON_KO)
    int result = COMM_processDistortionResponse(worker_socket, print_mutex);

    free(data);
    FRAME_destroyFrame(metadata_frame);

    return result;
}

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
int COMM_reconnectToWorker(char* filename, char* worker_type, MainWorker* main_worker, int gotham_socket, pthread_mutex_t *print_mutex) {
    int reconnect_result = COMM_requestWorkerAndEstablishConnection(filename, worker_type, main_worker, gotham_socket, RECONNECTION, print_mutex);
    if (reconnect_result == FAILED_TO_CONNECT || reconnect_result == CONNECTED_TO_SAME_WORKER) {
        return 0; 
    }
    return 1; 
}

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
int COMM_retrieveFileMetadata(int worker_socket, DistortionContext* distorted_file, pthread_mutex_t *print_mutex) {
    char* data_buffer = NULL;
    int unexpected_error = 1;

    // Rebem la trama del worker
    FrameResult result = FRAME_receiveFrame(worker_socket);

    // Si hi ha error en deserialitzar la trama retornem codi d'error 
    if (result.error_code != FRAME_SUCCESS) {
        // Si ha caigut el worker setegem unexpected flag a 0
        if(result.error_code == FRAME_DISCONNECTED) {
            STRING_printF(print_mutex, STDOUT_FILENO, RED, "Worker disconnected while distortiong %s\n", distorted_file->filename);
            unexpected_error = 0; // Indiquem caiguda
        }
        if (result.frame) FRAME_destroyFrame(result.frame);
        return unexpected_error ? UNEXPECTED_ERROR : REMOTE_END_DISCONNECTION; // Retornem codi d'error: -1 = error deserialització; 0 = caiguda
    }

    Frame *response_frame = result.frame;
    
    if(response_frame->type == 0x04) {
        data_buffer = strdup((char *)response_frame->data);  // Copiem les dades de la trama a buffer auxiliar
        if (!data_buffer) {
            FRAME_destroyFrame(response_frame);
            return UNEXPECTED_ERROR; // Retornem codi d'error
        }

        // Extreiem el filesize del fitxer distorsionat i l'emmagatzemem a l'estructura de context de distorsió
        char* filesize_str = strtok(data_buffer, "&");
        distorted_file->filesize = atoi(filesize_str); 
        
        // Alliberem el md5sum del fitxer original i fem que estructura de context referencï el md5sum del fitxer distorsionat 
        freePointer((void**)&distorted_file->md5sum);
        distorted_file->md5sum = strdup(strtok(NULL, "&"));

        free(data_buffer);

        // Setegem el número de paquets equivalents al filesize del fitxer
        distorted_file->n_packets = distorted_file->filesize / DATA_SIZE;
        if (distorted_file->filesize % DATA_SIZE != 0) {
            distorted_file->n_packets++;
        }

        // Setegem número de paquets processats a 0
        distorted_file->n_processed_packets = 0;

        FRAME_destroyFrame(response_frame);
        STRING_printF(print_mutex,STDOUT_FILENO, GREEN, "Successfully retrieved distorted file's metadata and set up distortion context\n");

        return TRANSFER_SUCCESS; // Procés executat satisfactòriament 
    }

    STRING_printF(print_mutex, STDOUT_FILENO, RED, "WERROR: wrong type received as distorted file's metadata\n");

    // Si trama rebuda no es correspon amb la 0x04 retornem codi d'error
    FRAME_destroyFrame(response_frame); 
    return UNEXPECTED_ERROR;  
}

/*********************************************** 
* 
* @Finalidad: Enviar una notificación a worker/gotham para desconectar al usuario especificado. 
* 
* @Parámetros: 
* in: server_socket = Descriptor del socket conectado al worker/gotham. 
* in: username = Nombre del usuario que se está desconectando. 
* in: type = Entero que indica si la desconexión se realiza respecto a gotham o un worker. 
* @Retorno: No retorna ningún valor. 
* 
************************************************/
void COMM_disconnectFromServer(int server_socket, char* username, int type __attribute__((unused))) {
    // Creem la trama amb TYPE: 0x07 i DATA: <userName>
    Frame *disconnect_frame = FRAME_createFrame(0x07, username, strlen(username));
    if (!disconnect_frame) return;

    // Enviem la trama al worker
    FRAME_sendFrame(server_socket, disconnect_frame);

    // Alliberem la memòria del Frame
    FRAME_destroyFrame(disconnect_frame);

    //IO_printFormat(STDOUT_FILENO, YELLOW "Disconnected gracefully from %s\n" RESET, type == COMM_WORKER ? "Worker" : "Gotham");
}