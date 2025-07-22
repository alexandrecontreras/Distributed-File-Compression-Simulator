/*********************************************** 
* 
* @Autores: Alexandre Contreras y Armand López
* @Propósito: Definir estructuras y constantes necesarias para la configuración y gestión 
*             del proceso Fleck, incluyendo el manejo de distorsiones y la interacción 
*             con Gotham y workers. 
* @Fecha de creación: 20 de octubre de 2024
* @Última modificación: 4 de enero de 2025
* 
************************************************/

#ifndef _TYPE_FLECK_CUSTOM_H_
#define _TYPE_FLECK_CUSTOM_H_

#include "../Libs/Structure/typeDistort.h"

typedef struct {
    char* username; 
    char* folder_path;
    char* gotham_ip; 
    int gotham_port;
} FleckConfig;

typedef struct {
    char* ip;
    int port;
    int socket;
} MainWorker;

typedef struct {
    char* filename; 
    int status; 
    int file_type; 
} CheckStatus;

typedef struct {
    int n_distortions;
    CheckStatus* distortions; 
} DistortionRecord; 

typedef struct {
    DistortionContext* distortion_context; // Punter a text_to_distort o media_to_distort
    int* distorting_flag;                  // Punter a flag distorting_text o distorting_media
    char* worker_type; 
    MainWorker* main_worker;               // Punter a l'estructura global de worker principal
    int gotham_socket;	
    char* folder_path;
    DistortionRecord* distortion_record;
    volatile int* exit_distortion;
    pthread_mutex_t* print_mutex;
    int* finished_distortion; 
} DistortionThreadArgsF;

#define TEXT    0
#define MEDIA   1

#endif // _TYPE_FLECK_CUSTOM_H_