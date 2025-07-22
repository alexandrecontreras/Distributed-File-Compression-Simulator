/*********************************************** 
* 
* @Autores: Alexandre Contreras y Armand López
* @Propósito: Proveer la implementación del programa Fleck, gestionando la configuración, 
*             conexiones con Gotham y workers, procesos de distorsión de archivos, 
*             manejo de comandos del usuario, y limpieza segura de recursos antes de finalizar. 
* @Fecha de creación: 18 de octubre de 2024
* @Última modificación: 4 de enero de 2025
* 
************************************************/

//Constant del sistema
#define _GNU_SOURCE

//Llibreries del sistema
#include <stdio.h>
#include <signal.h>
#include <pthread.h>

//Llibreries pròpies
#include "../Libs/IO/io.h"                          // Per a les funcions d'entrada/sortida
#include "../Libs/String/string.h"                  // Per a les funcions de manipulació de strings
#include "../Libs/Dir/dir.h"                        // Per a les funcions de manipulació de directoris
#include "../Libs/Load/load_config.h"               // Per a les funcions de càrrega de fitxers de configuració
#include "../Libs/Socket/socket.h"                  // Per a les funcions de connexió per sockets
#include "../Libs/Monitor/monitor.h"                // Per a les funcions de monitorització
#include "../Libs/File/file.h"                      // Per a les funcions de manipulació de fitxers
#include "../Libs/Communication/communication.h"    // Per a les funcions de comunicació amb Gotham, Enigma i Harley

//Moduls de Fleck
#include "Modules/CMD/cmd.h"                        // Per a les funcions de comandes
#include "Modules/Exit/exit.h"                      // Per a les funcions de sortida del programa
#include "Modules/Communication/communication.h"    // Per a les funcions de comunicació amb Gotham, Enigma i Harley
#include "Modules/Distortion/distortion.h"          // Per a les funcions de distorsió de fitxers

//.h estructures
#include "../Libs/Structure/typeDistort.h"                 // Per a les estructures de distorsió de text i media
#include "../Libs/Structure/typeMonitor.h"                 // Per a les estructures de monitorització
#include "typeFleck.h"                                     // Per a les estructures de configuracio de Fleck

//Variables globals
int gotham_socket = -1;      

volatile int exit_distortion = 0;                           // Variable global per a forçar la terminació de threads
volatile int exit_program_flag = 0;                         // Variable global per controlar la sortida del programa, en el cas de Ctrl+C, GothamCrash o Logout

pthread_mutex_t print_mutex = PTHREAD_MUTEX_INITIALIZER;    // Mutex per a la impressió per pantalla

