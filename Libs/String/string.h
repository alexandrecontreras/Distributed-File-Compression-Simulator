/*********************************************** 
* 
* @Autores: Alexandre Contreras y Armand López
* @Propósito: Proporcionar funciones para la manipulación y validación de cadenas, 
*             incluyendo conversión, limpieza, validación de IPs, e impresión sincronizada.
* @Fecha de creación: 12 de octubre de 2024
* @Última modificación: 4 de enero de 2025
* 
************************************************/

#ifndef _STRING_CUSTOM_H_
#define _STRING_CUSTOM_H_

//Constants del sistema
#define _GNU_SOURCE

//Libreries del sistema
#include <stdio.h>  // Esto ahora habilitará asprintf y vasprintf
#include <ctype.h>       // Para tolower(), isspace()
#include <string.h>      // Para strlen()
#include <stdlib.h>      // Para malloc(), free()
#include <pthread.h>     // Para manejo de mutex
#include <unistd.h>      // Para write()
#include <arpa/inet.h>   // Para inet_pton()
#include <netinet/in.h>  // Para sockaddr_in
#include <stdarg.h>      // Para manejo de argumentos variables

//Llibreries pròpies
#include "../IO/io.h"  // Per a les funcions d'entrada/sortida

//Funcions

/*********************************************** 
* 
* @Finalidad: Convertir todos los caracteres de una cadena a minúsculas. 
* 
* @Parámetros: 
* in/out: str = Puntero a la cadena que se convertirá a minúsculas. 
* 
* @Retorno: Ninguno. 
* 
************************************************/
void STRING_toLowerCase(char* str);

/*********************************************** 
* 
* @Finalidad: Eliminar todos los espacios en blanco de una cadena y compactar los caracteres restantes. 
* 
* @Parámetros: 
* in/out: cmd = Puntero a la cadena de entrada que será modificada para eliminar los espacios. 
* 
* @Retorno: Ninguno. 
* 
************************************************/
void STRING_removeSpaces(char* cmd);

/*********************************************** 
* 
* @Finalidad: Eliminar todos los caracteres '&' de una cadena. 
* 
* @Parámetros: 
* in/out: username = Puntero a la cadena que será procesada para eliminar los caracteres '&'. 
* 
* @Retorno: Ninguno. 
* 
************************************************/
void STRING_checkCharacterAmpersand(char *username);

/*********************************************** 
* 
* @Finalidad: Verificar si una cadena representa una dirección IP válida en formato IPv4. 
* 
* @Parámetros: 
* in: ip = Puntero a la cadena que contiene la dirección IP a verificar. 
* 
* @Retorno: 
*           1 = La cadena es una dirección IP válida. 
*           0 = La cadena no es una dirección IP válida. 
* 
************************************************/
int STRING_isValidIP(const char *ip);

/*********************************************** 
* 
* @Finalidad: Inicializar un mutex utilizado para la sincronización de operaciones de impresión en pantalla. 
* 
* @Parámetros: 
* in/out: print_mutex = Mutex que será inicializado. 
* 
* @Retorno: Ninguno. 
* 
************************************************/
void STRING_initScreenMutex(pthread_mutex_t print_mutex);

/*********************************************** 
* 
* @Finalidad: Destruir un mutex utilizado para la sincronización de operaciones de impresión en pantalla. 
* 
* @Parámetros: 
* in/out: print_mutex = Mutex que será destruido. 
* 
* @Retorno: Ninguno. 
* 
************************************************/
void STRING_destroyScreenMutex(pthread_mutex_t print_mutex);

/*********************************************** 
* 
* @Finalidad: Imprimir una cadena formateada con un color especificado, utilizando 
*             un mutex para sincronizar el acceso a la salida estándar. 
* 
* @Parámetros: 
* in: print_mutex = Puntero al mutex utilizado para sincronizar la operación de impresión. 
* in: color = Cadena que especifica el color en formato ANSI. 
* in: format = Cadena de formato para la impresión (similar a `printf`). 
* in: ... = Lista de argumentos variables para el formato. 
* 
* @Retorno: Ninguno. 
* 
************************************************/
void STRING_printF(pthread_mutex_t *print_mutex, int fd, const char *color, const char *format, ...);
#endif // _STRING_CUSTOM_H_