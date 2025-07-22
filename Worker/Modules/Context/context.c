
/***********************************************
*
* @Autores: Alexandre Contreras, Armand López.
*
* @Finalidad: Implementar funciones para gestionar el contexto de distorsión, 
*             incluyendo la inicialización, actualización de contadores de workers, 
*             validación y recuperación de metadatos, así como la sincronización con 
*             regiones de memoria compartida.
*
* @Fecha de creación: 29 de octubre de 2024.
*
* @Última modificación: 4 de enero de 2025.
*
************************************************/

#include "context.h"


DistortionContext CONTEXT_initializeContext() {
    DistortionContext context;
    context.file_path = NULL;
    context.filename = NULL;
    context.username = NULL;
    context.md5sum = NULL;
    context.filesize = 0;
    context.factor = 0;
    context.current_stage = 0;
    context.n_packets = 0;
    context.n_processed_packets = 0;
    return context;
}

/*********************************************** 
* 
* @Finalidad: Actualizar el contador global de workers (enigmas o harleys) en una región 
*             de memoria compartida, incrementando o decrementando su valor según sea necesario. 
* 
* @Parámetros: 
* in/out: sWorkerCountMutex = Puntero al semáforo que actúa como mutex para sincronizar el acceso al contador. 
* in: increment = Indicador de operación (1 para incrementar el contador, 0 para decrementar). 
* in: type = Tipo de worker (`ENIGMA_COUNTER` para enigmas, `HARLEY_COUNTER` para harleys). 
* 
* @Retorno: 
*           1 = El worker que llamó la función es el último de su tipo conectado al sistema. 
*           0 = Hay más workers de este tipo conectados al sistema. 
*          -1 = Error al interactuar con la memoria compartida o al inicializar el contador. 
* 
************************************************/
int CONTEXT_updateWorkerCount(semaphore *sWorkerCountMutex, int increment, int type) {
    int segment_created = 1;
    int is_last_worker = 0; 

    key_t key = ftok("../../Gotham/config.dat", type);  // Generem clau diferent segons si el comptador referencia enigmes (type=2) o harleys (type=3)
    if (key == -1) return -1;

    int shm_id = shmget(key, sizeof(int), IPC_CREAT | IPC_EXCL | 0666);  // Intentem crear la regió de memòria
    if (shm_id == -1) {
        if (errno == EEXIST) {  // Si la regió de memòria ja existeix, obtenim el seu identificador (shmid) i setegem el flag 'segment_exists' 
            shm_id = shmget(key, sizeof(int), 0666);
            segment_created = 0;
            if (shm_id == -1) return -1;
        } else {
           return -1;
        } 
    }

    // Obtenim el comptador de harleys/enigmes de la regió de memòria compartida
    int *workerCount = (int *)shmat(shm_id, NULL, 0);
    if (workerCount == (void *)-1) return -1;

    // Si hem creat el segment de memòria associat al comptador de workers, l'hem d'inicialitzar
    if (segment_created) {
        *workerCount = 0;  
    }

    // Modifiquem el valor del comptador
    SEM_wait(sWorkerCountMutex);
    if (increment) {
        (*workerCount)++;
        IO_printFormat(STDOUT_FILENO, YELLOW "%s counter incremented. Current %ss connected to Mr.J.System: %d\n" RESET, type==ENIGMA_COUNTER?"Engima":"Harley", type==ENIGMA_COUNTER?"Engima":"Harley", *workerCount);
    } else {
        (*workerCount)--;
        IO_printFormat(STDOUT_FILENO, YELLOW "%s counter decremented. Current %ss connected to Mr.J.System: %d\n" RESET, type==ENIGMA_COUNTER?"Engima":"Harley", type==ENIGMA_COUNTER?"Engima":"Harley", *workerCount);
        if(*workerCount == 0) is_last_worker = 1; 
    }
    SEM_signal(sWorkerCountMutex);

    //Fem detach 
    if (shmdt(workerCount) == -1) {
        perror("shmdt failed");
        return -1;
    }

    // Si el worker que crida la funció es tracta de l'últim del seu tipus, eliminem la regió de memòria associada al comptador de workers d'aquest tipus
    if(is_last_worker) shmctl (shm_id, IPC_RMID, NULL);
    return is_last_worker;  // Booleà que, en cas d'haver cridat la funció per decrementar el comptador, indica si el worker que ha cridat el mètode es tracta del darrer del seu tipus connectat al sistema
}

