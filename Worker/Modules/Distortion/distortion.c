/***********************************************
*
* @Autores: Alexandre Contreras, Armand López.
*
* @Finalidad: Implementar funciones relacionadas con el manejo de distorsión de archivos, 
*             incluyendo la gestión de procesos de distorsión en hilos dedicados, 
*             el cálculo de hash MD5, el envío y recepción de datos, y la sincronización 
*             con memoria compartida.
*
* @Fecha de creación: 15 de octubre de 2024.
*
* @Última modificación: 4 de enero de 2025.
*
************************************************/

#include "distortion.h"

/*********************************************** 
* 
* @Finalidad: Inicializar y asignar memoria para una estructura `DistortionThreadArgsW`, 
*             configurando los argumentos necesarios para manejar un hilo de distorsión. 
* 
* @Parámetros: 
* in: server = Puntero a la estructura `WorkerServer` asociada al servidor de workers. 
* in: distortions_folder_path = Ruta a la carpeta de distorsiones donde se procesará el archivo. 
* in: exit_distortion = Puntero a una bandera `volatile int` que indica si se debe interrumpir la distorsión. 
* in: sWorkerCountMutex = Puntero al semáforo que actúa como mutex para el contador global de workers. 
* in: file_type = Tipo de archivo que se procesará (e.g., `'t'` para texto o `'m'` para multimedia). 
* in: client_socket = Descriptor del socket del cliente asociado a la distorsión. 
* in: print_mutex = Puntero al mutex utilizado para sincronizar los mensajes de impresión. 
* 
* @Retorno: 
*           Puntero a la estructura `DistortionThreadArgsW` inicializada. 
*           Retorna NULL si ocurre un error al asignar memoria. 
* 
************************************************/
DistortionThreadArgsW* DIST_initDistortionArgs(WorkerServer* server, char* distortions_folder_path, volatile int* exit_distortion, semaphore* sWorkerCountMutex, char file_type, int client_socket, pthread_mutex_t* print_mutex) {
    DistortionThreadArgsW* args = (DistortionThreadArgsW*)malloc(sizeof(DistortionThreadArgsW));
    if (args == NULL) return NULL;
    args->client_socket = client_socket;
    args->server = server;
    args->exit_distortion = exit_distortion; 
    args->distortions_folder_path = distortions_folder_path;
    args->sWorkerCountMutex = sWorkerCountMutex;
    args->file_type = file_type; 
    args->print_mutex = print_mutex;

    return args;
}

/*********************************************** 
* 
* @Finalidad: Realizar una distorsión de texto copiando palabras desde un archivo original 
*             a un archivo temporal si cumplen con un umbral mínimo de longitud. 
* 
* @Parámetros: 
* in: original_file = Ruta al archivo original que será procesado. 
* in: tmp_file = Ruta al archivo temporal donde se escribirán las palabras que cumplan el umbral. 
* in: threshold = Longitud mínima de las palabras para ser incluidas en el archivo temporal. 
* 
* @Retorno: 
*           DISTORTION_SUCCESSFUL = La distorsión se realizó correctamente. 
*           DISTORTION_FAILED = Error durante el proceso de distorsión (e.g., lectura/escritura o asignación de memoria). 
* 
************************************************/
int DIST_SOdistortText(char *original_file, char *tmp_file, int threshold) {
    int fd_original = open(original_file, O_RDONLY);
    if (fd_original < 0) {
        perror("Error opening original file");
        return DISTORTION_FAILED;
    }

    int fd_tmp = open(tmp_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd_tmp < 0) {
        perror("Error opening temporary file");
        close(fd_original);
        return DISTORTION_FAILED;
    }

    // Espacio dinámico para palabras
    char *word = malloc(1); // Capacidad inicial para una palabra es 1
    if (!word) {
        perror("Error allocating memory for word");
        close(fd_original);
        close(fd_tmp);
        return DISTORTION_FAILED;
    }
    int word_capacity = 1;
    int word_length = 0;

    char current_char;
    int bytes_read;

    // Leer carácter a carácter
    while ((bytes_read = read(fd_original, &current_char, 1)) > 0) {
        if (isspace(current_char) || ispunct(current_char) || (unsigned char)current_char == 0xE2) {
            // Final de una palabra
            if (word_length >= threshold) {
                write(fd_tmp, word, word_length);
                write(fd_tmp, " ", 1); // Añadir espacio después de la palabra
            }
            word_length = 0; // Reiniciar la longitud de la palabra
        } else {
            // Expandir el espacio dinámico si es necesario
            if (word_length + 1 >= word_capacity) {
                word_capacity += 1; // Incrementar capacidad en 1 byte
                char *new_word = realloc(word, word_capacity);
                if (!new_word) {
                    perror("Error reallocating memory for word");
                    free(word);
                    close(fd_original);
                    close(fd_tmp);
                    return DISTORTION_FAILED;
                }
                word = new_word;
            }
            // Agregar carácter a la palabra
            word[word_length++] = current_char;
        }
    }

    // Procesar la última palabra si no terminó con un espacio
    if (word_length >= threshold) {
        write(fd_tmp, word, word_length);
        write(fd_tmp, " ", 1);
    }

    // Liberar memoria y cerrar archivos
    free(word);
    close(fd_original);
    close(fd_tmp);

    return DISTORTION_SUCCESSFUL;
}

