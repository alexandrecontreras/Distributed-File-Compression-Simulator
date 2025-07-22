/***********************************************
*
* @Autores: Alexandre Contreras, Armand López.
* 
* @Finalidad: Implementar funciones de manipulación de cadenas para su uso en aplicaciones 
*             de sistemas distribuidos, con énfasis en la limpieza, validación y sincronización de salida estándar.
* 
* @Fecha de creación: 30 de noviembre de 2024.
* 
* @Última modificación: 4 de enero de 2025.
* 
************************************************/

#include "string.h"

/*********************************************** 
* 
* @Finalidad: Convertir todos los caracteres de una cadena a minúsculas. 
* 
* @Parámetros: 
* in/out: str = Puntero a la cadena que se convertirá a minúsculas. 
* 
* @Retorno: Ninguno. 
* 
************************************************/
void STRING_toLowerCase(char* str) {
    int i = 0;

    for (i = 0; str[i]; i++) {
        str[i] = tolower(str[i]);
    }
}

/*********************************************** 
* 
* @Finalidad: Eliminar todos los espacios en blanco de una cadena y compactar los caracteres restantes. 
* 
* @Parámetros: 
* in/out: cmd = Puntero a la cadena de entrada que será modificada para eliminar los espacios. 
* 
* @Retorno: Ninguno. 
* 
************************************************/
void STRING_removeSpaces(char* cmd) {
    int i = 0, j = 0;

    while (isspace(cmd[i])) {
        i++;
    }

    while (cmd[i] != '\0') {
        if (!isspace(cmd[i])) {
            cmd[j++] = cmd[i];
        }
        i++;
    }

    cmd[j] = '\0';
}

/*********************************************** 
* 
* @Finalidad: Eliminar todos los caracteres '&' de una cadena. 
* 
* @Parámetros: 
* in/out: username = Puntero a la cadena que será procesada para eliminar los caracteres '&'. 
* 
* @Retorno: Ninguno. 
* 
************************************************/
void STRING_checkCharacterAmpersand(char *username) {
    if (username == NULL) return; // Validar si el puntero es NULL

    int i = 0, j = 0;
    while (username[i] != '\0') {
        if (username[i] != '&') {
            username[j++] = username[i]; // Copiar los caracteres que no sean '&'
        }
        i++;
    }
    username[j] = '\0'; // Asegurarse de terminar la cadena
}

/*********************************************** 
* 
* @Finalidad: Verificar si una cadena representa una dirección IP válida en formato IPv4. 
* 
* @Parámetros: 
* in: ip = Puntero a la cadena que contiene la dirección IP a verificar. 
* 
* @Retorno: 
*           1 = La cadena es una dirección IP válida. 
*           0 = La cadena no es una dirección IP válida. 
* 
************************************************/
int STRING_isValidIP(const char *ip) {
    struct sockaddr_in sa;
    return inet_pton(AF_INET, ip, &(sa.sin_addr)) != 0;
}

/*********************************************** 
* 
* @Finalidad: Inicializar un mutex utilizado para la sincronización de operaciones de impresión en pantalla. 
* 
* @Parámetros: 
* in/out: print_mutex = Mutex que será inicializado. 
* 
* @Retorno: Ninguno. 
* 
************************************************/
void STRING_initScreenMutex(pthread_mutex_t print_mutex) {
    pthread_mutex_init(&print_mutex, NULL);
}

/*********************************************** 
* 
* @Finalidad: Destruir un mutex utilizado para la sincronización de operaciones de impresión en pantalla. 
* 
* @Parámetros: 
* in/out: print_mutex = Mutex que será destruido. 
* 
* @Retorno: Ninguno. 
* 
************************************************/
void STRING_destroyScreenMutex(pthread_mutex_t print_mutex) {
    pthread_mutex_destroy(&print_mutex);
}

/*********************************************** 
* 
* @Finalidad: Imprimir una cadena formateada con un color especificado, utilizando 
*             un mutex para sincronizar el acceso a la salida estándar. 
* 
* @Parámetros: 
* in: print_mutex = Puntero al mutex utilizado para sincronizar la operación de impresión. 
* in: color = Cadena que especifica el color en formato ANSI. 
* in: format = Cadena de formato para la impresión (similar a `printf`). 
* in: ... = Lista de argumentos variables para el formato. 
* 
* @Retorno: Ninguno. 
* 
************************************************/
void STRING_printF(pthread_mutex_t *print_mutex, int fd, const char *color, const char *format, ...) {
    pthread_mutex_lock(print_mutex); // Bloquear el mutex

    // Manejo de argumentos variables
    va_list args;
    va_start(args, format);

    // Crear la cadena formateada
    char *formatted_string;
    if (vasprintf(&formatted_string, format, args) == -1) {
        va_end(args);
        pthread_mutex_unlock(print_mutex);
        return; // Salimos si no se pudo asignar memoria
    }

    va_end(args);

    // Agregar el color y el reset a la cadena
    char *final_string;
    if (asprintf(&final_string, "%s%s%s", color, formatted_string, "\033[0m") == -1) {
        free(formatted_string);
        pthread_mutex_unlock(print_mutex);
        return; // Salimos si no se pudo asignar memoria
    }

    free(formatted_string); // Liberar la cadena formateada intermedia

    // Escribir la cadena final en la salida estándar
    write(fd, final_string, strlen(final_string));

    free(final_string); // Liberar la cadena final

    pthread_mutex_unlock(print_mutex); // Desbloquear el mutex
}