/*********************************************** 
* 
* @Finalidad: Extraer y validar los metadatos de un archivo a partir de una cadena de datos 
*             en formato delimitado. 
* 
* @Parámetros: 
* in: data_buffer = Cadena que contiene los metadatos delimitados por el carácter `&`. 
* out: username = Puntero que recibirá el nombre del usuario. 
* out: filename = Puntero que recibirá el nombre del archivo. 
* out: filesize = Puntero que recibirá el tamaño del archivo. 
* out: md5sum = Puntero que recibirá el hash MD5 del archivo. 
* out: factor = Puntero que recibirá el factor de distorsión. 
* 
* @Retorno: 
*           1 = Los metadatos fueron extraídos y validados correctamente. 
*           0 = Error en la extracción o alguno de los atributos es inválido. 
* 
************************************************/
int CONTEXT_extractAndValidateMetadata(char *data_buffer, char **username, char **filename, int *filesize, char **md5sum, int *factor) {
    //extreiem els atributs del camp de dades 
    *username = strtok(data_buffer, "&");
    *filename = strtok(NULL, "&");
    char *filesize_str = strtok(NULL, "&");
    *md5sum = strtok(NULL, "&");
    char *factor_str = strtok(NULL, "&");

    //verifiquem que no hi ha cap atribut buit
    if (!(*username) || !(*filename) || !filesize_str || !(*md5sum) || !factor_str) {
        return 0;  
    }

    //convertim i validem filesize
    *filesize = atoi(filesize_str);
    if (*filesize <= 0) {
        return 0;  //filesize no vàlid
    }

    //convertim i validem factor
    *factor = atoi(factor_str);
    if (*factor <= 0) {
        return 0;  //factor no vàlid
    }

    return 1;  //tots els atributs són vàlids
}

/*********************************************** 
* 
* @Finalidad: Inicializar el progreso de distorsión de un archivo, configurando los 
*             parámetros necesarios para iniciar o reanudar el proceso de distorsión. 
* 
* @Parámetros: 
* in/out: distortion_context = Puntero a la estructura `DistortionContext` que será inicializada. 
* in: resuming = Indicador de si se está reanudando una distorsión previa (1) o iniciando una nueva (0). 
* in: current_stage = Etapa actual de la distorsión en caso de reanudarla. 
* in: shm_total_packets = Número total de paquetes almacenados en memoria compartida (usado al reanudar). 
* in: n_processed_packets = Número de paquetes ya procesados en caso de reanudación. 
* 
* @Retorno: 
*           1 = Inicialización exitosa. 
* 
************************************************/
int CONTEXT_initDistortionProgress(DistortionContext* distortion_context, int current_stage, int n_processed_packets) {
    // Inicialitzem etapa de distorsió a "recepció del fitxer"
    distortion_context->current_stage = current_stage;

    int total_packets = distortion_context->filesize / DATA_SIZE;
    if (distortion_context->filesize % DATA_SIZE != 0) {
        total_packets++;
    }

    // Inicialitzem el nombre de paquets
    distortion_context->n_packets = total_packets; 

    // Inicialitzem paquets processats
    distortion_context->n_processed_packets = n_processed_packets; 

    return 1;
}

/*********************************************** 
* 
* @Finalidad: Calcular el porcentaje de progreso de una distorsión basada en la etapa actual 
*             y el número de paquetes procesados. 
* 
* @Parámetros: 
* in: context = Estructura `DistortionContext` que contiene los datos de la distorsión, 
*               incluyendo la etapa actual, el número total de paquetes, y los procesados. 
* 
* @Retorno: 
*           Un valor de tipo `float` que representa el porcentaje de progreso: 
*           - Entre 0 y 50% durante las etapas iniciales (`STAGE_SND_METADATA` o inferiores). 
*           - Entre 50 y 100% durante las etapas posteriores. 
* 
************************************************/
float getProgressPercentage(DistortionContext context) {
    if (context.current_stage <= STAGE_SND_METADATA) {
        return ((float)context.n_processed_packets * 50) / (float)context.n_packets;
    } else {
        return 50 + ((float)context.n_processed_packets * 50) / (float)context.n_packets;
    }
}

