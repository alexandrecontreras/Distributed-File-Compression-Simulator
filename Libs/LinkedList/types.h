/*********************************************** 
* 
* @Autores: Alexandre Contreras y Armand López
* @Propósito: Definir las estructuras `Fleck` y `Worker` para representar 
*             clientes conectados al sistema, proporcionando campos esenciales 
*             para la identificación y gestión de las conexiones. 
* @Fecha de creación: 15 de octubre de 2024
* @Última modificación: 4 de enero de 2025
* 
************************************************/

#ifndef STRUCT_H
#define STRUCT_H

typedef struct {
    char* username;  
    char* ip;         
    int port;       
    int socket_fd;       
} Fleck;

typedef struct {
    char* worker_type; 
    char* ip;           
    int port;         
    int socket_fd; 
    int is_main;        
} Worker;

#endif
