/*********************************************** 
* 
* @Autores: Alexandre Contreras, Armand López. 
* 
* @Finalidad: Implementar las funciones necesarias para manejar las conexiones, 
*             solicitudes y desconexiones en el servidor Gotham, incluyendo la 
*             validación de atributos, asignación de workers principales y gestión 
*             de solicitudes de distorsión. 
* 
* @Fecha de creación: 30 de octubre de 2024. 
* 
* @Última modificación: 4 de enero de 2025. 
* 
************************************************/

#include "handle.h"


/*********************************************** 
* 
* @Finalidad: Proporcionar a un fleck los detalles del worker principal asignado 
*             para el tipo de media solicitado, incluyendo IP y puerto. En caso de 
*             no encontrar un worker adecuado, se envía una respuesta de error. 
* 
* @Parámetros: 
* in/out: server = Puntero a la estructura `GothamServer` que contiene la lista de workers conectados. 
* in: client_socket = Descriptor del socket del fleck que solicita los detalles. 
* in: mediaType = Tipo de media solicitado (e.g., "Text" o "Media"). 
* in: type = Tipo de trama para la respuesta (e.g., código del protocolo). 
* 
* @Retorno: Ninguno. 
* 
************************************************/
void HANDLE_provideFleckMainWorkerDetails(GothamServer *server, int client_socket, char *mediaType, int type) {
    char *worker_ip = NULL;	
    int worker_port;

    WORKER_LINKEDLIST_goToHead(&server->worker_list);
    int found = 0;
    
    while(!WORKER_LINKEDLIST_isAtEnd(server->worker_list)) {
        WorkerElement worker = WORKER_LINKEDLIST_get(&server->worker_list);
        
        if (strcmp(worker.worker_type, mediaType) == 0) {
            if (worker.is_main) {
                found = 1;  
                worker_ip = strdup(worker.ip); //fem copia de l'atribut original per a que el punter worker_ip passat per parametres no referencii el contingut original
                worker_port = worker.port;
                break;      
            }
        }

        WORKER_LINKEDLIST_next(&server->worker_list);
    }

    if (found) {
        // Crear trama amb IP i port del worker, Mirar lo de nptioms y cosas d estas
        char *data;

        if (asprintf(&data, "%s&%d", worker_ip, worker_port) == -1) {
            IO_printStatic(STDOUT_FILENO, "Error: Could not create connection string\n");
            free(worker_ip);
            if (data) {
                free(data);
            }
            return;
        }
        COMM_sendDistortResponse(client_socket, data, 1, type);   
        IO_printStatic(STDOUT_FILENO, YELLOW "Forwarding worker connection details...\n" RESET);
        free(worker_ip); 
        free(data);                  
    }
    else {
        // Crear trama de error
        IO_printFormat(STDOUT_FILENO, RED "No workers of type %s found\n" RESET, mediaType);
        COMM_sendDistortResponse(client_socket, "DISTORT_KO", 0, type);  //KO
    }
    return;
}

/*********************************************** 
* 
* @Finalidad: Validar los atributos de una solicitud de distorsión, verificando el tipo de media, 
*             el nombre del archivo, y si la extensión es válida para el tipo especificado. 
* 
* @Parámetros: 
* in/out: data_buffer = Cadena que contiene los datos de la solicitud a validar. 
* in/out: mediaType = Puntero que recibirá el tipo de media extraído ("Media" o "Text"). 
* in/out: fileName = Puntero que recibirá el nombre del archivo extraído. 
* 
* @Retorno: 
*           0 = La solicitud es inválida (atributos faltantes, tipo o extensión no válidos). 
*           1 = La solicitud es válida y corresponde a un tipo "Media". 
*           2 = La solicitud es válida y corresponde a un tipo "Text". 
* 
************************************************/
int HANDLE_validateAttributesDistortRequest (char *data_buffer, char *mediaType, char **fileName) {
    mediaType = strtok(data_buffer, "&");
    *fileName = strtok(NULL, "&");

    if (!(*mediaType) || !(*fileName)) {
        return 0;  
    }

    if (strcmp(mediaType, "Media") != 0 && strcmp(mediaType, "Text") != 0) {
        return 0;  
    }

    //Mirar si te una extensió
    const char *dot = strrchr(*fileName, '.');
    if (!dot || dot == *fileName) {
        return 0;
    }

    // Parsejar l'extensió
    char *extension = FILE_getFileExtension(*fileName);

    if (!extension) {
        return 0;  
    }

    //Bucle per mirar si dintr dels tres arrays globals es troba l'extensió
    if (strcmp(mediaType, "Media") == 0) {
        // Verificar a audioExtensions
        for (int i = 0; i < audioExtensionsSize; i++) {
            if (strcmp(extension, audioExtensions[i]) == 0) {
                return 1; 
            }
        }
        
        // Verificar a imageExtensions
        for (int i = 0; i < imageExtensionsSize; i++) {
            if (strcmp(extension, imageExtensions[i]) == 0) {
                return 1;  
            }
        }
        return 0;  
    }

    // Si mediaType es "Text", verifica que l'extensió sigui .txt
    if (strcmp(mediaType, "Text") == 0 && strcmp(extension, "txt") == 0) {
        return 2; 
    }

    return 0;
}