/*********************************************** 
* 
* @Finalidad: Calcular el porcentaje de progreso de una distorsión basada en la etapa actual 
*             y el número de paquetes procesados. 
* 
* @Parámetros: 
* in: context = Estructura `DistortionContext` que contiene los datos de la distorsión, 
*               incluyendo la etapa actual, el número total de paquetes, y los procesados. 
* 
* @Retorno: 
*           Un valor de tipo `float` que representa el porcentaje de progreso: 
*           - Entre 0 y 50% durante las etapas iniciales (`STAGE_SND_METADATA` o inferiores). 
*           - Entre 50 y 100% durante las etapas posteriores. 
* 
************************************************/
int CONTEXT_fetchDistortionContext(DistortionContext* distortion_context, char* filename, int* shm_id) {
    int resume_distortion = 0;         // Flag que indica que cal resumir la distorsió
    DistortionProgress* distortion_progress = NULL;
    
    // 1- Comprovem si existeix el fitxer amb nom 'filename' al directori global de distorsions en curs. Això ens permet determinar si hem de resumir o iniciar la distorsió.
    char* global_file_path = FILE_buildSharedFilePath(filename, distortion_context->username);
    if (global_file_path == NULL) return 0;
    
    // Si el fitxer existeix, setegem flag per indicar que hem de resumir la distorsió
    if(DIR_directoryExists(global_file_path)) {
        resume_distortion = 1;
        // 2- Obtenim clau associada a la distorsió a partir del fitxer creat/recuperat
        key_t key = ftok(global_file_path, 12);
        if(key == -1) {
            free(global_file_path);
            return 0;
        }

        // Movem el fitxer del directori global al directori de distorsions del worker actual
        int mv_status = DIR_moveFileToPrivateFolder(filename, distortion_context->username, distortion_context->file_path);
        free(global_file_path);
        if(!mv_status) return 0;

        // 3- Obtenim l'id corresponent a la regió de memòria on es troba el context de la distorsió
        *shm_id = shmget(key, sizeof(DistortionProgress), IPC_CREAT | 0666);
        if ((*shm_id) == -1) return 0;

        // 4- Obtenim el context de distorsió 
        distortion_progress = (DistortionProgress*)shmat((*shm_id), NULL, 0);
        if (distortion_progress == (void *)-1) return 0;
    } 
    // Si el fitxer no existeix, el creem per a poder generar la clau que utilitzarem per a crear la regió de memòria on desarem el context de la distorsió
    else {
        free(global_file_path);
        int fd = open(distortion_context->file_path, O_CREAT, 0666);
        if (fd == -1) {
            free(distortion_context->file_path);
            return 0;
        } 
        close(fd);
    }

    // 5- Preparem la distorsió segons si l'hem d'iniciar o resumir
    int init_ok = CONTEXT_initDistortionProgress(distortion_context, resume_distortion ? distortion_progress->current_stage : STAGE_RECV_FILE,  resume_distortion ? distortion_progress->n_processed_packets : 0); 

    if(resume_distortion) { 
        if (shmdt(distortion_progress) == -1 || !init_ok) return 0;
        float progress_percentage = getProgressPercentage(*distortion_context);
        IO_printFormat(STDOUT_FILENO, MAGENTA "Fetched distortion context. Current progress: %d%%. Resuming distortion...\n" RESET, (int)progress_percentage);
    }
    else {
        IO_printStatic(STDOUT_FILENO, MAGENTA "Created distortion context. starting distortion...\n" RESET);
    }

    return 1; 
}

/*********************************************** 
* 
* @Finalidad: Inicializar y configurar los metadatos del contexto de distorsión, 
*             incluyendo el nombre del archivo, el usuario asociado, el hash MD5, 
*             y otros atributos relevantes. 
* 
* @Parámetros: 
* in/out: distortion_context = Puntero a la estructura `DistortionContext` que será inicializada. 
* in: filename = Nombre del archivo que se va a distorsionar. 
* in: username = Nombre del usuario asociado al archivo. 
* in: md5sum = Hash MD5 del archivo. 
* in: filesize = Tamaño del archivo en bytes. 
* in: factor = Factor de distorsión aplicado al archivo. 
* in: distortions_folder_path = Ruta al directorio donde se procesará el archivo. 
* 
* @Retorno: 
*           1 = La estructura fue inicializada correctamente. 
*           0 = Error durante la asignación de memoria o construcción de rutas. 
* 
************************************************/
int CONTEXT_initContextMetadata(DistortionContext* distortion_context, char* filename, char* username, char* md5sum, int filesize, int factor, char* distortions_folder_path) {
    // Creem i assignem el path del fitxer a distorsionar
    distortion_context->file_path = FILE_buildPrivateFilePath(distortions_folder_path, filename, username);
    if (!distortion_context->file_path) return 0;

    // Copiem el nom del fitxer al context de distorsió
    distortion_context->filename = strdup(filename);
    if(!distortion_context->filename) return 0;  

    distortion_context->username = strdup(username);
    if(!distortion_context->username) return 0;

    // Generem el MD5 del fitxer i el copiem al camp corresponent de l'estructura de context
    distortion_context->md5sum = strdup(md5sum);
    if(!distortion_context->md5sum) return 0; 

    distortion_context->filesize = filesize;
    distortion_context->factor = factor;

    return 1;
}