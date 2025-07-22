/*********************************************** 
* 
* @Autores: Alexandre Contreras, Armand López. 
* 
* @Finalidad: Implementar funciones relacionadas con el manejo de procesos de distorsión 
*             de archivos, incluyendo la inicialización de contextos, gestión de 
*             comunicaciones, ejecución de hilos, y registro de progreso en el sistema 
*             de distorsión. 
* 
* @Fecha de creación: 20 de octubre de 2024. 
* 
* @Última modificación: 4 de enero de 2025. 
* 
************************************************/

#include "distortion.h"

/*********************************************** 
* 
* @Finalidad: Actualizar el campo `file_path` de la estructura `DistortionContext` 
*             añadiendo el sufijo `_distorted`. 
* 
* @Parámetros: 
* in/out: context = Puntero a la estructura `DistortionContext` a actualizar. 
* 
* @Retorno: 
*           1 = Operación exitosa. 
*           0 = Error al asignar memoria. 
* 
************************************************/
int DIST_updateContextFilePath(DistortionContext* context) {
    char* distorted_filepath = NULL;

    // Construïm el path del fitxer distorsionat
    if(asprintf(&distorted_filepath, "%s_distorted", context->file_path) < 0) return 0;
    // Alliberem el path antic
    free(context->file_path);
    // Assignem el nou path
    context->file_path = distorted_filepath;
    return 1; 
}

/*********************************************** 
* 
* @Finalidad: Actualizar el estado de las distorsiones en el registro según el tipo 
*             de archivo y el resultado de la operación. 
* 
* @Parámetros: 
* in/out: distortion_record = Puntero al registro de distorsiones a actualizar. 
* in: finished = Indicador de finalización (1 para completado, 0 para fallido). 
* in: type = Tipo de archivo cuya distorsión se debe actualizar. 
* 
* @Retorno: Ninguno. 
* 
************************************************/
void DIST_updateDistortionRecord(DistortionRecord* distortion_record, int finished, int type) {
    for (int i = 0; i < distortion_record->n_distortions; i++) {
        if (distortion_record->distortions[i].status == ONGOING && distortion_record->distortions[i].file_type == type) {
            distortion_record->distortions[i].status = finished ? COMPLETED : FAILED;
        }
    }
}

