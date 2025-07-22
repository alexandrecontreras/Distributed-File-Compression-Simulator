/***********************************************
*
* @Autores: Alexandre Contreras, Armand López.
* 
* @Finalidad: Implementar funciones para la gestión de sockets en aplicaciones cliente-servidor, 
*             incluyendo la creación de sockets de escucha y cliente, y operaciones seguras 
*             como aceptar conexiones con `select`.
* 
* @Fecha de creación: 11 de noviembre de 2024.
* 
* @Última modificación: 4 de enero de 2025.
* 
************************************************/

#include "socket.h"

/*********************************************** 
* 
* @Finalidad: Inicializar un socket en modo escucha, enlazándolo a una dirección IP y un puerto específicos, 
*             y configurarlo para aceptar conexiones entrantes. 
* 
* @Parámetros: 
* in: ip = Dirección IP donde el socket escuchará las conexiones. 
* in: port = Puerto donde el socket escuchará las conexiones. 
* in: max_clients = Número máximo de clientes en cola de espera. 
* 
* @Retorno: 
*           Descriptor del socket en modo escucha si la operación es exitosa. 
*          -1 = Error al crear, enlazar o configurar el socket en modo escucha. 
* 
************************************************/
int SOCKET_initListenSocket(const char* ip, int port, int max_clients) {
    int listen_socket;
    struct sockaddr_in addr;

    listen_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_socket < 0) {
        IO_printStatic(STDOUT_FILENO, "Error: Error creating socket\n");
        return -1; 
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip);

    if (bind(listen_socket, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        IO_printStatic(STDOUT_FILENO, "Error: Error binding socket\n");
        close(listen_socket);
        return -1; 
    }

    if (listen(listen_socket, max_clients) < 0) {
        IO_printStatic(STDOUT_FILENO, "Error: Error listening on socket\n");
        close(listen_socket);
        return -1; 
    }

    return listen_socket;
}

/*********************************************** 
* 
* @Finalidad: Crear e inicializar un socket cliente para conectarse a un servidor 
*             en una dirección IP y puerto específicos. 
* 
* @Parámetros: 
* in: ip = Dirección IP del servidor al que se conectará el cliente. 
* in: port = Puerto del servidor al que se conectará el cliente. 
* 
* @Retorno: 
*           Descriptor del socket del cliente si la conexión es exitosa. 
*          -1 = Error al crear, configurar o establecer la conexión con el servidor. 
* 
************************************************/
int SOCKET_initClientSocket(const char* ip, int port) {
    int client_socket;
    struct sockaddr_in server_addr;

    //crea el socket
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0) {
        IO_printStatic(STDOUT_FILENO, RED "Error creating client socket. Exiting...\n" RESET);
        return -1;
    }

    //configura l'adreça del servidor
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    //converteix l'adreça IP a format binari
    if (inet_pton(AF_INET, ip, &server_addr.sin_addr) <= 0) {
        //IO_printStatic(STDOUT_FILENO, RED "Invalid IP address. Exiting...\n" RESET);
        close(client_socket);
        return -1;
    }

    //estableix la connexió
    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        close(client_socket);
        return -1;
    }

    return client_socket;
}

/*********************************************** 
* 
* @Finalidad: Cerrar un socket abierto y establecer su descriptor a `-1` para indicar 
*             que ya no está activo. 
* 
* @Parámetros: 
* in/out: socket = Puntero al descriptor del socket que se cerrará. 
* 
* @Retorno: Ninguno. 
* 
************************************************/
void SOCKET_closeSocket(int* socket) { 
    if(*socket != -1) {
        close(*socket);
        *socket = -1; 
    }
}

/*********************************************** 
* 
* @Finalidad: Aceptar una conexión de cliente de forma segura en un socket de escucha, 
*             utilizando la función `select` para manejar posibles bloqueos y timeouts. 
* 
* @Parámetros: 
* in: listen_socket = Descriptor del socket en modo escucha donde se aceptará la conexión. 
* 
* @Retorno: 
*           > 0 = Descriptor del socket del cliente aceptado. 
*          -1 = Error al aceptar la conexión, ya sea por timeout o fallo en `select`. 
* 
************************************************/
int SOCKET_safe_accept(int listen_socket) {
    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(listen_socket, &read_fds);

    struct timeval timeout = {1, 0};  // 1 second timeout
    int select_result = select(listen_socket + 1, &read_fds, NULL, NULL, &timeout);

    if (select_result < 0) {
        return -1;
    }

    if (select_result == 0) {
        return -1;
    }

    if (FD_ISSET(listen_socket, &read_fds)) {
        // The socket is ready to accept a connection
        int client_socket = accept(listen_socket, NULL, NULL);

        return client_socket;  // Return the accepted socket, even in case of errors.
    }

    return -1;  // This should never be reached if the socket is ready to accept
}