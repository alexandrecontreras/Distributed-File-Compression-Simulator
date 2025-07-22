/*********************************************** 
* 
* @Autores: Alexandre Contreras, Armand López. 
* 
* @Finalidad: Proveer funciones para operaciones de entrada/salida, incluyendo lectura 
*             hasta delimitadores específicos, lectura no bloqueante con manejo de 
*             interrupciones y liberación segura de memoria dinámica. 
* 
* @Fecha de creación: 10 de noviembre de 2024. 
* 
* @Última modificación: 4 de enero de 2025.
* 
************************************************/

#include "io.h"

// Variables globals per a les extensions de fitxers
const char *audioExtensions[] = {"wav"};
const char *imageExtensions[] = {"png", "jpg", "jpeg", "bmp", "tga"};
const int audioExtensionsSize = sizeof(audioExtensions) / sizeof(audioExtensions[0]);
const int imageExtensionsSize = sizeof(imageExtensions) / sizeof(imageExtensions[0]);

/*********************************************** 
* 
* @Finalidad: Leer caracteres de un descriptor de archivo hasta encontrar un delimitador 
*             especificado o el final del archivo (EOF). Los caracteres leídos se almacenan 
*             en un buffer dinámico. 
* 
* @Parámetros: 
* in: fd = Descriptor de archivo desde el cual se leerán los caracteres. 
* in: cEnd = Carácter delimitador que indica el fin de la lectura. 
* 
* @Retorno: 
*           Puntero a una cadena dinámica que contiene los caracteres leídos hasta el delimitador. 
*           Retorna NULL si ocurre un error, se alcanza el EOF sin leer nada, o falla la asignación de memoria. 
* 
************************************************/
char *IO_readUntil(int fd, char cEnd) {
    int i = 0;
    ssize_t chars_read;
    char c = 0;
    char *buffer = NULL;

    while (1) {
        chars_read = read(fd, &c, sizeof(char));  
        
        if (chars_read == 0) {  // EOF         
            if (i == 0) { // Si no se ha leído nada, retorna NULL
                return NULL;
            }  
            break;
        } else if (chars_read < 0) { // Error de lectura
            IO_printStatic(STDOUT_FILENO, "Error: Read error\n");
            free(buffer);
            return NULL;
        }

        if (c == cEnd) {  // Encontrado el delimitador
            break;
        }

        buffer = (char *)realloc(buffer, i + 2); // Asignar más memoria
        if (!buffer) {
            IO_printStatic(STDOUT_FILENO, "Error: Memory allocation failed\n");
            free(buffer);  // Alliberem memoria alocada fins al moment
            return NULL;
        }
        buffer[i++] = c;  // Guardar el carácter leído
    }

    buffer[i] = '\0';  // Null-terminate the string
    return buffer;
}

/*********************************************** 
* 
* @Finalidad: Leer caracteres de un descriptor de archivo de forma no bloqueante hasta 
*             encontrar un delimitador específico, el final del archivo (EOF), o una señal de interrupción. 
* 
* @Parámetros: 
* in: fd = Descriptor de archivo desde el cual se leerán los caracteres. 
* in: cEnd = Carácter delimitador que indica el fin de la lectura. 
* in: exit_flag = Puntero a una bandera `volatile int` que indica si se debe interrumpir la operación de lectura. 
* 
* @Retorno: 
*           Puntero a una cadena dinámica que contiene los caracteres leídos hasta el delimitador. 
*           Retorna NULL si ocurre un error, se alcanza el EOF sin leer nada, o si se detecta la señal de interrupción. 
* 
************************************************/
char *IO_nonBlockingReadUntil(int fd, char cEnd, volatile int* exit_flag, int* flag2, int* flag3) {
    int i = 0;
    ssize_t chars_read;
    char c = 0;
    char *buffer = NULL;

    // Declarem el fd no bloquejant
    fcntl(fd, F_SETFL, O_NONBLOCK);

    //Bucle per anar mirant l'estat del fd i detectar si el usuari ens ha introduit alguna comanda
    while (1) {
        // Comprovem si s'ha de sortir del bucle per el cas de Ctrl+C, GothamCrash o Logout
        if ((*exit_flag) || (*flag2) || (*flag3)) {
            free(buffer);
            return NULL;
        }

        // Inicialitzem el conjunt de fd (en el nostre cas només un fd, el de STDIN)
        fd_set read_fds;
        struct timeval timeout = {0, 100000};  

        // Inicialitzem el conjunt de fd
        FD_ZERO(&read_fds);
        FD_SET(fd, &read_fds);

        // Compromovem si hi ha alguna activitat en el fd en un temps determinat
        int ret;
        while ((ret = select(fd + 1, &read_fds, NULL, NULL, &timeout)) < 0) {
            // Comprovem que no sigui un error d'interrupció
            if (errno != EINTR) {  
                IO_printStatic(STDOUT_FILENO, "Error: Select error\n");
                free(buffer);
                return NULL;
            }
        }

        // Si ha passat el temps sense activitat, tornem a anar al principi del bucle
        if (ret == 0) { 
            continue;  
        }

        // Si no hi ha error, ni timeout, llegim el caràcter
        chars_read = read(fd, &c, sizeof(char));

        if (chars_read == 0) {  // EOF
            if (i == 0) {  
                return NULL;
            }
            break;  
        } else if (chars_read < 0) { 
            // Comprovem si és un error de lectura no bloquejant 
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                continue;
            }
            IO_printStatic(STDOUT_FILENO, "Error: Read error\n");
            free(buffer);
            return NULL;
        }

        // Si hem llegit el caràcter de finalització, sortim del bucle
        if (c == cEnd) {  
            break;
        }

        // Anem guardant els caràcters llegits en el buffer
        char *new_buffer = (char *)realloc(buffer, i + 2); 
        if (!new_buffer) {
            IO_printStatic(STDOUT_FILENO, "Error: Memory allocation failed\n");
            free(buffer);  // Alliberem memoria alocada fins al moment
            return NULL;
        }
        buffer = new_buffer;
        buffer[i++] = c;  
    }

    if(!buffer) {
        buffer = (char*) malloc (sizeof(char));
        if(!buffer) return NULL; 
    }
    
    // Tanquem el buffer amb el caracter de finalització
    buffer[i] = '\0';  
    return buffer;
}

/*********************************************** 
* 
* @Finalidad: Liberar la memoria dinámica apuntada por un puntero y establecerlo a `NULL` 
*             para evitar accesos posteriores a memoria no válida. 
* 
* @Parámetros: 
* in/out: ptr = Doble puntero a la memoria que se desea liberar. Si el puntero o el contenido 
*               apuntado es `NULL`, la función no realiza ninguna acción. 
* 
* @Retorno: Ninguno. 
* 
************************************************/
void freePointer(void **ptr) {
    if (ptr != NULL && *ptr != NULL) {
        free(*ptr);  
        *ptr = NULL; 
    }
}