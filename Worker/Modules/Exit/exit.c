
/***********************************************
*
* @Autores: Alexandre Contreras, Armand López.
*
* @Finalidad: Implementar funciones para gestionar la limpieza de recursos y memoria 
*             utilizados en el sistema de distorsión de archivos. Incluye la liberación 
*             de memoria dinámica, cierre de sockets, y manejo de memoria compartida 
*             asociada a procesos de distorsión.
*
* @Fecha de creación: 23 de octubre de 2024.
*
* @Última modificación: 4 de enero de 2025.
*
************************************************/

#include "exit.h"

/*********************************************** 
* 
* @Finalidad: Liberar la memoria dinámica asociada a las estructuras `WorkerConfig` y 
*             `WorkerServer`, incluyendo sus campos internos y mutexes. 
* 
* @Parámetros: 
* in/out: worker = Doble puntero a la estructura `WorkerConfig` que se liberará. 
* in/out: server = Doble puntero a la estructura `WorkerServer` que se liberará. 
* 
* @Retorno: Ninguno. 
* 
************************************************/
void EXIT_freeMemory(WorkerConfig** worker, WorkerServer** server) {
    if (worker != NULL && *worker != NULL) {
        freePointer((void**)&((*worker)->gotham_ip));
        freePointer((void**)&((*worker)->worker_ip));
        freePointer((void**)&((*worker)->folder_path));
        freePointer((void**)&((*worker)->worker_type));
        freePointer((void**)worker);  
    }
    
    if (server != NULL && *server != NULL) {
        freePointer((void**)&((*server)->clients));
        freePointer((void**)&((*server)->active_threads));

        pthread_mutex_destroy(&(*server)->clients_mutex);
        pthread_mutex_destroy(&(*server)->thread_list_mutex);

        freePointer((void**)server);
    }
}

/*********************************************** 
* 
* @Finalidad: Esperar a que terminen todos los hilos activos almacenados en un array, 
*             utilizando un mutex para sincronizar el acceso al array. 
* 
* @Parámetros: 
* in: threads = Puntero al array de identificadores de hilos (`pthread_t`). 
* in: thread_count = Número de hilos activos en el array. 
* in: thread_list_mutex = Puntero al mutex utilizado para sincronizar el acceso al array de hilos. 
* 
* @Retorno: Ninguno. 
* 
************************************************/
void EXIT_joinActiveThreads(pthread_t *threads, int thread_count, pthread_mutex_t *thread_list_mutex) {
    if (threads != NULL) {
        pthread_mutex_lock(thread_list_mutex);
        for (int i = 0; i < thread_count; i++) {
            pthread_join(threads[i], NULL);
        }
        pthread_mutex_unlock(thread_list_mutex);
    }
}

/*********************************************** 
* 
* @Finalidad: Realizar la limpieza de los recursos asociados al worker principal, 
*             cerrando el socket de escucha y esperando la finalización de los hilos 
*             activos de distorsión. 
* 
* @Parámetros: 
* in/out: server = Puntero a la estructura `WorkerServer` que contiene los recursos a limpiar. 
* 
* @Retorno: Ninguno. 
* 
************************************************/
void EXIT_cleanupMainWorker(WorkerServer* server) {
    if (server != NULL) {
        if (server->listen_socket != -1) {
            close(server->listen_socket);
            server->listen_socket = -1;
        }

        IO_printStatic(STDOUT_FILENO, PURPLE "Waiting for active distortion threads to finish...\n" RESET);
        EXIT_joinActiveThreads(server->active_threads, server->active_thread_count, &server->thread_list_mutex); // Tanquem sockets de clients en terminar els threads per a no interrommpre cap fase del procés de distorsió
        IO_printStatic(STDOUT_FILENO, PURPLE "Distortion threads successfully terminated\n" RESET);
    }
}

/*********************************************** 
* 
* @Finalidad: Verificar si el worker actual es el último de su tipo conectado al sistema, 
*             utilizando un contador almacenado en memoria compartida. 
* 
* @Parámetros: 
* in: sMutex = Puntero al semáforo utilizado como mutex para sincronizar el acceso al contador. 
* in: type = Tipo de worker (`ENIGMA_COUNTER` para enigmas o `HARLEY_COUNTER` para harleys). 
* 
* @Retorno: 
*           1 = El worker es el último de su tipo conectado al sistema. 
*           0 = Hay más workers de este tipo conectados al sistema. 
*          -1 = Error al interactuar con la memoria compartida o el semáforo. 
* 
************************************************/
int EXIT_isLastWorker(semaphore* sMutex, int type) {
    int is_last_worker = 0;

    key_t key = ftok("../../Gotham/config.dat", type);  // Generem clau diferent segons si el comptador referencia enigmes (type=2) o harleys (type=3)
    if (key == -1) return -1;

    int shm_id = shmget(key, sizeof(int), IPC_CREAT | 0666);  // Intentem crear la regió de memòria

    int *workerCount = (int *)shmat(shm_id, NULL, 0);
    if (workerCount == (void *)-1) return -1;

    SEM_wait(sMutex);
    if(*workerCount == 1) is_last_worker = 1;
    SEM_signal(sMutex);

    if (shmdt(workerCount) == -1) return -1;

    return is_last_worker; 
}

