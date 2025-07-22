/***********************************************
*
* @Autores: Alexandre Contreras, Armand López.
* 
* @Finalidad: Implementar funciones de comunicación con Gotham y Flecks, incluyendo 
*             el manejo de tramas de conexión, asignación de roles y transmisión de metadatos 
*             relacionados con archivos en procesos distribuidos.
* 
* @Fecha de creación: 7 de noviembre de 2024.
* 
* @Última modificación: 4 de enero de 2025.
* 
************************************************/

#include "communication.h"

/*********************************************** 
* 
* @Finalidad: Crear y enviar una trama de conexión al servidor Gotham, incluyendo 
*             el tipo de worker, su IP y puerto. 
* 
* @Parámetros: 
* in: gotham_socket = Descriptor del socket conectado al servidor Gotham. 
* in: config = Puntero a la estructura `WorkerConfig` que contiene la configuración del worker. 
* 
* @Retorno: 
*           0 = La trama de conexión fue enviada con éxito. 
*          -1 = Error al asignar memoria o al enviar la trama. 
* 
************************************************/
int COMM_sendConnectionFrame(int gotham_socket, WorkerConfig *config) {
    char *data;
    // Format: TYPE: 0x02, DATA: <workerType>&<IP>&<Port>

    //creem la cadena amb el tipus de worker, IP i port dinàmics
    if (asprintf(&data, "%s&%s&%d", config->worker_type, config->worker_ip, config->worker_port) == -1) {
        return -1;  // Error allocating memory for data
    }

    //creem la trama de connexió
    Frame *connect_frame = FRAME_createFrame(0x02, (const char *)data, strlen(data));

    //enviem la trama de connexió a Gotham
    int result = FRAME_sendFrame(gotham_socket, connect_frame);

    //destruïm la trama 
    FRAME_destroyFrame(connect_frame);

    free(data);

    return result; // Retornar el resultat de l'enviament
}

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
int COMM_connectToGotham(int gotham_socket, WorkerConfig *config) {
    //enviem trama de connexió
    if (COMM_sendConnectionFrame(gotham_socket, config) < 0) {
        IO_printStatic(STDOUT_FILENO, RED "Error: failed to send Gotham the connection frame\n" RESET);
        return -1;  
    }

    //rebem resposta de gotham (OK o KO)
    FrameResult result = FRAME_receiveFrame(gotham_socket);
    if (result.error_code != FRAME_SUCCESS) {
        return -1;
    }

    Frame *frame = result.frame;

    //verifiquem la resposta de Gotham
    if (frame->type == 0x02) {
        if (frame->data_length == 0) {
            IO_printStatic(STDOUT_FILENO, GREEN "\nConnected to Mr. J. System.\n" RESET); //gotham ha retornat OK
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
        FRAME_destroyFrame(frame);  
        return -1; 
    }

    //destruïm trama
    FRAME_destroyFrame(frame);
    return 0;  
}

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
int COMM_waitForMainWorkerAssignment(int gotham_socket, volatile int* exit_program) {
    int main_worker = 0;
    FrameResult result;

    while (!main_worker && !(*exit_program)) { 
        // Rebem trames de Gotham esperant a que ens assignin com a worker principal
        result = FRAME_receiveFrame(gotham_socket);
        if (result.error_code != FRAME_SUCCESS) {
            if(result.frame) FRAME_destroyFrame(result.frame);
            if (result.error_code == FRAME_DISCONNECTED) {
                // Si Gotham cau o es desconnecta
                return COMM_GOTHAM_CRASHED;  
            } else if (result.error_code == FRAME_PENDING) {
                // Hem executat handler de SIGINT i ha tancat socket Gotham
                return COMM_SIGINT_RECEIVED;
            }
            continue;  // Continuem amb el bucle per altres errors relacionats amb trames
        }

        // Processem la trama rebuda
        Frame* frame = result.frame;
        if (frame->type == 0x08) {
            IO_printStatic(1, YELLOW "Received assignment to be the main worker\n" RESET);
            main_worker = 1;  
            FRAME_destroyFrame(frame);  
            return COMM_ASSIGNED_MAIN_WORKER;  
        }

        // Destruïm trama després de processar qualsevol altre tipus
        FRAME_destroyFrame(frame);
    }

    return COMM_PENDING;
}

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
int COMM_retrieveFileMetadata(int fleck_socket, DistortionContext* distortion_context, char* distortions_folder_path, int* shm_id) {
    // Atributs a extreure del camp de dades de la trama
    char *username = NULL, *filename = NULL, *md5sum = NULL;
    int filesize = 0, factor = 0;
    char* data_buffer = NULL; 
    // 1- Rebem la trama de fleck
    FrameResult result = FRAME_receiveFrame(fleck_socket);

    // Si hi ha error en deserialitzar la trama retornem codi d'error (si caigués el fleck en aquest punt es consideraria error, per exemple si fallés el seu thread)
    if (result.error_code != FRAME_SUCCESS) {
        if(result.error_code == FRAME_DISCONNECTED) IO_printStatic(STDOUT_FILENO, BLUE "Fleck disconnected abruptly. Closing the connection...\n" RESET); // Si falla el thread de fleck abortem distorsió 
        if (result.frame) FRAME_destroyFrame(result.frame);
        return 0; // Retornem codi d'error
    }

    Frame *response_frame = result.frame;
    if(response_frame->type == 0x03) {
        data_buffer = strdup((char *)response_frame->data);  // Copiem les dades de la trama a buffer auxiliar
        FRAME_destroyFrame(response_frame);
        if (!data_buffer) {
            IO_printStatic(STDOUT_FILENO, "Error: Could not allocate memory for response.\n");
            return 0; // Retornem codi d'error
        }

        // 2- Extreiem i validem atributs
        int valid_attributes = CONTEXT_extractAndValidateMetadata(data_buffer, &username, &filename, &filesize, &md5sum, &factor);

        // 3- Enviem check_ok o check_ko al fleck
        if(!valid_attributes) {
            COMM_sendConnectionResponse(fleck_socket, "CON_KO" , 0, 0x03);  // KO si els atributs no són vàlids
            free(data_buffer);
            return 0;
        }
        
        // Si les metadades rebudes són vàlides responem amb un CHECK_OK
        COMM_sendConnectionResponse(fleck_socket, NULL, 1, 0x03);  //OK

        // Inicialitzem les metadades del context de la distorsió. 
        CONTEXT_initContextMetadata(distortion_context, filename, username, md5sum, filesize, factor, distortions_folder_path);

        // 4- Creem o recuperem el progrés de la distorsió
        int fetch_successfull = CONTEXT_fetchDistortionContext(distortion_context, filename, shm_id);
        free(data_buffer); 

        if(!fetch_successfull) {
            IO_printStatic(STDOUT_FILENO, RED "ERROR: Failed to fetch distortion context\n" RESET); 
            return 0; 
        }

        return 1; // Procés executat satisfactòriament 
    }

    FRAME_destroyFrame(response_frame);
    return 0; 
}

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
int COMM_sendFleckFileMetadata(DistortionContext context, int fleck_socket, pthread_mutex_t* print_mutex) {
    char* data = NULL;
    int success = 1; 

    if(asprintf(&data, "%d&%s", context.filesize, context.md5sum) < 0) return 0; 

    Frame *metadata_frame = FRAME_createFrame(0x04, data, strlen(data)); 
    if(FRAME_sendFrame(fleck_socket, metadata_frame) < 0) {
        STRING_printF(print_mutex, STDOUT_FILENO, RED, "ERROR: failed to send distorted file's metadata\n");
        success = 0;
    }
    FRAME_destroyFrame(metadata_frame);
    free(data);

    STRING_printF(print_mutex, STDOUT_FILENO, MAGENTA, "Sent fleck distorted file's metadada\n");
    return success ? TRANSFER_SUCCESS : UNEXPECTED_ERROR;
}  

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
void COMM_handleFleckDisconnection(int fleck_socket, pthread_mutex_t* print_mutex) {
    char* data = NULL;

    FrameResult result = FRAME_receiveFrame(fleck_socket);
    if (result.error_code != FRAME_SUCCESS) {
        if (result.frame) FRAME_destroyFrame(result.frame);
        return;
    }

    Frame *response_frame = result.frame;
    if (response_frame->type == 0x07) {
        data = (char*) response_frame->data; 
        STRING_printF(print_mutex, STDOUT_FILENO, GREEN, "Fleck %s disconnected gracefully\n", data);
        FRAME_destroyFrame(response_frame);
        return;  
    }

    FRAME_destroyFrame(response_frame);
    return;
}