/*********************************************** 
* 
* @Finalidad: Manejar el flujo completo de una operación de distorsión de archivos. 
*             Incluye el envío de metadatos y archivo, la recepción del archivo distorsionado, 
*             y la verificación de integridad, gestionando reconexiones en caso de fallos. 
* 
* @Parámetros: 
* in: arg = Puntero genérico que contiene los argumentos del hilo encapsulados en una 
*           estructura `DistortionThreadArgsF`. 
* 
* @Retorno: Siempre retorna `NULL`. 
* 
************************************************/
void* DIST_handleFileDistortion(void* arg) {
    DistortionThreadArgsF* distortion_args = (DistortionThreadArgsF*) arg; 

    DistortionContext* distortion_context = distortion_args->distortion_context; 
    int* distorting_flag = distortion_args->distorting_flag;
    int worker_socket = distortion_args->main_worker->socket; 
    char* worker_type = distortion_args->worker_type;
    MainWorker* main_worker = distortion_args->main_worker; 

    int gotham_socket = distortion_args->gotham_socket;
    volatile int* exit_distortion = distortion_args->exit_distortion; 
    int* finished_distortion = distortion_args->finished_distortion;

    distortion_context->current_stage = STAGE_SND_FILE;
    *distorting_flag = 1;
    *finished_distortion = 0;

enviaMetadades:
    // Fase 1: enviament al worker de les metadades del fitxer a distorsionar
    if (COMM_sendFileMetadata(worker_socket, distortion_context->username, distortion_context->filename, distortion_context->filesize, distortion_context->md5sum, distortion_context->factor, distortion_args->print_mutex) < 0) {
        goto exit_thread;
    }

    STRING_printF(distortion_args->print_mutex, STDOUT_FILENO, MAGENTA, "Sent worker original file's metadada\n");
    
    while(!*finished_distortion && !*exit_distortion) {
        switch(distortion_context->current_stage) {
            case STAGE_SND_FILE:
                // Fase 2: enviament del fitxer a distorsionar
                int send_result = COMM_sendFile(distortion_context->file_path, distortion_context->filename, distortion_context->n_packets, &distortion_context->n_processed_packets, worker_socket, exit_distortion, FLECK, distortion_args->print_mutex);
                if(send_result != TRANSFER_SUCCESS) {
                    if(send_result == UNEXPECTED_ERROR || send_result == INTERRUPTED_BY_SIGINT) goto exit_thread; 
                    // Si el worker ha caigut demanem a gotham el nou worker principal i ens intentem connectar a aquest
                    if (!COMM_reconnectToWorker(distortion_context->filename, worker_type, main_worker, gotham_socket, distortion_args->print_mutex)) goto exit_thread;
                    
                    goto enviaMetadades; // Connexió satisfactòria a un NOU worker principal
                }

                // Fase 3: worker compara md5sum de la trama de metadades amb el del fitxer reconstruït i ens envia CHECK_OK O CHECK_KO
                int check_ok = COMM_retrieveMD5Check(worker_socket, FLECK, distortion_args->print_mutex);
                if(check_ok != TRANSFER_SUCCESS) {
                    if(send_result == UNEXPECTED_ERROR || send_result == INTERRUPTED_BY_SIGINT) goto exit_thread; // Si hi ha error en rebre la trama/ worker retorna check_ko / ha hagut sigint abortem distorsió
                    if (!COMM_reconnectToWorker(distortion_context->filename, worker_type, main_worker, gotham_socket, distortion_args->print_mutex)) goto exit_thread;

                    goto enviaMetadades; 
                }
                distortion_context->current_stage = STAGE_RCV_METADATA;
            break; 
            case STAGE_RCV_METADATA:
                // Fase 4: recepció de les metadades del fitxer distorsionat i actualització de l'estructura de context
                int frame_error = COMM_retrieveFileMetadata(worker_socket, distortion_context, distortion_args->print_mutex); 
                if(frame_error != TRANSFER_SUCCESS) {
                    if(frame_error == UNEXPECTED_ERROR) goto exit_thread; // Si hi ha error en rebre la trama abortem distorsió
                    if (!COMM_reconnectToWorker(distortion_context->filename, worker_type, main_worker, gotham_socket, distortion_args->print_mutex)) goto exit_thread;

                    goto enviaMetadades;
                }

                // Una vegada l'enviament del fitxer ha estat completat satisfactòriament, cal adaptar l'estructura de context per a que el filepath apunti al fitxer on reconstruirem el l'arxiu distorsionat 
                if(!DIST_updateContextFilePath(distortion_context)) goto exit_thread; 
                distortion_context->current_stage = STAGE_RECV_FILE;
            break; 
            case STAGE_RECV_FILE:
                // Fase 5: recepció del fitxer distorsionat
                int rcv_result = COMM_receiveFile(distortion_context->file_path, distortion_context->filename, distortion_context->n_packets, &distortion_context->n_processed_packets, worker_socket, exit_distortion, FLECK, distortion_args->print_mutex);
                if(rcv_result != TRANSFER_SUCCESS) {
                    if(send_result == UNEXPECTED_ERROR || send_result == INTERRUPTED_BY_SIGINT) goto exit_thread; // Si hi ha hagut error inesperat en la rececpió del fitxer abortem distorsió
                    if (!COMM_reconnectToWorker(distortion_context->filename, worker_type, main_worker, gotham_socket, distortion_args->print_mutex)) goto exit_thread;
                    
                    goto enviaMetadades; // Connexió satisfactòria a un NOU worker principal
                }

                // Fase 6: comprovació del md5sum i notificació pertinent al worker
                int verify_status = COMM_verifyFileIntegrity(distortion_context->file_path, distortion_context->md5sum, worker_socket, distortion_args->print_mutex);
                if(verify_status != TRANSFER_SUCCESS) goto exit_thread;
                distortion_context->current_stage = STAGE_DISCONNECT;
            break;
            case STAGE_DISCONNECT:
                // Fase 7: desconnexió 
                COMM_disconnectFromServer(worker_socket, distortion_context->username, COMM_WORKER); 
                *finished_distortion = 1;
            break; 
        }
    }

exit_thread:
    SOCKET_closeSocket(&worker_socket);
    DIST_updateDistortionRecord(distortion_args->distortion_record, *finished_distortion, !strcmp(worker_type, "Text") ? TEXT : MEDIA);
    EXIT_cleanupDistortionContext(&distortion_context); 
    *distorting_flag = 0;
    *finished_distortion = 1;
    STRING_printF(distortion_args->print_mutex, STDOUT_FILENO, YELLOW, "Exiting distortion thread...\n");
    free(distortion_args);
    return NULL;
}

