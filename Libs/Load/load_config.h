/*********************************************** 
* 
* @Autores: Alexandre Contreras y Armand López
* @Propósito: Proveer funciones para la carga y visualización de configuraciones 
*             desde archivos hacia estructuras específicas según su tipo.
* @Fecha de creación: 25 de octubre de 2024
* @Última modificación: 4 de enero de 2025
* 
************************************************/

#ifndef _LOAD_CUSTOM_H_
#define _LOAD_CUSTOM_H_

// Constants del sistema
#define _GNU_SOURCE     // Permet utilitzar asprintf

//Llibreries del sistema
#include <stdio.h>      // Entrada/salida estándar
#include <stdlib.h>     // malloc, free, atoi
#include <string.h>     // strcmp, strlen
#include <unistd.h>     // read, write, STDOUT_FILENO

//Llibreries pròpies
#include "../IO/io.h"           // readUntil, printFormat, printStatic
#include "../String/string.h"   // checkCaracterAmpersand

//.h de les estructures
#include "../../Fleck/typeFleck.h"
#include "../../Gotham/typeGotham.h"
#include "../../Worker/typeWorker.h"

#define LOAD_SUCCESS    1
#define LOAD_FAILURE    0

#define GOTHAM_CONF         1
#define FLECK_CONF          2
#define WORKER_CONF         3
//Funcions

/*********************************************** 
* 
* @Finalidad: Leer un archivo de configuración y cargar sus valores en una estructura 
*             específica según su tipo. 
* 
* @Parámetros: 
* in: filename = Nombre del archivo de configuración. 
* in/out: config_struct = Puntero a la estructura donde se almacenará la configuración cargada. 
* in: type = Tipo de configuración (e.g., `FLECK_CONF`, `GOTHAM_CONF`, `WORKER_CONF`). 
* 
* @Retorno: 
*           LOAD_SUCCESS = La configuración fue cargada correctamente. 
*           LOAD_FAILURE = Error al abrir, leer o cerrar el archivo de configuración. 
* 
************************************************/
int LOAD_loadConfigFile(char* filename, void* config_struct, int type);

/*********************************************** 
* 
* @Finalidad: Imprimir la configuración de una estructura especificada según su tipo. 
* 
* @Parámetros: 
* in: config_struct = Puntero a la estructura de configuración. 
* in: type = Tipo de configuración (e.g., `FLECK_CONF`, `GOTHAM_CONF`, `WORKER_CONF`). 
* 
* @Retorno: Ninguno. 
* 
************************************************/
void LOAD_printConfig(void* config_struct, int type);

#endif // _LOAD_CUSTOM_H_