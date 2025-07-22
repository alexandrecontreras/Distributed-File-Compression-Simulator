/*********************************************** 
* 
* @Autores: Alexandre Contreras y Armand López
* @Propósito: Definir las estructuras necesarias para la monitorización de la conexión 
*             con el servidor Gotham, permitiendo detectar desconexiones y gestionar 
*             el estado de salida del programa.
* @Fecha de creación: 20 de octubre de 2024
* @Última modificación: 4 de enero de 2025
* 
************************************************/

#ifndef _TYPE_MONITOR_CUSTOM_H_
#define _TYPE_MONITOR_CUSTOM_H_

typedef struct {
    int gotham_socket; 
    volatile int* exit_program_flag; 
    int* gotham_alive;
} MonitoringThreadArgs;

#endif // _TYPE_MONITOR_CUSTOM_H_