/*********************************************** 
* 
* @Finalidad: Procesar un comando de distorsión, validando su formato, verificando la existencia y tipo del archivo, e iniciando el proceso
            de distorsión correspondiente (ya sea de texto o media). 
* 
* @Parámetros: 
* in: cmd = Comando de distorsión en formato de cadena que incluye el nombre del archivo y el factor de distorsión. 
* in: fleck_config = Puntero a la estructura `FleckConfig` que contiene la configuración del fleck
* in: distortion_context = Array que contiene las estructuras de contexto de distorsión de texto y de media
* in: main_worker = Array que contiene las estructuras `MainWorker` para Harley y Enigma.
* in: distortion_threads = Array que contiene los hilos de distorsión de texto y de media
* in/out: distortion_record = Puntero a la estructura `DistortionRecord` que almacena el historial de distorsiones ralizadas y en curso.
* in/out: connected_to_gotham = Puntero a un valor entero que indica si hay una conexión activa con el servidor Gotham (1 si está conectado, 0 si no lo está).
* in/out: distorting_flag = Array que contiene los flags de distorsión de texto y media en curso
* 
* @Retorno: ---
* 
************************************************/
void processDistortionCommand(char* cmd, FleckConfig* fleck_config, DistortionContext distortion_context[], MainWorker main_worker[], pthread_t distortion_threads[], DistortionRecord* distortion_record, int* connected_to_gotham, int distorting_flag[], int* finished_distortion) {
    if (!(*connected_to_gotham)) {
        STRING_printF(&print_mutex, STDOUT_FILENO, RED, "Cannot distort, you are not connected to Mr. J System.\n");
        return;
    }

    char *cmd_copy = strdup(cmd);
    char *filename = NULL;
    char *extension = NULL;
    int factor = 0;

    // Validem la comanda i extraiem el nom del fitxer i factor introduïts
    if (!CMD_isDistortCommandValid(cmd_copy, &filename, &factor, &print_mutex)) {
        STRING_printF(&print_mutex, STDOUT_FILENO, RED, "Error: Invalid distortion command\n");
        goto cleanup;
    }

    if(!DIR_checkDistortedFile(fleck_config->folder_path, filename, &print_mutex)) {
        STRING_printF(&print_mutex, STDOUT_FILENO, RED, "Error: The file %s is already distorted\n", filename);
        goto cleanup;
    }

    if (!DIR_fileExistsInFolder(fleck_config->folder_path, filename, &print_mutex)) {
        STRING_printF(&print_mutex, STDOUT_FILENO, RED, "Error: The file does not exist in the fleck folder\n");
        goto cleanup;
    }

    extension = FILE_determineFileType(filename, &print_mutex);
    if (extension == NULL || strcmp(extension, "Unknown") == 0) {
        STRING_printF(&print_mutex, STDOUT_FILENO, RED, "Error: The file format is not valid\n");
        goto cleanup;
    }

    // Gestionar distorsió de text
    if (strcmp(extension, "Text") == 0) {
        if (!DIST_prepareAndStartDistortion(&distortion_context[TEXT], filename, fleck_config->username, "Text", &distortion_threads[TEXT], factor, &distorting_flag[TEXT], &main_worker[TEXT], gotham_socket, fleck_config->folder_path, distortion_record, &exit_distortion, &finished_distortion[TEXT], &print_mutex)) {
            goto cleanup;
        }
    }

    // Gestionar distorsió de media
    if (strcmp(extension, "Media") == 0) {
        if (!DIST_prepareAndStartDistortion(&distortion_context[MEDIA], filename, fleck_config->username, "Media", &distortion_threads[MEDIA], factor, &distorting_flag[MEDIA], &main_worker[MEDIA], gotham_socket, fleck_config->folder_path, distortion_record, &exit_distortion, &finished_distortion[MEDIA], &print_mutex)) {
            goto cleanup;
        }
    }

cleanup:
    free(cmd_copy);
    free(filename);
    free(extension);
}