/***********************************************
*
* @Finalidad: Obtener el nombre de usuario asociado a un socket de cliente en el servidor Gotham.
*
* @Parámetros:
* in/out: server = Puntero a la estructura `GothamServer` que contiene los detalles de los clientes conectados.
* in: client_socket = Descriptor del socket del cliente del que se desea obtener el nombre de usuario.
*
* @Retorno:
*           NULL = No se encontró un fleck asociado al socket especificado.
*           char* = Puntero al nombre de usuario asociado al socket.
*
************************************************/
char *HANDLE_getUsernameFromSocket(GothamServer *server, int client_socket) {
    FLECK_LINKEDLIST_goToHead(&server->fleck_list);
    while (!FLECK_LINKEDLIST_isAtEnd(server->fleck_list)) {
        Fleck fleck = FLECK_LINKEDLIST_get(&server->fleck_list);
        if (fleck.socket_fd == client_socket) {
            return fleck.username;
        }
        FLECK_LINKEDLIST_next(&server->fleck_list);
    }
    return NULL;
}	
/*********************************************** 
* 
* @Finalidad: Manejar una solicitud de distorsión recibida desde un fleck, validar los 
*             atributos de la solicitud y proporcionar los detalles del worker principal 
*             correspondiente o responder con un error si la solicitud es inválida. 
* 
* @Parámetros: 
* in/out: server = Puntero a la estructura `GothamServer` que contiene los detalles de los workers conectados. 
* in: client_socket = Descriptor del socket del cliente que realizó la solicitud. 
* in: result = Estructura `FrameResult` que contiene los datos de la trama de la solicitud. 
* in: type = Tipo de trama que indica si es una nueva solicitud de distorsión o una reanudación (e.g., `0x10` o `0x11`). 
* 
* @Retorno: Ninguno. 
* 
************************************************/
void HANDLE_handleDistortionRequest(GothamServer *server, int client_socket, FrameResult result, int type) {
    IO_printFormat(STDOUT_FILENO, YELLOW "\n%s\n", type == 0x11 ? "Processing request to resume distortion" : "Distortion request received");
    char *data_buffer = strdup((char *)result.frame->data);

    if (!data_buffer) {
        IO_printStatic(STDOUT_FILENO, RED "Error: Could not process warp request\n" RESET);
        COMM_sendDistortResponse(client_socket, "MEDIA_KO", 0, type);  //KO
        writeLog(result.frame, server->fd_log, "Error: Could not process warp request");
        return;
    }

    //Validar atributs de la trama
    char *mediaType = NULL;
    char *fileName = NULL;
    int valid = 0; 

    char *data = NULL;
    char *username = strdup(HANDLE_getUsernameFromSocket(server, client_socket));
    
    valid = HANDLE_validateAttributesDistortRequest(data_buffer, mediaType, &fileName);
  

    switch (valid) {
        case 0:
            IO_printStatic(STDOUT_FILENO, RED "Error: Invalid warp request\n" RESET);
            COMM_sendDistortResponse(client_socket, "MEDIA_KO", 0, type); //KO
            writeLog(result.frame, server->fd_log, "Error: Invalid warp request");
            break;
        case 1:
            HANDLE_provideFleckMainWorkerDetails(server, client_socket, "Media", type);
            if (asprintf(&data, "Fleck requested distortion: username=%s, mediaType=MEDIA, fileName=%s", username, fileName) == -1) {
                IO_printStatic(STDOUT_FILENO, "Error: Creating log\n");
                free(data);
                return;
            }
            writeLog(result.frame, server->fd_log, data);
            free(data);
            break;

        case 2:
            HANDLE_provideFleckMainWorkerDetails (server, client_socket, "Text", type);
            if (asprintf(&data, "Fleck requested distortion: username=%s, mediaType=TEXT, fileName=%s", username, fileName) == -1) {
                IO_printStatic(STDOUT_FILENO, "Error: Creating log\n");
                free(data);
                return;
            }
            writeLog(result.frame, server->fd_log, data);
            free(data);
            break;
    }

    free(data_buffer);
    free(username);
}

