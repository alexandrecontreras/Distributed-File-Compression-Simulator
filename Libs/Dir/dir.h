/*********************************************** 
* 
* @Autores: Alexandre Contreras y Armand López
* @Propósito: Proveer funciones para la manipulación de directorios, incluyendo verificación 
*             de existencia, listado de archivos por tipo, y movimientos entre directorios privados 
*             y compartidos. 
* @Fecha de creación: 29 de octubre de 2024
* @Última modificación: 4 de enero de 2025
* 
************************************************/

#ifndef _DIR_CUSTOM_H_
#define _DIR_CUSTOM_H_

//Constant del sistema
#define _GNU_SOURCE

//Llibreries del sistema
#include <stdio.h>     // Para sprintf
#include <stdlib.h>    // Para malloc, free, asprintf
#include <string.h>    // Para strcmp, strlen
#include <unistd.h>    // Para STDOUT_FILENO
#include <dirent.h>    // Para opendir, readdir, closedir, rewinddir
#include <sys/stat.h>   // stat, struct stat

//Llibreries pròpies
#include "../IO/io.h"
#include "../File/file.h"
#include "../String/string.h"

//Funcions

/*********************************************** 
* 
* @Finalidad: Verificar si un archivo específico existe en un directorio dado. 
* 
* @Parámetros: 
* in: folder_path = Ruta del directorio donde se buscará el archivo. 
* in: filename = Nombre del archivo que se desea verificar. 
* in: print_mutex = Mutex para sincronizar los mensajes de impresión en caso de error. 
* 
* @Retorno: 
*           1 = El archivo existe en el directorio. 
*           0 = El archivo no existe o ocurrió un error al acceder al directorio. 
* 
************************************************/
int DIR_fileExistsInFolder(const char *folder_path, const char *filename, pthread_mutex_t *print_mutex);

/*********************************************** 
* 
* @Finalidad: Mostrar los archivos disponibles en un directorio según el tipo especificado 
*             (e.g., "Text" para archivos `.txt` o "Media" para otros tipos de archivos). 
* 
* @Parámetros: 
* in: type = Tipo de archivo a listar ("Text" para archivos `.txt` o "Media" para otros). 
* in: folder_path = Ruta del directorio donde se buscarán los archivos. 
* in: print_mutex = Mutex para sincronizar los mensajes de impresión. 
* 
* @Retorno: Ninguno. 
* 
************************************************/
void DIR_printTextDirectory(char *type, char *folder_path, pthread_mutex_t *print_mutex);

/***********************************************
*
* @Finalidad: Verificar si un archivo distorsionado específico existe en un directorio dado.
*
* @Parámetros:
* in: folder_path = Ruta del directorio donde se buscará el archivo distorsionado.
* in: file_name = Nombre del archivo original que se desea verificar.
*
* @Retorno:
*           1 = El archivo distorsionado no existe en el directorio.
*           0 = El archivo distorsionado existe en el directorio.
*           -1 = Error al intentar acceder al directorio.
*
************************************************/
int DIR_checkDistortedFile(char *folder_path, char *file_name, pthread_mutex_t *print_mutex);

/*********************************************** 
* 
* @Finalidad: Verificar si un directorio especificado existe en el sistema de archivos. 
* 
* @Parámetros: 
* in: path = Ruta del directorio que se desea verificar. 
* 
* @Retorno: 
*           1 = El directorio existe. 
*           0 = El directorio no existe o ocurrió un error al intentar acceder a él. 
* 
************************************************/
int DIR_directoryExists(const char* path);

/*********************************************** 
* 
* @Finalidad: Mover un archivo desde una carpeta compartida a una carpeta privada especificada. 
* 
* @Parámetros: 
* in: filename = Nombre del archivo que se desea mover. 
* in: username = Nombre del usuario asociado al archivo. 
* in: private_path = Ruta completa de la carpeta privada de destino. 
* 
* @Retorno: 
*           1 = El archivo se movió correctamente. 
*           0 = Ocurrió un error al intentar mover el archivo. 
* 
************************************************/
int DIR_moveFileToPrivateFolder(char* filename, char* username, char* private_path);

/*********************************************** 
* 
* @Finalidad: Mover un archivo desde una carpeta privada a una carpeta compartida especificada. 
* 
* @Parámetros: 
* in: filename = Nombre del archivo que se desea mover. 
* in: username = Nombre del usuario asociado al archivo. 
* in: private_path = Ruta completa de la carpeta privada de origen. 
* 
* @Retorno: 
*           1 = El archivo se movió correctamente. 
*           0 = Ocurrió un error al intentar mover el archivo. 
* 
************************************************/
int DIR_moveFileToSharedFolder(char* filename, char* username, char* file_path);
#endif // _DIR_CUSTOM_H_