/*********************************************** 
* 
* @Finalidad: Establecer una conexión con el servidor Gotham, incluyendo la creación 
*             de un socket, la conexión formal al servidor y la inicialización de un 
*             hilo para monitorear la conexión. 
* 
* @Parámetros: 
* in/out: cmd = Puntero a la cadena del comando que se libera en caso de error. 
* in: gotham_alive = Puntero a un valor entero que indica si el servidor Gotham está activo (1 si está vivo, 0 si no).
* in/out: monitor_thread = Puntero a la estructura `pthread_t` donde se almacenará el identificador del hilo encargado de monitorear la conexión con Gotham.
* in: fleck_config = Puntero a la estructura `FleckConfig` que contiene la configuración de fleck, incluyendo la IP y el puerto de Gotham.
* in: distortion_context = Array que contiene las estructuras de contexto de distorsión para los tipos de archivo (texto y media).
* in: main_worker = Array que contiene las estructuras `MainWorker` para Harley y Enigma.
* in: distortion_record = Puntero a la estructura `DistortionRecord` que almacena el historial de distorsiones realizadas y en curso.
* in/out: connected_to_gotham = Puntero a un valor entero que indica si hay una conexión activa con el servidor Gotham (1 si está conectado, 0 si no lo está).
* 
* @Retorno: ---
************************************************/
void connectToServer(char **cmd, int *gotham_alive, pthread_t *monitor_thread, FleckConfig* fleck_config, int* connected_to_gotham) {
    // Comprovar si ja estem connectats a Gotham
    if (*connected_to_gotham) {
        STRING_printF(&print_mutex, STDOUT_FILENO, RED, "Error: You are already connected to Gotham.\n");
        return;
    }

    // Crear i establir connexió amb gotham
    gotham_socket = SOCKET_initClientSocket(fleck_config->gotham_ip, fleck_config->gotham_port);
    if (gotham_socket < 0) {
        IO_printStatic(STDOUT_FILENO, RED "Failed to connect to Gotham.\n" RESET);
        free(*cmd); 
        *cmd = NULL; 
        return;
    }

    // Establir connexió formal amb Gotham
    if (COMM_connectToGotham(gotham_socket, fleck_config) < 0) {
        IO_printStatic(STDOUT_FILENO, RED "Failed to connect to Gotham.\n" RESET);
        free(*cmd); 
        *cmd = NULL; 
    }
    else {
        *connected_to_gotham = 1; 
        MonitoringThreadArgs* monitor_args = MONITOR_initMonitoringArgs(gotham_socket, &exit_program_flag, gotham_alive);
        if(!monitor_args) return;

        // Llancem thread que s'encarrega ÚNICAMENT de detectar caigudes de gotham
        if (pthread_create(monitor_thread, NULL, MONITOR_connectionMonitor, (void*)monitor_args) != 0) {
            free(monitor_args);
            IO_printStatic(STDOUT_FILENO, RED "Error: Failed to create connection monitor thread\n" RESET);
        }
    }
}

/*********************************************** 
* 
* @Finalidad: Calcular y devolver el progreso actual de la distorsión de un archivo, 
*             expresado como un porcentaje basado en el número de paquetes procesados 
*             y el estado actual del proceso. 
* 
* @Parámetros: 
* in: file_type = Tipo de archivo cuya distorsión se está procesando (`TEXT` o `MEDIA`). 
*             Este valor determina qué contexto de distorsión (texto o media) se usará para calcular el progreso.
* in: distortion_context = Array que contiene las estructuras de contexto de distorsión para los tipos de archivo (texto y media).
* 
* @Retorno: 
* Retorna un valor de tipo `float` que representa el porcentaje de progreso de la distorsión, 
*          expresado en un rango de 0 a 100. 
************************************************/
float getDistortionProgress(int file_type, DistortionContext distortion_context[]) {
    float processed_packets;
    float total_packets;
    int distortion_stage; 

    processed_packets = file_type == TEXT ? (float) distortion_context[TEXT].n_processed_packets : (float) distortion_context[MEDIA].n_processed_packets;
    total_packets = file_type == TEXT ? (float) distortion_context[TEXT].n_packets : (float) distortion_context[MEDIA].n_packets; 
    distortion_stage = file_type == TEXT? distortion_context[TEXT].current_stage : distortion_context[MEDIA].current_stage;

    if(distortion_stage == STAGE_SND_FILE || distortion_stage == STAGE_RCV_METADATA) {
        return (processed_packets * 50) / total_packets;
    }
    else {
        return 50 + (processed_packets * 50 / total_packets);
    }
}

