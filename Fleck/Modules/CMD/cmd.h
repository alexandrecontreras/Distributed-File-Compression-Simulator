/*********************************************** 
* 
* @Autores: Alexandre Contreras y Armand López
* @Propósito: Ejecutar el servidor del worker, aceptando conexiones de flecks, 
*             gestionando threads para distorsión y manejando el cierre seguro del servidor. 
* @Fecha de creación: 15 de octubre de 2024
* @Última modificación: 4 de enero de 2025
* 
************************************************/

#ifndef _CMD_FLECK_CUSTOM_H_
#define CMD__CMD_FLECK_CUSTOM_H_

//Llibreries del sistema
#include <stdlib.h>     // malloc, realloc, free, atoi
#include <string.h>     // strlen, strncmp, strcmp
#include <ctype.h>      // isspace, isdigit, tolower
#include <unistd.h>     // STDOUT_FILENO (si estás interactuando con la salida estándar)
#include <pthread.h>    // pthread_mutex_t

//Llibreries pròpies
#include "../../../Libs/IO/io.h"                  // Per a les funcions d'entrada/sortida
#include "../../../Libs/String/string.h"          // Per a les funcions de manipulació de strings

#define CMD_INVALID        -1
#define CMD_CONNECT         0
#define CMD_LOGOUT          1
#define CMD_LISTMEDIA       2
#define CMD_LISTTEXT        3
#define CMD_DISTORT         4
#define CMD_CHECKSTATUS     5
#define CMD_CLEARALL        6

/*********************************************** 
* 
* @Finalidad: Validar si un comando cumple con el formato esperado para el sistema de distorsión. 
*             Extrae el nombre del archivo y el factor de distorsión si el comando es válido. 
* 
* @Parámetros: 
* in: cmd = Comando de entrada en forma de cadena que contiene la palabra "distort", 
*           el nombre del archivo y el factor de distorsión.
* in/out: filename_ptr = Puntero al nombre del archivo donde se almacenará el valor extraído. 
*                        Debe ser inicializado por el usuario o será nulo si no se necesita. 
* in/out: factor_ptr = Puntero entero donde se almacenará el factor extraído. 
*                      Debe ser inicializado por el usuario o será nulo si no se necesita.
* in: print_mutex = Mutex para gestionar la exclusión mutua al imprimir mensajes de error. 
* 
* @Retorno: Retorna 1 si el comando es válido y contiene todos los parámetros necesarios. 
*           Retorna 0 si el comando es inválido o hay errores en el formato del comando. 
* 
************************************************/
int CMD_isDistortCommandValid(const char* cmd, char** filename_ptr, int* factor_ptr, pthread_mutex_t *print_mutex);

/*********************************************** 
* 
* @Finalidad: Convertir un comando introducido como cadena a su equivalente numérico predefinido. 
*             Verifica la validez de comandos específicos y maneja errores de entrada. 
* 
* @Parámetros: 
* in: cmd = Comando de entrada en formato de cadena. Puede incluir espacios y debe ser 
*           uno de los comandos reconocidos por el sistema.
* in: print_mutex = Mutex utilizado para garantizar exclusión mutua al imprimir mensajes 
*                   de error relacionados con el comando.
* 
* @Retorno: Retorna un entero que corresponde al comando reconocido:
*           CMD_DISTORT = Si el comando es "distort" y es válido.
*           CMD_CONNECT = Si el comando es "connect".
*           CMD_LOGOUT = Si el comando es "logout".
*           CMD_LISTMEDIA = Si el comando es "listmedia".
*           CMD_LISTTEXT = Si el comando es "listtext".
*           CMD_CHECKSTATUS = Si el comando es "checkstatus".
*           CMD_CLEARALL = Si el comando es "clearall".
*           CMD_INVALID = Si el comando no es válido o no está reconocido.
* 
************************************************/
int CMD_changeComandToNumber(char* cmd, pthread_mutex_t *print_mutex);

#endif // _CMD_FLECK_CUSTOM_H_