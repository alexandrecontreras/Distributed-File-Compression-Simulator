/*********************************************** 
* 
* @Autores: Alexandre Contreras y Armand López
* @Propósito: Definir estructuras relacionadas con el proceso de distorsión de archivos, 
*             incluyendo progreso, metadatos y contexto general para gestionar múltiples etapas.
* @Fecha de creación: 15 de octubre de 2024
* @Última modificación: 4 de enero de 2025
* 
************************************************/

#ifndef _TYPE_DISTORT_CUSTOM_H_
#define _TYPE_DISTORT_CUSTOM_H_

typedef struct {
    char* file_path;
    char* filename; 
    char* username;
    char *md5sum;
    int filesize;              
    int factor;
    int current_stage;
    int n_packets;
    int n_processed_packets;
} DistortionContext;

typedef struct {
    int current_stage;
    int n_packets;
    int n_processed_packets;
} DistortionProgress;

#endif // _TYPE_DISTORT_CUSTOM_H_