/*********************************************** 
* 
* @Finalidad: Inicializar y asignar memoria para una estructura `DistortionThreadArgsF` 
*             con los parámetros necesarios para manejar un hilo de distorsión. 
* 
* @Parámetros: 
* in: type = Tipo de distorsión solicitada (e.g., "Text" o "Media"). 
* in: context = Puntero a la estructura `DistortionContext` que contiene el contexto de la distorsión. 
* in: distorting_flag = Puntero a una bandera que indica si la distorsión está en curso. 
* in: main_worker = Puntero al `MainWorker` asociado. 
* in: gotham_socket = Descriptor del socket conectado al servidor Gotham. 
* in: folder_path = Ruta al directorio donde se procesará el archivo distorsionado. 
* in: username = Nombre del usuario que solicita la distorsión. 
* in: distortion_record = Puntero al registro de distorsiones para registrar el progreso. 
* in: exit = Puntero a una bandera de tipo `volatile int` que indica si se debe salir del hilo. 
* in: print_mutex = Mutex para garantizar la exclusión mutua al imprimir mensajes. 
* in: finished_distortion = Puntero a flag que indica si el thread de distorsión ha finalizado. 
*
* @Retorno: Retorna un puntero a la estructura `DistortionThreadArgsF` inicializada 
*           o `NULL` si no se pudo asignar memoria. 
* 
************************************************/
DistortionThreadArgsF* DIST_initDistortionThreadArgs(char* type, DistortionContext *context, int *distorting_flag, MainWorker *main_worker, int gotham_socket, char* folder_path, DistortionRecord* distortion_record, volatile int* exit, int* finished_distortion, pthread_mutex_t* print_mutex) {
    DistortionThreadArgsF* distortion_args = (DistortionThreadArgsF*) malloc (sizeof(DistortionThreadArgsF)); 
    if(!distortion_args) return NULL;

    distortion_args->distortion_context = context;
    distortion_args->distorting_flag = distorting_flag;
    distortion_args->main_worker = main_worker; 
    distortion_args->worker_type = type;
    distortion_args->gotham_socket = gotham_socket;
    distortion_args->folder_path = folder_path;
    distortion_args->distortion_record = distortion_record;
    distortion_args->exit_distortion = exit;
    distortion_args->print_mutex = print_mutex;
    distortion_args->finished_distortion = finished_distortion;
    return distortion_args;
}

/*********************************************** 
* 
* @Finalidad: Configurar e inicializar la estructura `DistortionContext` con la información 
*             necesaria para realizar una distorsión, incluyendo el archivo, tamaño, 
*             hash MD5, y número de paquetes. 
* 
* @Parámetros: 
* in/out: context = Puntero a la estructura `DistortionContext` que se va a configurar. 
* in: folder_path = Ruta al directorio que contiene el archivo. 
* in: filename = Nombre del archivo que se va a distorsionar. 
* in: factor = Factor de distorsión aplicado al archivo. 
* 
* @Retorno: Retorna un entero que indica el estado de la operación:
*           1 = Configuración completada con éxito. 
*           0 = Error al calcular el tamaño del archivo o el hash MD5. 
*          -1 = Error al asignar memoria o construir la ruta del archivo. 
* 
************************************************/
int DIST_setupDistortionContext(DistortionContext *context, char *folder_path, char *filename, char* username, int factor) {    
    context->file_path = FILE_buildPrivateFilePath(folder_path, filename, NULL);
    if(context->file_path == NULL) return -1;

    context->filename = strdup(filename);
    if (!context->filename) return -1; 

    context->factor = factor;

    context->filesize = FILE_getFileSize(context->file_path);
    if (context->filesize < 0) return 0; 

    context->md5sum = FILE_calculateMD5(context->file_path);
    if (!context->md5sum) return 0; 

    context->username = strdup(username);
    if (!context->username) return 0;

    context->n_packets = context->filesize / DATA_SIZE;
    if (context->filesize % DATA_SIZE != 0) {
        context->n_packets++;
    }

    context->n_processed_packets = 0;

    return 1;
}

/*********************************************** 
* 
* @Finalidad: Registrar una nueva distorsión en el registro de distorsiones, 
*             almacenando el nombre del archivo, el tipo y el estado inicial. 
* 
* @Parámetros: 
* in/out: distortion_record = Puntero al registro de distorsiones donde se añadirá la nueva entrada. 
* in: filename = Nombre del archivo asociado a la distorsión. 
* in: type = Tipo de archivo a distorsionar ("Text" o "Media"). 
* 
* @Retorno: Ninguno. 
* 
************************************************/
void DIST_recordDistortion(DistortionRecord* distortion_record, char* filename, char* type) {
    distortion_record->distortions = realloc(distortion_record->distortions, (distortion_record->n_distortions + 1) * sizeof(CheckStatus));
    if (!distortion_record->distortions) return;
    distortion_record->distortions[distortion_record->n_distortions].filename = strdup(filename);
    distortion_record->distortions[distortion_record->n_distortions].file_type = !strcmp(type, "Text") ? TEXT : MEDIA;
    distortion_record->distortions[distortion_record->n_distortions].status = ONGOING;
    distortion_record->n_distortions++;
}

