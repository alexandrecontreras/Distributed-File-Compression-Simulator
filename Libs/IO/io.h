/*********************************************** 
* 
* @Autores: Alexandre Contreras y Armand López
* @Propósito: Proveer funciones para la gestión de entrada y salida (I/O) 
*             en sistemas que interactúan con descriptores de archivo. 
*             Incluye macros para impresión formateada y estática, así como 
*             funciones para lectura dinámica y no bloqueante. 
* @Fecha de creación: 12 de octubre de 2024
* @Última modificación: 4 de enero de 2025
* 
************************************************/

#ifndef _IO_CUSTOM_H_
#define _IO_CUSTOM_H_

// Constants del sistema
#define _GNU_SOURCE     // Permet utilitzar asprintf

//Libreries del sistema
#include <stdlib.h>     // Funcions de gestió de memòria dinàmica
#include <unistd.h>     // Accés a crides del sistema relacionades amb fitxers (read, write)
#include <fcntl.h>      // Configuració de descriptores de fitxer com a no bloquejants
#include <errno.h>      // Permet manejar i comprovar errors de les funcions del sistema
#include <sys/select.h> // Per la funció select, que monitoritza múltiples descriptores de fitxer
#include <sys/time.h>   // Per l'estructura timeval, necessària per establir temporitzadors en select
#include <string.h>     // Per treballar amb cadenes (strlen)

// Defines del colors per a la impressió
#define RED "\x1B[31m"
#define GREEN "\x1B[32m"
#define YELLOW "\x1B[33m"
#define BLUE "\x1B[34m"
#define MAGENTA "\x1B[35m"
#define CYAN "\x1B[36m"
#define PURPLE "\x1b[38;5;99m"
#define GRAY "\x1b[38;5;250m"
#define RESET "\x1B[0m"
#define PINK "\x1b[38;2;255;215;255m"
#define LAVENDER "\x1b[38;2;215;175;255m"

// Variables globals per a les extensions de fitxers
extern const char *audioExtensions[];
extern const char *imageExtensions[];
extern const int audioExtensionsSize;
extern const int imageExtensionsSize;

// Macros
/*********************************************** 
* 
* @Finalidad: Formatear una cadena y escribirla en un descriptor de archivo. 
* 
* @Parámetros: 
* in: fd = Descriptor de archivo donde se escribirá la cadena formateada. 
* in: format = Cadena de formato. 
* in: ... = Lista de argumentos variables para el formato. 
* 
* @Retorno: Ninguno. 
* 
************************************************/
#define IO_printFormat(fd, format, ...) ({                                       \
        char *buffer;                                                         \
        int len = asprintf(&buffer, format, __VA_ARGS__);                     \
        if (len == -1) {                                                      \
            perror("asprintf failed");                                        \
            exit(-1);                                                         \
        }                                                                     \
        write(fd, buffer, len);                                               \
        free(buffer);                                                         \
})

/*********************************************** 
* 
* @Finalidad: Escribir una cadena estática en un descriptor de archivo. 
* 
* @Parámetros: 
* in: fd = Descriptor de archivo donde se escribirá la cadena. 
* in: x = Cadena estática que se escribirá. 
* 
* @Retorno: Ninguno. 
* 
************************************************/
#define IO_printStatic(fd, x) ({                                                 \
        write(fd, x, strlen(x));                                              \
})

//Funcions

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
char *IO_readUntil(int fd, char cEnd);

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
char *IO_nonBlockingReadUntil(int fd, char cEnd, volatile int* exit_flag, int* flag2, int* flag3);

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
void freePointer(void **ptr);

#endif // _IO_CUSTOM_H_