/*********************************************** 
* 
* @Finalidad: Gestionar los archivos generados durante el proceso de distorsión, moviéndolos 
*             al directorio global en caso de una señal SIGINT o eliminándolos si la 
*             distorsión se completó o fue abortada. 
* 
* @Parámetros: 
* in: context = Estructura `DistortionContext` que contiene la información del archivo a procesar. 
* in: sigint_flag = Bandera que indica si la limpieza fue causada por una señal SIGINT. 
* in: shm_id = Identificador de la memoria compartida asociada al proceso de distorsión. 
* in: sWorkerCountMutex = Puntero al semáforo utilizado para sincronizar el acceso al contador de workers. 
* in: file_type = Tipo de archivo (`'t'` para texto o `'m'` para multimedia). 
* 
* @Retorno: Ninguno. 
* 
************************************************/
void EXIT_cleanupDistortionFiles(DistortionContext context, int sigint_flag, int shm_id, semaphore* sWorkerCountMutex, char file_type) {
    // Si hem arribat a aquest punt per una senyal sigint, movem el fitxer de distorsió al directori global
    if(sigint_flag && !EXIT_isLastWorker(sWorkerCountMutex, file_type == 't' ? ENIGMA_COUNTER : HARLEY_COUNTER)) {
        if(!DIR_moveFileToSharedFolder(context.filename, context.username, context.file_path)) {
            shmctl(shm_id, IPC_RMID, NULL);
        }
    }
    // Si hem arribat a aquest punt per abortament/compleció de la distorsió eliminem el fitxer
    else {
        if(context.file_path != NULL) SO_deleteImage(context.file_path);
    }
}

/*********************************************** 
* 
* @Finalidad: Gestionar y limpiar la memoria compartida asociada al proceso de distorsión, 
*             desasociándola o eliminándola según el estado del proceso y la señal recibida. 
* 
* @Parámetros: 
* in: distortion_context = Estructura `DistortionContext` que contiene el estado actual del proceso de distorsión. 
* in: shm_id = Identificador de la memoria compartida utilizada para almacenar el progreso de la distorsión. 
* in: exit_distortion = Bandera que indica si el proceso de distorsión fue interrumpido. 
* in: sWorkerCountMutex = Puntero al semáforo utilizado para sincronizar el acceso al contador global de workers. 
* in: file_type = Tipo de archivo procesado (`'t'` para texto o `'m'` para multimedia). 
* 
* @Retorno: Ninguno. 
* 
************************************************/
void EXIT_cleanupSharedMemory(DistortionContext distortion_context, int shm_id, volatile int exit_distortion, semaphore* sWorkerCountMutex, char file_type) {
    // Si hem arribat a aquest punt per avortament/ compleció de la distorsió eliminem la regió de memòria compartida
    if (!exit_distortion && shm_id > 0) {
        shmctl(shm_id, IPC_RMID, NULL);
    } 
    // Si arribem a aquest punt per senyal sigint valorem si s'ha d'eliminar la regió de memòria
    else if(EXIT_isLastWorker(sWorkerCountMutex, file_type == 't' ? ENIGMA_COUNTER : HARLEY_COUNTER) && shm_id > 0){
        shmctl(shm_id, IPC_RMID, NULL); // Si som l'últim worker connectat al sistema eliminem regió, sino la deixem intacta per a que el worker que prengui el relleu pugui resumir la distorsió
    }
    else {
        // Si la regió de memòria no ha estat creada la creem
        if(shm_id == 0) {
            char* global_file_path = FILE_buildSharedFilePath(distortion_context.filename, distortion_context.username);
            if(!global_file_path) return;
            key_t key = ftok(global_file_path, 12);
            free(global_file_path);

            if(key == -1) return;
            shm_id = shmget(key, sizeof(DistortionProgress), IPC_CREAT | 0666);
            if ((shm_id) == -1) return;
        }
        // Si ha hagut sigint i no som l'últim worker connectat al sisema, actualitzem copiem els valors de context local al context compartit
        DistortionProgress* distortion_progress = (DistortionProgress*)shmat((shm_id), NULL, 0);
        if (distortion_progress == (void *)-1) return; 

        distortion_progress->current_stage = distortion_context.current_stage;
        distortion_progress->n_packets = distortion_context.n_packets;
        distortion_progress->n_processed_packets = distortion_context.n_processed_packets;
        
        if (shmdt(distortion_progress) == -1) return;
    }
}

/*********************************************** 
* 
* @Finalidad: Liberar la memoria asociada a una estructura `DistortionContext`, 
*             incluyendo los punteros internos que almacenan información del archivo 
*             y metadatos relacionados con la distorsión. 
* 
* @Parámetros: 
* in/out: context = Puntero a la estructura `DistortionContext` cuya memoria será liberada. 
* 
* @Retorno: Ninguno. 
* 
************************************************/
void EXIT_cleanupDistortionContext(DistortionContext *context) {
    freePointer((void**)&((context)->filename));
    freePointer((void**)&((context)->md5sum));
    freePointer((void**)&((context)->file_path));
    freePointer((void**)&((context)->username));
}