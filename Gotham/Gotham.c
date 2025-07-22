/*********************************************** 
* 
* @Autores: Alexandre Contreras y Armand López
* @Propósito: Implementar el servidor Gotham, incluyendo la configuración inicial, manejo 
*             de conexiones entrantes, registro de eventos en un log y limpieza segura 
*             de recursos al finalizar. 
* @Fecha de creación: 26 de octubre de 2024
* @Última modificación: 4 de enero de 2025
* 
************************************************/

//Constant del sistema
#define _GNU_SOURCE

//Llibreries del sistema
#include <stdlib.h>     // malloc, free, exit
#include <string.h>     // strcmp, strcpy, etc.
#include <unistd.h>     // close, STDOUT_FILENO
#include <fcntl.h>      // open
#include <signal.h>     // signal, SIGINT
#include <stdio.h>
#include <time.h>

//Llibreries pròpies
#include "../Libs/IO/io.h"                          // Per a les funcions d'entrada/sortida
#include "../Libs/Load/load_config.h"               // Per a les funcions de càrrega de fitxers de configuració
#include "../Libs/String/string.h"                  // Per a les funcions de manipulació de strings
#include "../Libs/Socket/socket.h"                  // Per a les funcions de creació de sockets
#include "../Libs/LinkedList/fleckLinkedList.h"     // Per a les funcions de la llista enllaçada de Flecks
#include "../Libs/LinkedList/workerLinkedList.h"    // Per a les funcions de la llista enllaçada de Workers
#include "../Libs/Frame/frame.h"                    // Per a les funcions de creació de frames
#include "../Libs/Communication/communication.h"    // Per a les funcions de comunicació

//Moduls de Gotham
#include "Modules/Exit/exit.h"                      // Per a les funcions de sortida del programa
#include "Modules/Handle/handle.h"                  // Per a les funcions de gestió de les connexions
#include "Modules/MC/manage_client.h"               // Per a les funcions de gestió dels clients
#include "Modules/Communication/communication.h"   // Per a les funcions de comunicació
#include "Modules/Server/server.h"                  // Per a les funcions del servidor

//.h estructures
#include "typeGotham.h"     // Per a les estructures de configuració de Gotham

//Define pròpi
#define LOG_FILE "logs.txt"

//Variables globals
volatile int exit_program = 0;                        // Variable global per a controlar si s'està sortint del programa

//Funcions