/*********************************************** 
* 
* @Finalidad: Verificar y mostrar el estado actual de las distorsiones en curso o finalizadas, 
*             incluyendo progreso, éxito o fallo de cada archivo. La información es 
*             impresa en la salida estándar. 
* 
* @Parámetros: 
* in: distortion_record = Puntero a la estructura `DistortionRecord` que contiene el historial de distorsiones 
*                         realizadas y en curso, incluyendo el estado de cada archivo (completado, fallido, en progreso).
* in: distortion_context = Array de estructuras `DistortionContext` que contienen la información sobre el progreso 
*                          de la distorsión para cada tipo de archivo (texto o media).
* 
* @Retorno: 
* Ninguno. La función imprime directamente los resultados del estado de las distorsiones en curso o finalizadas en 
*          la salida estándar. Si no hay distorsiones, se muestra un mensaje informativo.
* 
************************************************/
void checkStatus(DistortionRecord* distortion_record, DistortionContext distortion_context[]) {
    int status; 
    int file_type;
    char* filename;
    float progress_percentage;
    int n_progress_chars; 

    if(distortion_record->n_distortions == 0) {
        STRING_printF(&print_mutex, STDOUT_FILENO, YELLOW, "You have no ongoing or finished distortions\n"); 
        return;
    }
    for(int i = 0; i < distortion_record->n_distortions; i++) {
        status = distortion_record->distortions[i].status;
        file_type = distortion_record->distortions[i].file_type;
        filename = distortion_record->distortions[i].filename;
        if(status == COMPLETED) {
            STRING_printF(&print_mutex, STDOUT_FILENO, GREEN, "%s\t\t100%% |====================|\n", filename); 
        } 
        else if(status == FAILED){
            STRING_printF(&print_mutex, STDOUT_FILENO, RED, "%s\t\tFAILED TO DISTORT\n", filename); 
        } 
        else {
            progress_percentage = getDistortionProgress(file_type, distortion_context);
            STRING_printF(&print_mutex, STDOUT_FILENO, YELLOW, "%s\t\t%d%%  |", filename, (int)progress_percentage);
            n_progress_chars = (int) (progress_percentage * 20.0f / 100.0f);
            for(int j = 0; j < 20; j++) {
                if(j < n_progress_chars) {
                    STRING_printF(&print_mutex, STDOUT_FILENO, YELLOW, "="); 
                }
                else {
                    STRING_printF(&print_mutex, STDOUT_FILENO, YELLOW, " "); 
                }
            }
            STRING_printF(&print_mutex, STDOUT_FILENO, YELLOW, "|\n"); 
        }
    }
}

/*********************************************** 
* 
* @Finalidad: Manejar y ejecutar la acción correspondiente a un comando ingresado por el usuario, realizando las operaciones asociadas 
*             al sistema en función del comando recibido. 
* 
* @Parámetros: 
* in/out: cmd = Cadena que contiene el comando a procesar.
* in/out: gotham_alive = Puntero a un valor entero que indica si el servidor Gotham está activo (1 si está vivo, 0 si no).
* in/out: monitor_thread = Puntero a la estructura `pthread_t` donde se almacenará el identificador del hilo 
*                          encargado de monitorear la conexión con Gotham.
* in: fleck_config = Puntero a la estructura `FleckConfig` que contiene la configuración de fleck
* in: distortion_context = Array de estructuras `DistortionContext` que contienen la información sobre el progreso 
*                          de la distorsión para cada tipo de archivo (texto y media).
* in: main_worker = Array que contiene las estructuras `MainWorker` para Harley y Enigma.
* in/out: distortion_record = Puntero a la estructura `DistortionRecord` que almacena el historial de distorsiones realizadas y en curso.
* in/out: distortion_threads = Array que contiene los identificadores de los hilos de distorsión de texto y de media.
* in/out: connected_to_gotham = Puntero a un valor entero que indica si hay una conexión activa con el servidor Gotham (1 si está conectado, 0 si no).
* in/out: distorting_flag = Array que contiene los flags de distorsión en curso de tipo texto y media.
* 
* @Retorno: ---
* 
************************************************/
void commandHandler(char* cmd, int *gotham_alive, pthread_t* monitor_thread, FleckConfig* fleck_config, DistortionContext distortion_context[], MainWorker main_worker[], DistortionRecord* distortion_record, pthread_t distortion_threads[], int* connected_to_gotham, int distorting_flag[], int* finished_distortion) {
    
    STRING_toLowerCase(cmd);

    switch (CMD_changeComandToNumber(cmd, &print_mutex)) {
        case CMD_CONNECT:
            connectToServer(&cmd, gotham_alive, monitor_thread, fleck_config, connected_to_gotham);
            break;
        case CMD_LOGOUT:
            exit_program_flag = 1;
            STRING_printF(&print_mutex, STDOUT_FILENO, YELLOW, "Thanks for using Mr. J System, see you soon, chaos lover :)\n");
            break;
        case CMD_LISTMEDIA:
            DIR_printTextDirectory("Media", fleck_config->folder_path, &print_mutex);
            break;
        case CMD_LISTTEXT:
            DIR_printTextDirectory("Text", fleck_config->folder_path, &print_mutex);
            break;
        case CMD_DISTORT:
            processDistortionCommand(cmd, fleck_config, distortion_context, main_worker, distortion_threads, distortion_record, connected_to_gotham, distorting_flag, finished_distortion); 
            break;
        case CMD_CHECKSTATUS:
            checkStatus(distortion_record, distortion_context);
            break;
        case CMD_CLEARALL:
            EXIT_freeDistortionRecord(distortion_record);
            break;
        default:
            STRING_printF(&print_mutex, STDOUT_FILENO, RED, "ERROR: Please input a valid command.\n");
            break;
    }

    if(cmd) free(cmd);

}