/*********************************************** 
* 
* @Finalidad: Validar los atributos de una solicitud de conexión, verificando la presencia 
*             y validez de la cadena, dirección IP y puerto proporcionados en la solicitud. 
* 
* @Parámetros: 
* in/out: data_buffer = Cadena que contiene los datos de la solicitud de conexión. 
* out: string = Puntero que recibirá la cadena extraída del campo de datos. 
* out: ip_address = Puntero que recibirá la dirección IP extraída. 
* out: port_str = Puntero que recibirá el puerto extraído como cadena. 
* 
* @Retorno: 
*           0 = Los atributos son inválidos (faltantes o valores no válidos). 
*           1 = Los atributos son válidos. 
* 
************************************************/
int HANDLE_validateAttributesConnection(char *data_buffer, char **string, char **ip_address, char **port_str) {
    //extreiem els atributs del camp de dades
    *string = strtok(data_buffer, "&");
    *ip_address = strtok(NULL, "&");
    *port_str = strtok(NULL, "&");

    //validem que tots els atributs existeixin
    if (!(*string) || !(*ip_address) || !(*port_str)) {
        return 0;  
    }

    //comprovem que l'adreça IP sigui vàlida
    if (!STRING_isValidIP(*ip_address)) {
        return 0;  
    }

    //comprovem que el port és vàlid 
    int port = atoi(*port_str);
    if (port <= 0 || port > 65535) {
        return 0;  
    }

    return 1;  //els atributs són correctes
}

/*********************************************** 
* 
* @Finalidad: Manejar una solicitud de conexión recibida desde un fleck o un worker, 
*             validando los atributos de la solicitud y agregando al cliente al servidor 
*             si los atributos son válidos. 
* 
* @Parámetros: 
* in/out: server = Puntero a la estructura `GothamServer` que administra los clientes y workers conectados. 
* in: client_socket = Descriptor del socket del cliente que envió la solicitud. 
* in: result = Estructura `FrameResult` que contiene los datos de la solicitud de conexión. 
* in: client_type = Tipo de cliente (`'f'` para fleck o `'w'` para worker). 
* 
* @Retorno: Ninguno. 
* 
************************************************/
void HANDLE_handleConnectionRequest(GothamServer *server, int client_socket, FrameResult result, char client_type) {
    // Creem una còpia dinàmica de data_buffer per evitar modificar l'original
    char *data_buffer = strdup((char *)result.frame->data);
    if (!data_buffer) {
        if (client_type == 'f') {
            COMM_sendConnectionResponse(client_socket, "CON_KO", 0, 0x01);  // Error de memòria
            writeLog(result.frame, server->fd_log, "Error: Could not process connection request");
        } else {
            COMM_sendConnectionResponse(client_socket, "CON_KO", 0, 0x02);  // Error de memòria
            writeLog(result.frame, server->fd_log, "Error: Could not process connection request");
        }
        return;
    }

    // Inicialitzem els punters als atributs per passar-los a la funció de validació
    char *username = NULL;
    char *worker_type = NULL;
    char *ip_address = NULL;
    char *port_str = NULL;
    int valid = 0;

    // Validem si els atributs del camp de dades són correctes
    valid = HANDLE_validateAttributesConnection(data_buffer, client_type == 'f' ? &username : &worker_type, &ip_address, &port_str);
    char *data = NULL;

    if (valid) {
        if (client_type == 'f') {
            COMM_sendConnectionResponse(client_socket, NULL, 1, 0x01);
            MC_addFleckToServer(server, client_socket, username, ip_address, port_str);
            if (asprintf(&data, "Fleck connected: username=%s", username) == -1) {
                IO_printStatic(STDOUT_FILENO, "Error: Creating log\n");
                free(data);
                return;
            }
            writeLog(result.frame, server->fd_log, data);
            free(data);
        }
        else {
            COMM_sendConnectionResponse(client_socket, NULL, 1, 0x02);
            int is_main_worker = MC_addWorkerToServer(server, client_socket, worker_type, ip_address, port_str);
            if (asprintf(&data, "%s connected: IP:%s:%s", !strcmp(worker_type, "Text") ? "Enigma" : "Harley", ip_address, port_str) == -1) {
                    IO_printStatic(STDOUT_FILENO, "Error: Creating log\n");
                    free(data);
                    return;
            }
            writeLog(result.frame, server->fd_log, data);
            free(data);

            if (is_main_worker) {
                COMM_sendNewMainWorkerResponse(client_socket);
                if (asprintf(&data, "%s assigned as primary worker: IP:%s:%s", !strcmp(worker_type, "Text") ? "Enigma" : "Harley", ip_address, port_str) == -1) {
                    IO_printStatic(STDOUT_FILENO, "Error: Creating log\n");
                    free(data);
                    return;
                }
                writeLog(result.frame, server->fd_log, data);
                free(data);
            }
        }        
    } else {
        IO_printStatic(STDOUT_FILENO, RED "Connection request failed: invalid attributes.\n" RESET);
        COMM_sendConnectionResponse(client_socket, "CON_KO", 0, client_type == 'f' ? 0x01 : 0x02);
        writeLog(result.frame, server->fd_log, "Error: Invalid connection request");

    }

    free(data_buffer); 
}