/*********************************************** 
* 
* @Finalidad: Preparar el contexto, solicitar un worker, y lanzar un hilo para iniciar 
*             el proceso de distorsión de un archivo. 
* 
* @Parámetros: 
* in/out: context = Puntero a la estructura `DistortionContext` donde se almacenará la información del proceso. 
* in: filename = Nombre del archivo a distorsionar. 
* in: username = Nombre del usuario que solicita la distorsión. 
* in: type = Tipo de distorsión a realizar ("Text" o "Media"). 
* in/out: thread = Puntero al identificador del hilo que manejará la distorsión. 
* in: factor = Factor de distorsión aplicado al archivo. 
* in/out: distorting_flag = Bandera que indica si hay un proceso de distorsión en curso. 
* in: main_worker = Puntero a la estructura `MainWorker` para manejar la conexión con el worker. 
* in: gotham_socket = Descriptor del socket conectado al servidor Gotham. 
* in: folder_path = Ruta al directorio donde se encuentra el archivo. 
* in/out: distortion_record = Puntero al registro de distorsiones donde se añadirá la nueva entrada. 
* in: exit_distortion = Bandera para indicar si se debe salir del proceso de distorsión. 
* in: print_mutex = Mutex para garantizar la exclusión mutua al imprimir mensajes de estado o error. 
* 
* @Retorno: Retorna un entero que indica el estado de la operación:
*           1 = Proceso de distorsión iniciado con éxito. 
*           0 = Error en alguna etapa del proceso (e.g., preparación del contexto, conexión, o creación del hilo). 
* 
************************************************/
int DIST_prepareAndStartDistortion (DistortionContext *context, char *filename, char* username, char *type, pthread_t *thread, int factor, int* distorting_flag, MainWorker* main_worker, int gotham_socket, char* folder_path, DistortionRecord* distortion_record, volatile int* exit_distortion, int* finished_distortion, pthread_mutex_t *print_mutex) {
    // Validem si ja hi ha una distorsió del tipus sol·licitat en curs
    if (*distorting_flag) {
        STRING_printF(print_mutex, STDOUT_FILENO, RED, "Error: %s distortion already in progress\n", type);
        return 0;  // Cas fallit
    }

    // Alliberem l'anterior thread de distorsió del tipus sol·licitat
    int join_result = pthread_join(*thread, NULL);
    if(join_result != 0) {
        if(join_result != ESRCH) { // Si no existeix el thread significa que és la primera vegada que el volem llançar. Si l'error és diferent a ESRCH avortem la distorsió
            STRING_printF(print_mutex, STDOUT_FILENO, RED, "Error joining distortion thread: %s\n", strerror(join_result));
            return 0;
        }
    }

    // Preparem l'estructura de context
    int ok = DIST_setupDistortionContext(context, folder_path, filename, username, factor);
    if (ok <= 0) {
        if (ok == 0) {
            EXIT_cleanupDistortionContext(&context); 
        }
        return 0;  // Cas fallit
    }

    // Demanen un worker a gotham i ens intentem connectar a aquest
    if (COMM_requestWorkerAndEstablishConnection(context->filename, type, main_worker, gotham_socket, CONNECTION, print_mutex) == FAILED_TO_CONNECT) {
        EXIT_cleanupDistortionContext(&context); 
        return 0;  // Cas fallit
    }

    DistortionThreadArgsF* distortion_args = DIST_initDistortionThreadArgs(type, context, distorting_flag, main_worker, gotham_socket, folder_path, distortion_record, exit_distortion, finished_distortion, print_mutex);

    // Llancem el thread de distorsió
    if (pthread_create(thread, NULL, DIST_handleFileDistortion, distortion_args) != 0) {
        STRING_printF(print_mutex, STDOUT_FILENO, RED, "Failed to create %s distortion thread\n", type);
        SOCKET_closeSocket(&main_worker->socket);
        EXIT_cleanupDistortionContext(&context); 
        free(distortion_args);
        return 0;  // Cas fallit
    }

    STRING_printF(print_mutex, STDOUT_FILENO, YELLOW, "\n%s distortion process successfully started\n", type);
    
    // Registrem la distorsió iniciada a l'estructura d'historial
    DIST_recordDistortion(distortion_record, context->filename, type); 
    
    return 1;  // Cas exitós
}