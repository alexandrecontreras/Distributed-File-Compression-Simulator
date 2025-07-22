/*********************************************** 
* 
* @Autores: Alexandre Contreras y Armand López
* @Propósito: Definir constantes, estructuras y funciones necesarias para gestionar 
*             el proceso de distorsión de archivos en el proceso Fleck. 
* @Fecha de creación: 22 de octubre de 2024
* @Última modificación: 4 de enero de 2025
* 
************************************************/

#ifndef _DISTORTION_FLECK_CUSTOM_H_
#define _DISTORTION_FLECK_CUSTOM_H_

//Constant del sistema
#define _GNU_SOURCE

//LLibreries del sistema
#include <stdlib.h>     // Para malloc, free, realloc, asprintf, strdup
#include <string.h>     // Para manipulación de cadenas: strcmp, strdup, etc.
#include <unistd.h>     // Para close, funciones de control de archivos
#include <pthread.h>    // Para threads: pthread_create, pthread_detach, pthread_kill
#include <fcntl.h>      // Para operaciones en descriptores de archivo
#include <errno.h>      // Para gestión de errores: errno
#include <signal.h>     // Para manejar señales: pthread_kill
#include <sys/socket.h> // Para operaciones de sockets: recv
#include <netinet/in.h> // Para estructuras relacionadas con direcciones IP y puertos
#include <arpa/inet.h>  // Para conversiones de direcciones IP: inet_ntop

//Llibreries pròpies
#include "../../../Libs/IO/io.h"                  // Per a les funcions d'entrada/sortida
#include "../../../Libs/File/file.h"                // Per a les funcions de manipulació de directoris
#include "../../../Libs/Communication/communication.h" // Per a les funcions de comunicació amb Gotham, Enigma i Harley
#include "../../../Libs/String/string.h"          // Per a les funcions de manipulació de strings

//Moduls de Fleck
#include "../Communication/communication.h"        // Per a les funcions de comunicació amb Gotham, Enigma i Harley
#include "../Exit/exit.h"                          // Per a les funcions de sortida del programa

//.h estructures
#include "../../typeFleck.h"                          // Per a les estructures de configuracio de Fleck
#include "../../../Libs/Structure/typeDistort.h"      // Per a les estructures de distorsió de text i media

//Constants pròpies
#define STAGE_SND_FILE      4
#define STAGE_RCV_METADATA  6
#define STAGE_RECV_FILE     0
#define STAGE_DISCONNECT    7

#define NO_ACK_ERROR        1
#define WORKER_DOWN         0
#define UNEXPECTED_ERROR   -1

#define ONGOING             1
#define COMPLETED           0
#define FAILED             -1

//Funcions
/*********************************************** 
* 
* @Finalidad: Preparar el contexto, solicitar un worker, y lanzar un hilo para iniciar 
*             el proceso de distorsión de un archivo. 
* 
* @Parámetros: 
* in/out: context = Puntero a la estructura `DistortionContext` donde se almacenará la información del proceso. 
* in: filename = Nombre del archivo a distorsionar. 
* in: username = Nombre del usuario que solicita la distorsión. 
* in: type = Tipo de distorsión a realizar ("Text" o "Media"). 
* in/out: thread = Puntero al identificador del hilo que manejará la distorsión. 
* in: factor = Factor de distorsión aplicado al archivo. 
* in/out: distorting_flag = Bandera que indica si hay un proceso de distorsión en curso. 
* in: main_worker = Puntero a la estructura `MainWorker` para manejar la conexión con el worker. 
* in: gotham_socket = Descriptor del socket conectado al servidor Gotham. 
* in: folder_path = Ruta al directorio donde se encuentra el archivo. 
* in/out: distortion_record = Puntero al registro de distorsiones donde se añadirá la nueva entrada. 
* in: exit_distortion = Bandera para indicar si se debe salir del proceso de distorsión. 
* in: print_mutex = Mutex para garantizar la exclusión mutua al imprimir mensajes de estado o error. 
* in: finished_distortion = Puntero a flag que indica si el thread de distorsión ha finalizado. 
*
* @Retorno: Retorna un entero que indica el estado de la operación:
*           1 = Proceso de distorsión iniciado con éxito. 
*           0 = Error en alguna etapa del proceso (e.g., preparación del contexto, conexión, o creación del hilo). 
* 
************************************************/
int DIST_prepareAndStartDistortion(DistortionContext *context, char *filename, char* username, char *type, pthread_t *thread, int factor, int* distorting_flag, MainWorker* main_worker, int gotham_socket, char* folder_path, DistortionRecord* distortion_record, volatile int* exit_distortion, int* finished_distortion, pthread_mutex_t *print_mutex);

#endif // _DISTORTION_FLECK_CUSTOM_H_