/*********************************************** 
* 
* @Finalidad: Distorsionar un archivo de texto, copiando palabras desde un archivo original 
*             a un archivo temporal si cumplen con un umbral mínimo de longitud. 
* 
* @Parámetros: 
* in: original_file = Ruta al archivo original que será procesado. 
* in: tmp_file = Ruta al archivo temporal donde se escribirán las palabras que cumplan el umbral. 
* in: threshold = Longitud mínima de las palabras para ser incluidas en el archivo temporal. 
* 
* @Retorno: 
*           DISTORTION_SUCCESSFUL = La distorsión se realizó correctamente. 
*           DISTORTION_FAILED = Error al abrir los archivos originales o temporales. 
* 
************************************************/
int DIST_distortText(char* original_file, char* tmp_file, int threshold) {
    int fd_original = open(original_file, O_RDONLY);
    if (fd_original < 0) {
        return DISTORTION_FAILED;
    }

    int fd_tmp = open(tmp_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd_tmp < 0) {
        close(fd_original);
        return DISTORTION_FAILED;
    }

    // Todo: extreure cada paraula del fitxer original i escriure-la al fitxer temporal si la seva llargada és superior al llindar
    DIST_SOdistortText(original_file, tmp_file, threshold);

    close(fd_original);
    close(fd_tmp);

    return DISTORTION_SUCCESSFUL; 
}

/*********************************************** 
* 
* @Finalidad: Distorsionar un archivo de acuerdo a su tipo (texto, audio o imagen), 
*             aplicando un factor específico y reemplazando el archivo original 
*             con el contenido distorsionado. 
* 
* @Parámetros: 
* in/out: context = Puntero a la estructura `sDistortionContext` que contiene los metadatos del archivo. 
* in: folder_path = Ruta a la carpeta donde se almacenará el archivo temporal. 
* in: print_mutex = Mutex para sincronizar los mensajes de impresión. 
* 
* @Retorno: 
*           DISTORTION_SUCCESSFUL = La distorsión se realizó correctamente. 
*           DISTORTION_FAILED = Error en alguna de las etapas del proceso de distorsión. 
* 
************************************************/
int DIST_distortFile(DistortionContext context, char* folder_path, pthread_mutex_t* print_mutex) {
    char* tmp_file = NULL;
    char* extension = NULL;
    int distortion_result = DISTORTION_SUCCESSFUL; 
    
    // Creem un path pel fitxer temporal
    if (asprintf(&tmp_file, ".%s%s_tmp%s", folder_path, context.username, context.filename) < 0) {
        return DISTORTION_FAILED;
    }

    // Copy the original file to the temporary file
    if (FILE_copyFile(context.file_path, tmp_file) < 0) {
        free(tmp_file);
        return DISTORTION_FAILED;
    }
    extension = FILE_getFileExtension(context.filename);
    if(!strcmp(extension, "txt")) {
        STRING_printF(print_mutex, STDOUT_FILENO, YELLOW, "Distorting text file...\n");
        distortion_result = DIST_distortText(context.file_path, tmp_file, context.factor);
    }
    else if(!strcmp(extension, "wav")) {
        STRING_printF(print_mutex, STDOUT_FILENO, YELLOW, "Distorting audio track...\n");
        distortion_result = SO_compressAudio(tmp_file, context.factor) == NO_ERROR ? DISTORTION_SUCCESSFUL : DISTORTION_FAILED;
    }
    else {
        STRING_printF(print_mutex, STDOUT_FILENO, YELLOW, "Distorting image...\n");
        distortion_result = SO_compressImage(tmp_file, context.factor) == NO_ERROR ? DISTORTION_SUCCESSFUL : DISTORTION_FAILED;
    }

    if (distortion_result == DISTORTION_FAILED) {
        SO_deleteImage(tmp_file);
        free(tmp_file);
        STRING_printF(print_mutex, STDOUT_FILENO, RED, "ERROR: failed to distort file\n");
        return distortion_result;  
    }

    // Reemplacem el contingut del fitxer original amb el del fitxer temporal
    if (FILE_replaceFile(tmp_file, context.file_path) < 0) {
        SO_deleteImage(tmp_file);
        free(tmp_file);
        return DISTORTION_FAILED;
    }

    SO_deleteImage(tmp_file);
    free(tmp_file);
    STRING_printF(print_mutex, STDOUT_FILENO, GREEN, "Compression successful\n");
    return DISTORTION_SUCCESSFUL;
}

