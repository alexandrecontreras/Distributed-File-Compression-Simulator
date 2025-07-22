/*********************************************** 
* 
* @Autores: Alexandre Contreras y Armand López
* @Propósito: Implementar el servidor del proceso Harley, encargado de gestionar 
*             conexiones con Gotham y Flecks, manejar la asignación de workers 
*             principales, y procesar tareas de distorsión de archivos multimedia. 
* 
* @Fecha de creación: 20 de octubre de 2024
* @Última modificación: 4 de enero de 2025
* 
************************************************/

//Constants del sistema
#define _GNU_SOURCE

//Llibreries del sistema
#include <stdlib.h>     // Para malloc, free, exit
#include <string.h>     // Para funciones de manipulación de cadenas: strcmp, etc.
#include <unistd.h>     // Para operaciones con descriptores de archivo: close, STDOUT_FILENO
#include <fcntl.h>      // Para funciones de control de archivos: open
#include <signal.h>     // Para manejo de señales: signal, SIGINT, SIGUSR1
#include <errno.h>      // Para definición de errores: EEXIST
#include <sys/shm.h>    // Para funciones de memoria compartida: shmget, shmat, shmdt, shmctl
#include <sys/types.h>  // Para tipos de datos: key_t 
#include <sys/ipc.h>    // Para funciones de memoria compartida: ftok
#include <pthread.h>    // Para hilos y mutex: pthread_t, pthread_mutex_init

//Llibreries pròpies
#include "../../Libs/IO/io.h"                          // Per a les funcions d'entrada/sortida
#include "../../Libs/String/string.h"                  // Per a les funcions de manipulació de strings
#include "../../Libs/Frame/frame.h"                        // Per a les funcions de creació i destrucció de trames
#include "../../Libs/Semaphore/semaphore_v2.h"                   // Per a les funcions de semàfors
#include "../../Libs/Load/load_config.h"               // Per a les funcions de càrrega de fitxers de configuració
#include "../../Libs/Socket/socket.h"                  // Per a les funcions de creació de sockets
#include "../../Libs/Monitor/monitor.h"                // Per a les funcions de monitoratge de connexions
#include "../../Libs/Communication/communication.h"    // Per a les funcions de comunicació
#include "../../Libs/Dir/dir.h"                        // Per a les funcions de manipulació de directoris
#include "../../Libs/File/file.h"                         // Per a les funcions de manipulació de fitxers
#include "../../Libs/Compress/so_compression.h"        // Per a les funcions de compressió

//Moduls de Worker
#include "../Modules/Exit/exit.h"                      // Per a les funcions de sortida del programa
#include "../Modules/Communication/communication.h"    // Per a les funcions de comunicació amb altres mòduls
#include "../Modules/Server/server.h"                  // Per a les funcions de servidor
#include "../Modules/Distortion/distortion.h"          // Per a les funcions de distorsió
#include "../Modules/MC/manage_client.h"               // Per a les funcions de gestió de clients
#include "../Modules/Context/context.h"                // Per a les funcions de context

//.h estructures
#include "../typeWorker.h"           // Per a les estructures de configuració de Worker

//Variables globals
volatile int exit_program = 0; 
volatile int exit_distortion = 0; 
pthread_mutex_t print_mutex = PTHREAD_MUTEX_INITIALIZER;           // Mutex per a la impressió per pantalla 
int gotham_socket = -1;

//Funcions

/*********************************************** 
* 
* @Finalidad: Manejar una señal específica en un hilo, utilizada para interrumpir 
*             métodos bloqueantes. Este handler no realiza ninguna operación. 
* 
* @Parámetros: 
* in: sig = Señal recibida (sin usar en el cuerpo de la función). 
* 
* @Retorno: Ninguno. 
* 
************************************************/
void handle_thread_signal(int sig __attribute__((unused))) {
    //handler buit per a interrompre mètodes bloquejants
}

