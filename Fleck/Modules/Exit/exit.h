/*********************************************** 
* 
* @Autores: Alexandre Contreras y Armand López
* @Propósito: Proporcionar funciones para liberar la memoria dinámica asociada a las 
*             estructuras del sistema Fleck, incluyendo configuraciones, contextos de 
*             distorsión y registros. Gestionar de forma segura la limpieza de recursos 
*             en el sistema. 
* @Fecha de creación: 25 de octubre de 2024
* @Última modificación: 4 de enero de 2025
* 
************************************************/

#ifndef _EXIT_FLECK_CUSTOM_H_
#define _EXIT_FLECK_CUSTOM_H_

//Llibreries del sistema
#include <stdlib.h>    // malloc, free
#include <unistd.h>    // close
#include <signal.h>    // pthread_kill, SIGUSR1
#include <pthread.h>   // pthread_t

//Llibreries pròpies
#include "../../../Libs/IO/io.h"                  // Per a les funcions d'entrada/sortida

//.h estructures
#include "../../typeFleck.h"                          // Per a les estructures de configuracio de Fleck
#include "../../../Libs/Structure/typeDistort.h"      // Per a les estructures de distorsió de text i media

//Funcions
/*********************************************** 
* 
* @Finalidad: Liberar la memoria dinámica asociada a las estructuras utilizadas en 
*             el sistema, incluyendo la configuración de Fleck, contextos de distorsión, 
*             workers principales y el registro de distorsiones. 
* 
* @Parámetros: 
* in/out: fleck_config = Puntero a la estructura `FleckConfig` cuya memoria será liberada. 
* in/out: text_to_distort = Puntero al contexto de distorsión de archivos de texto. 
* in/out: media_to_distort = Puntero al contexto de distorsión de archivos de medios. 
* in/out: main_enigma = Puntero a la estructura del worker principal de tipo Enigma. 
* in/out: main_harley = Puntero a la estructura del worker principal de tipo Harley. 
* in/out: distortion_record = Puntero al registro de distorsiones que será liberado. 
* 
* @Retorno: Ninguno. 
* 
************************************************/
void EXIT_freeMemory(FleckConfig *fleck_config, DistortionContext *text_to_distort, DistortionContext *media_to_distort, MainWorker *main_enigma, MainWorker *main_harley, DistortionRecord* distortion_record);

/*********************************************** 
* 
* @Finalidad: Liberar la memoria dinámica asociada a los campos de una estructura 
*             `DistortionContext`. 
* 
* @Parámetros: 
* in/out: context = Puntero a la estructura `DistortionContext` cuya memoria será liberada. 
* 
* @Retorno: Ninguno. 
* 
************************************************/
void EXIT_cleanupDistortionContext(DistortionContext** context);

/*********************************************** 
* 
* @Finalidad: Liberar la memoria dinámica asociada a las entradas de un registro 
*             de distorsiones (`DistortionRecord`) y reiniciar el contador de distorsiones. 
* 
* @Parámetros: 
* in/out: record = Puntero a la estructura `DistortionRecord` cuya memoria será liberada. 
* 
* @Retorno: Ninguno. 
* 
************************************************/
void EXIT_freeDistortionRecord(DistortionRecord* record);

#endif // _EXIT_FLECK_CUSTOM_H_