/*********************************************** 
* 
* @Finalidad: Leer mensajes desde un pipe para registrarlos en un archivo de log. 
*             Finaliza cuando recibe un mensaje específico ('X'). 
* 
* @Parámetros: 
* in: fd_arkham = Array de descriptores de archivo para el pipe utilizado para la comunicación. 
*                 `fd_arkham[0]` es el extremo de lectura, y `fd_arkham[1]` es el extremo de escritura (cerrado en esta función). 
* 
* @Retorno: Ninguno. La función termina el proceso con `exit(EXIT_SUCCESS)` al finalizar. 
* 
************************************************/
void arkhamLog(int fd_arkham[2]) {
    pthread_mutex_t print_mutex = PTHREAD_MUTEX_INITIALIZER; // Mutex per a la impressió per pantalla
    STRING_initScreenMutex(print_mutex); // Inicialitzar el mutex per a la impressió per pantalla

    int log_fd = open(LOG_FILE, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (log_fd == -1) {
        IO_printStatic(STDOUT_FILENO, RED "Error: Opening log file\n" RESET);
        exit(EXIT_FAILURE);
    }

    char *log = NULL;

    while (1) {
        log = IO_readUntil(fd_arkham[0], '\n');  // Leer desde el pipe

        if (log) {
            if (strcmp(log, "X") == 0) { // Si recibe 'X', salir del bucle
                free(log);
                break;
            }
            // Escribir en el archivo de logs
            STRING_printF(&print_mutex, log_fd, RESET, log);
            STRING_printF(&print_mutex, log_fd, RESET, " \n");
            free(log); // Liberar memoria dinámica
        }
    }

    STRING_destroyScreenMutex(print_mutex); // Destruir el mutex de pantalla
    close(fd_arkham[0]); // Cerrar el extremo de lectura del pipe
    close(log_fd);       // Cerrar el archivo de logs

    exit(EXIT_SUCCESS);
}

/*********************************************** 
* 
* @Finalidad: Manejar la señal SIGINT (Ctrl+C) estableciendo una bandera para indicar 
*             que el programa debe finalizar. 
* 
* @Parámetros: 
* in: sig = Señal recibida (no utilizada en el cuerpo de la función). 
* 
* @Retorno: Ninguno. 
* 
************************************************/
void handle_sigint(int sig __attribute__((unused))) {
    IO_printStatic(STDOUT_FILENO, "\nReceived SIGINT (Ctrl+C), cleaning up...\n");
    exit_program = 1;
}

/*********************************************** 
* 
* @Finalidad: Punto de entrada principal para el servidor Gotham. Configura el servidor, 
*             gestiona señales, inicia el proceso de log y maneja las conexiones entrantes 
*             hasta que se recibe una señal de terminación (SIGINT). 
* 
* @Parámetros: 
* in: argc = Número de argumentos pasados al programa. Debe ser 2 para una ejecución válida. 
* in: argv = Array de cadenas que contiene los argumentos. El segundo argumento debe ser 
*            la ruta al archivo de configuración de Gotham. 
* 
* @Retorno: 
*           0 = Ejecución exitosa. 
*           EXIT_FAILURE = Error crítico durante la inicialización o ejecución. 
* 
************************************************/
int main(int argc, char** argv) {
    int fd_arkham[2];               // File descriptor del pipe amb Arkham

    if (pipe(fd_arkham)==-1){
        IO_printStatic(STDOUT_FILENO, RED "Error: Creating pipe\n" RESET);
        exit(-1);
    }

    //configurem signal
    signal(SIGINT, handle_sigint);

    int n = fork();
    switch (n) {
        case -1:
            IO_printStatic(STDOUT_FILENO, RED "Error: Creating fork\n" RESET);
            close(fd_arkham[0]);
            close(fd_arkham[1]);
            exit(EXIT_FAILURE);
            break;

        case 0: //AL FILL
            close(fd_arkham[1]); // Cerrar el extremo de escritura del pipe en el hijo
            arkhamLog(fd_arkham);
            break;
        
        default: //AL PARE
            close(fd_arkham[0]); // Cerrar el extremo de lectura de la pipe
            
            GothamConfig *global_gotham_conf = NULL;        // Variable global per a la configuració de Gotham
            GothamServer *global_gotham_server = NULL;      // Variable global per al servidor de Gotham
            GothamConfig gotham_conf;       //Estructura de configuració de Gotham
            GothamServer gotham_server;     // Estructura del servidor Gotham

            gotham_server.fd_log = fd_arkham[1]; // Assignar el file descriptor de la pipe a la variable global

            //referenciem variables globals a les estructures de configuració i servidor respectivament
            global_gotham_conf = &gotham_conf;
            global_gotham_server = &gotham_server;

            if (argc != 2) {
                IO_printStatic(STDOUT_FILENO, "Navigate to the directory of the program you want to run and execute -> Gotham <config_file>\n");
                close(fd_arkham[1]); // Cerrar el extremo de escritura
                exit(EXIT_FAILURE);
            }
            
            //Carregar configuració del fitxer
            if (LOAD_loadConfigFile(argv[1], &gotham_conf, GOTHAM_CONF) == LOAD_FAILURE) {
                IO_printStatic(STDOUT_FILENO, RED "Error: Loading Gotham configuration\n" RESET);
                close(fd_arkham[1]); // Cerrar el extremo de escritura
                exit(EXIT_FAILURE);
            }

            LOAD_printConfig(&gotham_conf, GOTHAM_CONF);
            
            if (SRV_initGothamServer(&gotham_server, &gotham_conf) == -1) {
                IO_printStatic(STDOUT_FILENO, RED "Error: Initializing Gotham server\n" RESET);
                write(fd_arkham[1], "X\n", 2); // Send 'X' to the pipe to stop the log process
                close(fd_arkham[1]); // Cerrar la pipe
                wait(NULL);
                exit(EXIT_FAILURE);
            }

            IO_printStatic(STDOUT_FILENO, YELLOW "\nGotham server initialized" RESET);
            IO_printStatic(STDOUT_FILENO, YELLOW "\nWaiting for connections…  \n" RESET);

            SRV_runGothamServer(&gotham_server, &exit_program);
            
            write(fd_arkham[1], "X\n", 2); // Send 'X' to the pipe to stop the log process
            IO_printStatic(STDOUT_FILENO, YELLOW "\nExiting Gotham server\n" RESET);
            close(fd_arkham[1]); // Cerrar la pipe
            wait(NULL); 

            //Tanquem connexions i alliberem recursos
            if (global_gotham_server != NULL) {
                EXIT_closeAllConnections(global_gotham_server);  
            }

            EXIT_freeMemory(global_gotham_conf, global_gotham_server); 
            break;
    }

    return 0;
}