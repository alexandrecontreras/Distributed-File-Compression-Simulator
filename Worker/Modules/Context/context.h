/*********************************************** 
* 
* @Autores: Alexandre Contreras y Armand López
* @Propósito: Proveer funciones para gestionar el contexto del worker, incluyendo la 
*             actualización del contador global, la validación de metadatos, y la 
*             inicialización y recuperación del contexto de distorsión. 
* 
* @Fecha de creación: 25 de octubre de 2024
* @Última modificación: 4 de enero de 2025
* 
************************************************/

#ifndef _CONTEXT_WORKER_H_
#define _CONTEXT_WORKER_H_

//Constants del sistema
#define _GNU_SOURCE

//Llibreries del sistema
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>

//Llibreries pròpies
#include "../../../Libs/IO/io.h"                          // Per a les funcions d'entrada/sortida
#include "../../../Libs/Semaphore/semaphore_v2.h"                   // Per a les funcions de semàfors
#include "../../../Libs/File/file.h"                      // Per a les funcions de manipulació de fitxers
#include "../../../Libs/Dir/dir.h"                        // Per a les funcions de manipulació de directoris
#include "../../../Libs/Frame/frame.h"                    // Per a les funcions de creació i destrucció de trames

//.h estructures
#include "../../typeWorker.h"           // Per a les estructures de configuració de Worker
#include "../../../Libs/Structure/typeDistort.h"       // Per a les estructures de context de distorsió

//Funcions

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
int CONTEXT_updateWorkerCount(semaphore *sWorkerCountMutex, int increment, int type);

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
int CONTEXT_extractAndValidateMetadata(char *data_buffer, char **username, char **filename, int *filesize, char **md5sum, int *factor);

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
int CONTEXT_fetchDistortionContext(DistortionContext* distortion_context, char* filename, int* shm_id);

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
int CONTEXT_initContextMetadata(DistortionContext* distortion_context, char* filename, char* username, char* md5sum, int filesize, int factor, char* distortions_folder_path);

DistortionContext CONTEXT_initializeContext();

#endif // _CONTEXT_WORKER_H_