
/***********************************************
*
* @Autores: Alexandre Contreras, Armand López.
* 
* @Finalidad: Proveer funciones para cargar configuraciones desde archivos y mostrar 
*             información de configuración en formato estructurado.
* 
* @Fecha de creación: 11 de noviembre de 2024.
* 
* @Última modificación: 4 de enero de 2025.
* 
************************************************/

#include "load_config.h"

/*********************************************** 
* 
* @Finalidad: Imprimir la configuración de una estructura especificada según su tipo. 
* 
* @Parámetros: 
* in: config_struct = Puntero a la estructura de configuración. 
* in: type = Tipo de configuración (e.g., `FLECK_CONF`, `GOTHAM_CONF`, `WORKER_CONF`). 
* 
* @Retorno: Ninguno. 
* 
************************************************/
void LOAD_printConfig(void* config_struct, int type) {
    switch (type) {
        case FLECK_CONF: 
            FleckConfig* fleck_config = (FleckConfig*) config_struct;
            IO_printFormat(STDOUT_FILENO, "%s user initialized\n\n", fleck_config->username);
            IO_printFormat(STDOUT_FILENO, "User - %s\n", fleck_config->username);
            IO_printFormat(STDOUT_FILENO, "Directory - %s\n", fleck_config->folder_path);
            IO_printFormat(STDOUT_FILENO, "IP - %s\n", fleck_config->gotham_ip);
            IO_printFormat(STDOUT_FILENO, "Port - %d\n\n", fleck_config->gotham_port);
            break; 

        case GOTHAM_CONF:
            GothamConfig* gotham_config = (GothamConfig*) config_struct;
            IO_printFormat(STDOUT_FILENO, "\nFleck IP: %s\n", gotham_config->fleck_ip);
            IO_printFormat(STDOUT_FILENO, "Fleck Port: %d\n", gotham_config->fleck_port);
            IO_printFormat(STDOUT_FILENO, "Worker IP: %s\n", gotham_config->worker_ip);
            IO_printFormat(STDOUT_FILENO, "Worker Port: %d\n", gotham_config->worker_port);
            break; 

        case WORKER_CONF:
            WorkerConfig* worker_config = (WorkerConfig*) config_struct;
            IO_printFormat(STDOUT_FILENO, "Gotham IP: %s\n", worker_config->gotham_ip);
            IO_printFormat(STDOUT_FILENO, "Gotham Port: %d\n", worker_config->gotham_port);
            IO_printFormat(STDOUT_FILENO, "Fleck IP: %s\n", worker_config->worker_ip);
            IO_printFormat(STDOUT_FILENO, "Fleck Port: %d\n", worker_config->worker_port);
            IO_printFormat(STDOUT_FILENO, "Folder Path: %s\n", worker_config->folder_path);
            IO_printFormat(STDOUT_FILENO, "Worker Type: %s\n", worker_config->worker_type);
            break; 

        default:

            break; 
    }
}

/*********************************************** 
* 
* @Finalidad: Leer un archivo de configuración y cargar sus valores en una estructura 
*             específica según su tipo. 
* 
* @Parámetros: 
* in: filename = Nombre del archivo de configuración. 
* in/out: config_struct = Puntero a la estructura donde se almacenará la configuración cargada. 
* in: type = Tipo de configuración (e.g., `FLECK_CONF`, `GOTHAM_CONF`, `WORKER_CONF`). 
* 
* @Retorno: 
*           LOAD_SUCCESS = La configuración fue cargada correctamente. 
*           LOAD_FAILURE = Error al abrir, leer o cerrar el archivo de configuración. 
* 
************************************************/
int LOAD_loadConfigFile(char* filename, void* config_struct, int type) {
    char* port_str = NULL;
    int fd_file = open(filename, O_RDONLY);
    if(fd_file < 0) {
        IO_printStatic(STDOUT_FILENO, "Could not open config file\n");
        return LOAD_FAILURE;
    }
         
    switch (type) {
        case FLECK_CONF:
            FleckConfig* fleck_config = (FleckConfig*) config_struct;
            fleck_config->username = IO_readUntil(fd_file, '\n');
            STRING_checkCharacterAmpersand(fleck_config->username);
            fleck_config->folder_path = IO_readUntil(fd_file, '\n');
            fleck_config->gotham_ip = IO_readUntil(fd_file, '\n');
            port_str = IO_readUntil(fd_file, '\n');
            fleck_config->gotham_port = atoi(port_str);
            free(port_str);
            break;

        case GOTHAM_CONF: 
            GothamConfig* gotham_config = (GothamConfig*) config_struct;
            gotham_config->fleck_ip = IO_readUntil(fd_file, '\n');
            port_str = IO_readUntil(fd_file, '\n');
            gotham_config->fleck_port = atoi(port_str);
            free(port_str);
            gotham_config->worker_ip = IO_readUntil(fd_file, '\n');
            port_str = IO_readUntil(fd_file, '\n');
            gotham_config->worker_port = atoi(port_str);
            free(port_str);
            break; 

        case WORKER_CONF:
            WorkerConfig* worker_config = (WorkerConfig*) config_struct;
            worker_config->gotham_ip = IO_readUntil(fd_file, '\n');
            port_str = IO_readUntil(fd_file, '\n');
            worker_config->gotham_port = atoi(port_str); 
            free(port_str);
            worker_config->worker_ip = IO_readUntil(fd_file, '\n');
            port_str = IO_readUntil(fd_file, '\n');
            worker_config->worker_port = atoi(port_str); 
            free(port_str);
            worker_config->folder_path = IO_readUntil(fd_file, '\n');
            worker_config->worker_type = IO_readUntil(fd_file, '\n');
            break;

        default:

            break; 
    }

    if (close(fd_file) == -1) {
        IO_printStatic(STDOUT_FILENO, "Error closing file descriptor\n");
        return LOAD_FAILURE;
    }

    return LOAD_SUCCESS; 
} 


