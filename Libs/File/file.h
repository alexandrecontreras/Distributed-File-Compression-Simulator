/*********************************************** 
* 
* @Autores: Alexandre Contreras y Armand López
* @Propósito: Proveer funciones para la manipulación de archivos, incluyendo operaciones 
*             como copiar, mover, reemplazar, calcular hash MD5, y determinar información 
*             sobre archivos como su tamaño o tipo. 
* @Fecha de creación: 12 de octubre de 2024
* @Última modificación: 4 de enero de 2025
* 
************************************************/

#ifndef _FILE_CUSTOM_H_
#define _FILE_CUSTOM_H_

//Constant del sistema
#define _GNU_SOURCE

//Llibreria del sistema
#include <stdio.h>        // perror()
#include <stdlib.h>       // malloc(), free(), strdup(), exit()
#include <string.h>       // strlen(), strcmp(), memcpy(), strrchr()
#include <unistd.h>       // close(), lseek(), usleep()
#include <fcntl.h>        // open(), flags O_RDONLY, O_WRONLY, O_CREAT
#include <sys/types.h>    // Definiciones de tipos de datos
#include <sys/stat.h>     // stat(), permisos de archivo
#include <sys/wait.h>     // wait()
#include <sys/socket.h>   // Sockets
#include <netinet/in.h>   // sockaddr_in
#include <arpa/inet.h>    // inet_addr(), htons()
#include <errno.h>        // errno
#include <time.h>         // time()
#include <signal.h>       // Manejo de señales
#include <ctype.h>        // tolower()

//Llibreries pròpies
#include "../IO/io.h"
#include "../String/string.h"

#define PATH_FLECK  1
#define PATH_WORKER 2

//Funcions

/*********************************************** 
* 
* @Finalidad: Construir una ruta completa a un archivo dentro de una carpeta privada, 
*             utilizando el nombre de usuario opcionalmente. 
* 
* @Parámetros: 
* in: distortions_folder_path = Ruta base de la carpeta de distorsiones privadas. 
* in: filename = Nombre del archivo. 
* in: username = Nombre del usuario asociado al archivo (puede ser NULL si no se utiliza). 
* 
* @Retorno: 
*           Puntero a una cadena que contiene la ruta completa del archivo en la carpeta privada. 
*           Retorna NULL si ocurre un error al construir la ruta. 
* 
************************************************/
char* FILE_buildPrivateFilePath(char* distortions_folder_path, char* filename, char* username);

/*********************************************** 
* 
* @Finalidad: Construir una ruta completa a un archivo dentro de la carpeta compartida 
*             de distorsiones en curso, utilizando el nombre de usuario y el nombre del archivo. 
* 
* @Parámetros: 
* in: filename = Nombre del archivo. 
* in: username = Nombre del usuario asociado al archivo. 
* 
* @Retorno: 
*           Puntero a una cadena que contiene la ruta completa del archivo en la carpeta compartida. 
*           Retorna NULL si ocurre un error al construir la ruta. 
* 
************************************************/
char* FILE_buildSharedFilePath(char* filename, char* username);

/*********************************************** 
* 
* @Finalidad: Copiar un archivo desde una ruta de origen a una ruta de destino 
*             utilizando la herramienta `cp` en un proceso hijo. 
* 
* @Parámetros: 
* in: source_path = Ruta completa del archivo de origen. 
* in: destination_path = Ruta completa donde se copiará el archivo de destino. 
* 
* @Retorno: 
*           0 = La copia del archivo se realizó con éxito. 
*          -1 = Ocurrió un error durante el fork o la ejecución de la copia. 
* 
************************************************/
int FILE_copyFile(const char *source_path, const char *destination_path);

/*********************************************** 
* 
* @Finalidad: Mover un archivo desde una ruta de origen a una ruta de destino 
*             utilizando la herramienta `mv` en un proceso hijo. 
* 
* @Parámetros: 
* in: source_path = Ruta completa del archivo de origen. 
* in: destination_path = Ruta completa donde se moverá el archivo de destino. 
* 
* @Retorno: 
*           0 = El archivo se movió con éxito. 
*          -1 = Ocurrió un error durante el fork o la ejecución del comando `mv`. 
* 
************************************************/
int FILE_moveFile(const char *source_path, const char *destination_path);