/*********************************************** 
* 
* @Finalidad: Manejar señales enviadas al hilo para interrumpir métodos bloqueantes. 
*             Este handler no realiza ninguna operación. 
* 
* @Parámetros: 
* in: sig = Señal recibida (no utilizada en el handler). 
* 
* @Retorno: Ninguno. 
* 
************************************************/
void handle_thread_signal(int sig __attribute__((unused))) {
    //Handler buit per a interrompre mètodes bloquejants
}

/*********************************************** 
* 
* @Finalidad: Manejar la señal SIGINT (Ctrl+C) para realizar tareas de limpieza, 
*             incluyendo cerrar el socket y configurar banderas de salida. 
* 
* @Parámetros: 
* in: sig = Señal recibida (no utilizada en el handler). 
* 
* @Retorno: Ninguno. 
* 
************************************************/
void sigint_handler(int sig __attribute__((unused))) {
    STRING_printF(&print_mutex, STDOUT_FILENO, YELLOW, "\nReceived SIGINT (Ctrl+C), cleaning up...\n");

    exit_program_flag = 1;
    exit_distortion = 1; 
    SOCKET_closeSocket(&gotham_socket);
}

/*********************************************** 
* 
* @Finalidad: Terminar el hilo de monitoreo de Gotham y cerrar el socket de conexión. 
* 
* @Parámetros: 
* in/out: monitor_thread = Puntero al identificador del hilo de monitoreo que será terminado. 
* 
* @Retorno: Ninguno. 
* 
************************************************/
void terminateMonitoringThread(pthread_t* monitor_thread) {
    SOCKET_closeSocket(&gotham_socket);
    if(*monitor_thread) {
        pthread_kill(*monitor_thread, SIGUSR1); 
        pthread_join(*monitor_thread, NULL);
    }
}

/*********************************************** 
* 
* @Finalidad: Manejar la desconexión de Gotham al finalizar un proceso de distorsión, 
*             asegurándose de cerrar el socket y el hilo de monitoreo si no hay otras 
*             distorsiones en curso. 
* 
* @Parámetros: 
* in: distorting_flag[] = Array de banderas que indican si hay procesos de distorsión en curso. 
* in: fleck_config = Configuración del fleck (no utilizado en esta función). 
* in/out: monitor_thread = Puntero al identificador del hilo de monitoreo que será terminado si se cierra la conexión. 
* in/out: finished_distortion = Array que indica los estados de finalización de los procesos de distorsión. 
* in: finished_type = Tipo de distorsión que ha finalizado (`TEXT` o `MEDIA`). 
* in/out: connected_to_gotham = Bandera que indica si la conexión con Gotham sigue activa. 
* 
* @Retorno: Ninguno. 
* 
************************************************/
void handleGothamDisconnection(int distorting_flag[], FleckConfig fleck_config __attribute__((unused)),pthread_t* monitor_thread, int* finished_distortion, int finished_type, int* connected_to_gotham) {
    int alternate_type = finished_type == TEXT ? MEDIA : TEXT;
    // Si la distorsió oposada a la que ha acabat no es troba en progrés tanquem la connexió amb gotham
    if(!distorting_flag[alternate_type]) {
        SOCKET_closeSocket(&gotham_socket);
        terminateMonitoringThread(monitor_thread);
        *connected_to_gotham = 0;
    }
    finished_distortion[finished_type] = 0;
}

