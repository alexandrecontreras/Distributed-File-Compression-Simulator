/***********************************************
*
* @Autores: Alexandre Contreras, Armand López.
* 
* @Finalidad: Implementar funciones para monitorear la conexión con Gotham y gestionar 
*             las estructuras necesarias para el monitoreo en un entorno multihilo.
* 
* @Fecha de creación: 11 de octubre de 2024.
* 
* @Última modificación: 4 de enero de 2025.
* 
************************************************/

#include "monitor.h"

/*********************************************** 
* 
* @Finalidad: Monitorear la conexión con Gotham, detectando desconexiones o problemas 
*             en el socket y actualizando las banderas de estado en consecuencia. 
* 
* @Parámetros: 
* in: args = Puntero a la estructura `MonitoringThreadArgs` que contiene los argumentos necesarios 
*            para monitorear la conexión (e.g., socket, banderas de estado). 
* 
* @Retorno: 
*           NULL = Finaliza el hilo de monitoreo tras detectar una desconexión o error crítico. 
* 
************************************************/
void* MONITOR_connectionMonitor(void* args) {
    MonitoringThreadArgs* monitor_args = (MonitoringThreadArgs*) args;
    fd_set readfds;
    struct timeval timeout;
    int result;
    volatile int* exit_program = monitor_args->exit_program_flag;
    int* gotham_alive = monitor_args->gotham_alive;
    int gotham_socket = monitor_args->gotham_socket;
    
    while (*gotham_alive && !*exit_program) {
        FD_ZERO(&readfds);
        FD_SET(gotham_socket, &readfds);
        timeout.tv_sec = 5;
        timeout.tv_usec = 0;

        result = select(gotham_socket + 1, &readfds, NULL, NULL, &timeout);

        if (result == -1) {
            IO_printStatic(STDOUT_FILENO, PURPLE "Error: Failed select in monitoring thread\n" RESET);
            break;
        } else if (result == 0) {
            // Timeout
            continue;
        }

        if (FD_ISSET(gotham_socket, &readfds)) {
            char buffer[1];
            ssize_t bytes_peeked = recv(gotham_socket, buffer, sizeof(buffer), MSG_PEEK); // Llegim dades sense eliminar-les del buffer

            if (bytes_peeked == 0) {
                IO_printStatic(STDOUT_FILENO, PURPLE "Error: Gotham connection lost\n" RESET);
                *gotham_alive = 0;
            } else if (bytes_peeked < 0) {
                if (errno != EAGAIN && errno != EWOULDBLOCK) {
                    IO_printStatic(STDOUT_FILENO, PURPLE "Error: Failed to peek from gotham socket\n" RESET);
                    break;
                }
            }
        }
    }

    if (!*gotham_alive) *exit_program = 1; 
    free(monitor_args);
    close(gotham_socket); 
    return NULL;
}

/*********************************************** 
* 
* @Finalidad: Inicializar y asignar memoria para una estructura `MonitoringThreadArgs`, 
*             configurando los argumentos necesarios para el monitoreo de la conexión. 
* 
* @Parámetros: 
* in: gotham_socket = Descriptor del socket de Gotham que será monitoreado. 
* in: exit_program = Puntero a una bandera `volatile int` que indica si el programa debe finalizar. 
* in: gotham_alive = Puntero a una bandera que indica si la conexión con Gotham está activa. 
* 
* @Retorno: 
*           Puntero a la estructura `MonitoringThreadArgs` inicializada. 
*           Retorna NULL si ocurre un error al asignar memoria. 
* 
************************************************/
MonitoringThreadArgs* MONITOR_initMonitoringArgs(int gotham_socket, volatile int* exit_program, int* gotham_alive) {
    MonitoringThreadArgs* args = (MonitoringThreadArgs*) malloc (sizeof(MonitoringThreadArgs));
    if (args == NULL) return NULL;

    args->exit_program_flag = exit_program;
    args->gotham_socket = gotham_socket;
    args->gotham_alive = gotham_alive; 

    return args;
}