//int FILE_removeFile(const char *file_path);

/*********************************************** 
* 
* @Finalidad: Obtener el tamaño de un archivo especificado por su ruta. 
* 
* @Parámetros: 
* in: full_path = Ruta completa del archivo cuyo tamaño se desea calcular. 
* 
* @Retorno: 
*           >= 0 = Tamaño del archivo en bytes. 
*           -1 = Ocurrió un error al intentar abrir el archivo. 
* 
************************************************/
int FILE_getFileSize(const char *full_path);

/*********************************************** 
* 
* @Finalidad: Obtener la extensión de un archivo a partir de su nombre. 
* 
* @Parámetros: 
* in: filename = Nombre del archivo del cual se desea obtener la extensión. 
* 
* @Retorno: 
*           Puntero a una cadena que contiene la extensión del archivo (sin el punto). 
*           Cadena vacía (`""`) si el archivo no tiene extensión. 
* 
************************************************/
char *FILE_getFileExtension(const char *filename);

/*********************************************** 
* 
* @Finalidad: Determinar el tipo de archivo (e.g., "Media", "Text" o "Unknown") 
*             en función de su extensión. 
* 
* @Parámetros: 
* in: filename = Nombre del archivo cuyo tipo se desea determinar. 
* in: print_mutex = Mutex para sincronizar los mensajes de impresión en caso de error. 
* 
* @Retorno: 
*           Puntero a una cadena dinámica que contiene el tipo de archivo:
*           - `"Media"` si la extensión coincide con audio o imágenes conocidas.
*           - `"Text"` si la extensión es `.txt`.
*           - `"Unknown"` si no se reconoce el tipo o si ocurre un error. 
* 
************************************************/
char* FILE_determineFileType(const char *filename, pthread_mutex_t *print_mutex);

/*********************************************** 
* 
* @Finalidad: Calcular el hash MD5 de un archivo especificado utilizando el comando `md5sum`. 
* 
* @Parámetros: 
* in: file_path = Ruta completa del archivo cuyo MD5 se desea calcular. 
* 
* @Retorno: 
*           Puntero a una cadena dinámica que contiene el hash MD5 del archivo. 
*           Retorna NULL si ocurre un error durante la operación (e.g., fallo en el fork, 
*           error en la ejecución de `md5sum` o problemas con la pipe). 
* 
************************************************/
char* FILE_calculateMD5(const char *file_path);

/*********************************************** 
* 
* @Finalidad: Comparar el hash MD5 de un archivo con un hash MD5 esperado para verificar 
*             si ambos coinciden. 
* 
* @Parámetros: 
* in: original_md5 = Cadena que contiene el hash MD5 esperado. 
* in: file_path = Ruta completa del archivo cuyo hash MD5 se calculará y comparará. 
* 
* @Retorno: 
*           1 = Los hashes MD5 coinciden. 
*           0 = Los hashes MD5 no coinciden o ocurrió un error al calcular el MD5 del archivo. 
* 
************************************************/
int FILE_compareMD5(char* original_md5, char* file_path);

/*********************************************** 
* 
* @Finalidad: Reemplazar el contenido de un archivo de destino con el contenido de un 
*             archivo de origen, utilizando un proceso hijo y el comando `cat`. 
* 
* @Parámetros: 
* in: source_path = Ruta completa del archivo de origen cuyo contenido se copiará. 
* in: destination_path = Ruta completa del archivo de destino que será sobrescrito. 
* 
* @Retorno: 
*           0 = El archivo de destino fue reemplazado con éxito. 
*          -1 = Ocurrió un error durante el fork, la apertura del archivo de destino, 
*               la redirección de salida o la ejecución del comando `cat`. 
* 
************************************************/
int FILE_replaceFile(const char *source_path, const char *destination_path);

#endif // _FILE_CUSTOM_H_