/*********************************************** 
* 
* @Finalidad: Punto de entrada principal del programa. Inicializa señales, carga la 
*             configuración, maneja los comandos del usuario y realiza tareas de limpieza 
*             antes de finalizar. 
* 
* @Parámetros: 
* in: argc = Número de argumentos pasados al programa. Debe ser 2 para una ejecución válida. 
* in: argv = Array de cadenas que contiene los argumentos. El segundo argumento debe ser 
*            la ruta al archivo de configuración de Fleck. 
* 
* @Retorno: 
*           0 = Ejecución exitosa. 
*           EXIT_FAILURE = Error crítico durante la inicialización o ejecución. 
* 
************************************************/
int main(int argc, char** argv) {
    char* command = NULL;

    int gotham_alive = 1; 
    pthread_t monitor_thread = 0;               // Thread per a la connexió al monitoreig de Gotham
    
    pthread_t distortion_threads[2] = {0, 0};   // Threads per a distorsió de text i media respectivament
    FleckConfig fleck_config;                   // Variable per a la configuració de Fleck
    DistortionContext distortion_context[2] = {{NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0}, {NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0}};
    MainWorker main_worker[2] = {{NULL, -1, -1}, {NULL, -1, -1}};
    DistortionRecord distortion_record = {0, NULL}; 
    int distorting_flag[2] = {0, 0};
    int finished_distortion[2] = {0, 0};
    int connected_to_gotham = 0; 

    signal(SIGUSR1, handle_thread_signal); 
    signal(SIGINT, sigint_handler);

    STRING_initScreenMutex(print_mutex);

    if (argc != 2) {
        IO_printStatic(STDOUT_FILENO, "Navigate to the directory of the program you want to run and execute-> Fleck <config_file>\n");
        exit(EXIT_FAILURE);
    }

    //Carregar configuració del fitxer
    if(LOAD_loadConfigFile(argv[1], &fleck_config, FLECK_CONF) == LOAD_FAILURE) exit(EXIT_FAILURE);
    LOAD_printConfig(&fleck_config, FLECK_CONF);

    while (!exit_program_flag) {
        STRING_printF(&print_mutex, STDOUT_FILENO, RESET, "$ ");
        command = IO_nonBlockingReadUntil(STDIN_FILENO, '\n', &exit_program_flag, &finished_distortion[TEXT], &finished_distortion[MEDIA]);
        if(!command) {
            if(exit_program_flag) break;
            handleGothamDisconnection(distorting_flag, fleck_config, &monitor_thread, finished_distortion, finished_distortion[TEXT] ? TEXT : MEDIA, &connected_to_gotham);
            continue;
        } 
        commandHandler(command, &gotham_alive, &monitor_thread, &fleck_config, distortion_context, main_worker, &distortion_record, distortion_threads, &connected_to_gotham, distorting_flag, finished_distortion);
    }
    
    for(int i = 0; i < 2; i++) {
        pthread_join(distortion_threads[i], NULL);
    }

    terminateMonitoringThread(&monitor_thread);

    EXIT_freeMemory(&fleck_config, &distortion_context[TEXT], &distortion_context[MEDIA], &main_worker[TEXT], &main_worker[MEDIA], &distortion_record);
    STRING_destroyScreenMutex(print_mutex);
    return 0;
}