/*********************************************** 
* 
* @Finalidad: Configurar el contexto de distorsión actualizando el tamaño del archivo, 
*             el hash MD5 y el progreso de la distorsión. 
* 
* @Parámetros: 
* in/out: context = Puntero a la estructura `sDistortionContext` que será configurada. 
* 
* @Retorno: 
*           1 = El contexto fue configurado correctamente. 
*           0 = Error durante la configuración del contexto. 
* 
************************************************/
int DIST_setupDistortionContext(DistortionContext* context) {    
    context->filesize = FILE_getFileSize(context->file_path);
    if (context->filesize < 0) return 0; 

    freePointer((void**)&(context->md5sum)); // Alliberem l'md5sum del fitxer original
    context->md5sum = FILE_calculateMD5(context->file_path); // Assignem l'md5sum del fitxer distorsionat
    if (!context->md5sum) return 0;

    context->n_packets = context->filesize / DATA_SIZE;
    if (context->filesize % DATA_SIZE != 0) {
        (context->n_packets)++;
    }

    context->n_processed_packets = 0;

    return 1;
}

/*********************************************** 
* 
* @Finalidad: Manejar el proceso completo de distorsión de un archivo en un hilo dedicado, 
*             gestionando las distintas etapas como recepción, verificación de integridad, 
*             distorsión, y envío del archivo distorsionado. 
* 
* @Parámetros: 
* in: args = Puntero a la estructura `DistortionThreadArgsW` que contiene los argumentos 
*            necesarios para gestionar el hilo de distorsión. 
* 
* @Retorno: 
*           NULL = El hilo finaliza su ejecución tras completar el proceso de distorsión 
*                  o ante un error crítico. 
* 
************************************************/
void* DIST_handleFileDistortion(void* args) {
    DistortionThreadArgsW* thread_args = (DistortionThreadArgsW*)args;
    int client_socket = thread_args->client_socket;
    WorkerServer* server = thread_args->server;
    volatile int* exit_distortion = thread_args->exit_distortion;
    semaphore* sWorkerCountMutex = thread_args->sWorkerCountMutex;  // Semàfor per a aplicar exclusió mutua en accedir al comptador global de harleys/enigmes

    DistortionContext distortion_context = CONTEXT_initializeContext();   // Estructura de context de distorsió que emmagatzemarà el progrés de la distorsió de manera que si cau el worker principal, el worker que prengui el relleu la pugui resumir
    int shm_id = 0;                                                       // Identificador associat a la regió de memòria compartida on es troba el context de la distorsió
    int finished_distortion = 0;                                          // Flag per a sortir del bucle de distorsió

    // 1- Rebem metadades del fitxer a distorsionar i, a partir d'aquestes, recuperem o creem el context de distorsió
    int stage_successfull = COMM_retrieveFileMetadata(client_socket, &distortion_context, thread_args->distortions_folder_path, &shm_id);
    if(!stage_successfull) goto exit_thread;

    // Iniciem o resumim la distorsió a partir de la fase indicada a l'estructura de context. Implementem un bucle per a poder llegir la flag "exit_distorsion" cada vegada que completem una fase. 
    while(!*(exit_distortion) && !finished_distortion) {
        switch(distortion_context.current_stage) {
            case STAGE_RECV_FILE: 
                // 2- Rebem el fitxer a distorsionar
                int recv_result = COMM_receiveFile(distortion_context.file_path, distortion_context.filename, distortion_context.n_packets, &(distortion_context.n_processed_packets), client_socket, exit_distortion, WORKER, thread_args->print_mutex);
                if(recv_result != TRANSFER_SUCCESS) goto exit_thread; // Tant si cau fleck com si hi ha error inesperat abortem distorsió
                
                distortion_context.current_stage = STAGE_CHECK_MD5; // Actualitzem estat de la distorsió a "comprovant md5"
            break; 
            case STAGE_CHECK_MD5:
                // 3- Comparem md5sum de les metadades amb md5sum del fitxer reconstruït. Enviem trama pertinent a fleck
                int verify_status = COMM_verifyFileIntegrity(distortion_context.file_path, distortion_context.md5sum, client_socket, thread_args->print_mutex);
                if(verify_status != TRANSFER_SUCCESS) goto exit_thread; // Tant si no coincideix l'md5 com si falla el send degut a un ctrl+c (tanca els sockets de clients) abortem distorsió. 

                distortion_context.current_stage = STAGE_DISTORT; // Actualitzem estat de la distorsió a "distorsionant"
            break;
            case STAGE_DISTORT:
                // 4- Distorsionem fitxer
                if(DIST_distortFile(distortion_context, thread_args->distortions_folder_path, thread_args->print_mutex) != DISTORTION_SUCCESSFUL) goto exit_thread;
                                                                                      
                distortion_context.current_stage = STAGE_SND_METADATA; // Actualitzem estat de la distorsió a "enviant metadades"
            break;
            case STAGE_SND_METADATA:
                // Una vegada la fase de processament del fitxer original ha estat completada, la informació que conté l'estrcutura de context ha de referenciar el fitxer distorionat
                int update_success = DIST_setupDistortionContext(&distortion_context); 
                if(update_success <= 0) goto exit_thread;
                // 5- Enviem metadades del fitxer distorsionat
                if(COMM_sendFleckFileMetadata(distortion_context, client_socket, thread_args->print_mutex) != TRANSFER_SUCCESS) goto exit_thread;

                distortion_context.current_stage = STAGE_SND_FILE; // Actualitzem estat de la distorsió a "enviant fitxer"
            break;
            case STAGE_SND_FILE:
                // 6- Enviem fitxer distorsionat a fleck i processem resposta de comprovació d'md5
                int snd_result = COMM_sendFile(distortion_context.file_path, distortion_context.filename, distortion_context.n_packets, &(distortion_context.n_processed_packets), client_socket, exit_distortion, WORKER, thread_args->print_mutex);
                if(snd_result != TRANSFER_SUCCESS) goto exit_thread;

                // Processem verificació de l'md5 del fleck
                int check_ok = COMM_retrieveMD5Check(client_socket, WORKER, thread_args->print_mutex);
                if(check_ok != TRANSFER_SUCCESS) goto exit_thread;
                distortion_context.current_stage = STAGE_FINISHED; // Actualitzem estat de la distorsió a "enviant fitxer"
            break; 
            case STAGE_FINISHED:
                COMM_handleFleckDisconnection(client_socket, thread_args->print_mutex); 
                finished_distortion = 1;
            break;
        }
    }
exit_thread:
    STRING_printF(thread_args->print_mutex, STDOUT_FILENO, YELLOW, "Exiting distortion thread...\n");

    MC_removeClient(server, client_socket); // Eliminem el socket del fleck de la llista de clients connectats
    EXIT_cleanupDistortionFiles(distortion_context, *exit_distortion, shm_id, sWorkerCountMutex, thread_args->file_type);
    EXIT_cleanupSharedMemory(distortion_context, shm_id, *exit_distortion, sWorkerCountMutex, thread_args->file_type);
    EXIT_cleanupDistortionContext(&distortion_context);  // Netegem l'estructura de context
    free(args); // Alliberem arguments del thread de distorsió
    return NULL;
}