/*********************************************** 
* 
* @Finalidad: Manejar la señal SIGINT (Ctrl+C), realizando tareas de limpieza y 
*             actualizando las banderas de salida del programa y de distorsión. 
* 
* @Parámetros: 
* in: sig = Señal recibida (sin usar en el cuerpo de la función). 
* 
* @Retorno: Ninguno. 
* 
************************************************/
void handle_sigint(int sig __attribute__((unused))) {
    STRING_printF(&print_mutex, STDOUT_FILENO, YELLOW, "\nReceived SIGINT (Ctrl+C), cleaning up...\n");
    exit_program = 1; 
    exit_distortion = 1; 
    SOCKET_closeSocket(&gotham_socket);
}

/*********************************************** 
* 
* @Finalidad: Punto de entrada principal del programa Harley. Inicializa la configuración, 
*             establece la conexión con Gotham, configura el servidor de workers, 
*             y gestiona la ejecución y limpieza de recursos. 
* 
* @Parámetros: 
* in: argc = Número de argumentos pasados al programa. 
* in: argv = Array de cadenas que contiene los argumentos. El segundo argumento debe ser 
*            la ruta al archivo de configuración. 
* 
* @Retorno: 
*           0 = Ejecución exitosa. 
*           EXIT_FAILURE = Error crítico durante la inicialización o ejecución. 
* 
************************************************/
int main(int argc, char** argv) {
    semaphore sHarleyCountMutex;     // Mutex per a accedir a comptador global de harleys

    WorkerConfig *harley_conf = NULL;
    WorkerServer *harley_server = NULL; 

    int created_semaphore = 0;       // Flag que indica si la regió de memòria associada al semàfor 'sHarleyCountMutex' ha sigut creada o ja existia
    int gotham_alive = 1; 
    pthread_t monitoring_thread;                // Thread per a la connexió al monitoreig de Gotham


    signal(SIGUSR1, handle_thread_signal);  // Configurem senyal SIGUSR1 per a interrompre mètodes bloquejants als threads principals
    signal(SIGINT, handle_sigint);          // Configurem handler de la senyal sigint

    STRING_initScreenMutex(print_mutex);

    if (argc != 2) {
        IO_printStatic(STDOUT_FILENO, "Navigate to the directory of the program you want to run and execute -> Harley <config_file>\n");
        exit(EXIT_FAILURE);
    }

    // Inicialitzem estructura de configuració
    harley_conf = (WorkerConfig*) malloc (sizeof(WorkerConfig));
    if(harley_conf == NULL) exit(EXIT_FAILURE);

    if(LOAD_loadConfigFile(argv[1], harley_conf, WORKER_CONF) == LOAD_FAILURE) {
        EXIT_freeMemory(&harley_conf, NULL);
        exit(EXIT_FAILURE);
    }
    LOAD_printConfig(harley_conf, WORKER_CONF);
    
    // Creem i establim connexió amb Gotham
    gotham_socket = SOCKET_initClientSocket(harley_conf->gotham_ip, harley_conf->gotham_port);
    if (gotham_socket < 0) {
        IO_printStatic(STDOUT_FILENO, RED "Failed to connect to Gotham. Exiting...\n" RESET);
        EXIT_freeMemory(&harley_conf, NULL);
        exit(EXIT_FAILURE);  // Si la connexió no és exitosa terminem el programa
    }

    // Establim connexió formal amb Gotham
    if (COMM_connectToGotham(gotham_socket, harley_conf) < 0) {
        IO_printStatic(STDOUT_FILENO, RED "Failed to connect to Gotham. Exiting...\n" RESET);
        close(gotham_socket);
        EXIT_freeMemory(&harley_conf, NULL);
        exit(EXIT_FAILURE);  // Sortir si la connexió no és exitosa
    }
    
    // Si hem establert correctament la connexió amb gotham, creem el semàfor necessari per manipular el comptador global de workers (mutex) i incrementem dit comptador
    SEM_constructor_with_name(&sHarleyCountMutex, ftok("../../Gotham/config.dat", 2), &created_semaphore);
    if(created_semaphore) SEM_init(&sHarleyCountMutex, 1);    // Si som el primer worker en crear el semàfor l'inicialitzem

    // Incrementem el comtpador global de harleys
    CONTEXT_updateWorkerCount(&sHarleyCountMutex, INC_WORKER_COUNTER, HARLEY_COUNTER);

    // Si gotham ens ha assignat com a worker principal, inicialitzem estructura de servidor i socket d'escolta
    harley_server = (WorkerServer*) malloc (sizeof(WorkerServer));
    if(harley_server == NULL) goto cleanup_harley; 

    // Configurem el socket d'escolta de flecks
    if(SRV_initWorkerServer(harley_server, harley_conf) < 0) goto cleanup_harley; 
    
    // Esperem assignació de worker principal
    int outcome = COMM_waitForMainWorkerAssignment(gotham_socket, &exit_program);
    if(outcome == COMM_GOTHAM_CRASHED || outcome == COMM_SIGINT_RECEIVED) {
        SOCKET_closeSocket(&harley_server->listen_socket);
        goto cleanup_harley; 
    }

    // Si hem rebut trama d'assignació de worker principal, com ja no rebem trames de gotham activament llancem thread de monitoreig per detectar caigudes
    MonitoringThreadArgs* gotham_monitoring_args = MONITOR_initMonitoringArgs(gotham_socket, &exit_program, &gotham_alive);
    if(gotham_monitoring_args == NULL) goto cleanup_harley; 

    if (pthread_create(&monitoring_thread, NULL, MONITOR_connectionMonitor, (void*)gotham_monitoring_args) != 0) {
        IO_printStatic(STDOUT_FILENO, RED "Error: Failed to create connection monitor thread.\n" RESET);
        free(gotham_monitoring_args);
        EXIT_cleanupMainWorker(harley_server);
        goto cleanup_harley; 
    }

    IO_printStatic(STDOUT_FILENO, YELLOW "\nHarley server initialized \n" RESET);
    IO_printStatic(STDOUT_FILENO, YELLOW "Waiting for connections…  \n" RESET); 

    SRV_runWorkerServer(harley_server, harley_conf->folder_path, &exit_program, &exit_distortion, &sHarleyCountMutex, 'm', &print_mutex); 
    
    // Només s'arribarà a aquest punt en cas que el worker sigui main; alliberem thread de 
    if(monitoring_thread) {
        pthread_kill(monitoring_thread, SIGUSR1); // Tancant el socket de gotham ja forçavem terminar el thread per com que el monitoreig es realitza cada 5 segons matem el thread per a donar resposta més ràpida a la caigua
        pthread_join(monitoring_thread, NULL);
    }
    EXIT_cleanupMainWorker(harley_server); // Tanquem socket d'escolta i esperem a que acabin els threads de distorsió

cleanup_harley:
    SOCKET_closeSocket(&gotham_socket);
    EXIT_freeMemory(&harley_conf, &harley_server); // Alliberem memòria asasociada a les estructures de configuració i servidor

    // Decrementem el comptador global de harleys
    int is_last_harley = CONTEXT_updateWorkerCount(&sHarleyCountMutex, DEC_WORKER_COUNTER, HARLEY_COUNTER);
    if(is_last_harley) {
        // Si el harley que acaba l'execució es tracta de l'últim connectat al sistema, eliminem les regions de memòria associades al comptador global i semàfor d'exclusió mutua
        STRING_printF(&print_mutex, STDOUT_FILENO, YELLOW, "I am the last harley connected to Mr.J.System, cleaning up shared memory...\n");
        SEM_destructor(&sHarleyCountMutex);
    }

    return 0;
}