/*********************************************** 
* 
* @Finalidad: Manejar una trama recibida desde un cliente (fleck o worker), procesando 
*             la acción correspondiente en función del tipo de trama. 
* 
* @Parámetros: 
* in/out: server = Puntero a la estructura `GothamServer` que contiene la lista de clientes y workers. 
* in: client_socket = Descriptor del socket del cliente que envió la trama. 
* in: client_type = Tipo de cliente (`'f'` para fleck o `'w'` para worker). 
* in: log_fd = Descriptor del archivo de log donde se registrarán eventos importantes. 
* 
* @Retorno: Ninguno. 
* 
************************************************/
void HANDLE_handleFrame(GothamServer *server, int client_socket, char client_type) {
    FrameResult result = FRAME_receiveFrame(client_socket);

    if (result.error_code != FRAME_SUCCESS) {
        if(result.error_code == FRAME_DISCONNECTED) {
            MC_removeClient(server, client_socket, client_type);  
        }
        else if(result.error_code == FRAME_RECV_ERROR) {
            writeLog(result.frame, server->fd_log, "Error: Could not receive frame");
            IO_printStatic(STDOUT_FILENO, "Error: Invalid frame received. Sending error response...\n");
            COMM_sendErrorFrame(client_socket);
        }
        return;
    }

    Frame *frame = result.frame;
    switch (frame->type) {
        case 0x01: // Fleck -> Gotham: sol·licitud de connexió
            HANDLE_handleConnectionRequest(server, client_socket, result, client_type); 
            break;

        case 0x02: // Worker -> Gotham: sol·licitud de connexió
            HANDLE_handleConnectionRequest(server, client_socket, result, client_type); 
            break;

        case 0x10: // Fleck -> Gotham: sol·licitud de distorsió
            HANDLE_handleDistortionRequest(server, client_socket, result, 0x10);
            break;

        case 0x11: // Fleck -> Gotham: resumir distorsió
            HANDLE_handleDistortionRequest(server, client_socket, result, 0x11);
            break;

        case 0x07: // Fleck -> Gotham: desconnexió de fleck // Worker -> Gotham: desconnexió de worker
            MC_removeClient(server, client_socket, client_type);  
            break;

        case 0x09: // Fleck/worker -> Gotham: resposta d'un fleck o un worker a una trama rebuda erréonea 
            IO_printStatic(STDOUT_FILENO, "Error: Please send the frame again.\n");
            break; 

        default:
            IO_printStatic(STDOUT_FILENO, "Error: Invalid frame received. Sending error response...\n");
            COMM_sendErrorFrame(client_socket);
            break;
    }

    FRAME_destroyFrame(frame);
}

/*********************************************** 
* 
* @Finalidad: Aceptar una nueva conexión entrante desde un cliente (fleck o worker), 
*             agregar su socket al servidor y devolver el descriptor del socket aceptado. 
* 
* @Parámetros: 
* in: listen_socket = Descriptor del socket en modo escucha que recibe la conexión entrante. 
* in/out: server = Puntero a la estructura `GothamServer` que administra los clientes conectados. 
* in: is_fleck = Indicador booleano que especifica si el cliente es un fleck (`true`) o un worker (`false`). 
* 
* @Retorno: 
*           > 0 = Descriptor del socket del cliente aceptado. 
*           -1 = Error al aceptar la conexión. 
* 
************************************************/
int HANDLE_acceptNewConnection(int listen_socket, GothamServer *server, bool is_fleck) {
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    
    int client_socket = accept(listen_socket, (struct sockaddr *)&client_addr, &addr_len);
    if (client_socket < 0) {
        IO_printStatic(STDOUT_FILENO, "Error: Error accepting connection.\n");
        return -1;
    }

    //afegim el socket a l'estructura de sockets de clients del server
    MC_addClient(server, client_socket, is_fleck ? 'f' : 'w');

    return client_socket;
}