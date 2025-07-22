/*********************************************** 
* 
* @Autores: Alexandre Contreras, Armand López. 
* 
* @Finalidad: Validar y procesar comandos del sistema de distorsión, verificando su 
*             formato, extrayendo parámetros como el nombre del archivo y el factor 
*             de distorsión, y transformándolos en equivalentes numéricos predefinidos. 
* 
* @Fecha de creación: 15 de octubre de 2024. 
* 
* @Última modificación: 4 de enero de 2025. 
* 
************************************************/

#include "cmd.h"

/*********************************************** 
* 
* @Finalidad: Validar si un comando cumple con el formato esperado para el sistema de distorsión. 
*             Extrae el nombre del archivo y el factor de distorsión si el comando es válido. 
* 
* @Parámetros: 
* in: cmd = Comando de entrada en forma de cadena que contiene la palabra "distort", 
*           el nombre del archivo y el factor de distorsión.
* in/out: filename_ptr = Puntero al nombre del archivo donde se almacenará el valor extraído. 
*                        Debe ser inicializado por el usuario o será nulo si no se necesita. 
* in/out: factor_ptr = Puntero entero donde se almacenará el factor extraído. 
*                      Debe ser inicializado por el usuario o será nulo si no se necesita.
* in: print_mutex = Mutex para gestionar la exclusión mutua al imprimir mensajes de error. 
* 
* @Retorno: Retorna 1 si el comando es válido y contiene todos los parámetros necesarios. 
*           Retorna 0 si el comando es inválido o hay errores en el formato del comando. 
* 
************************************************/
int CMD_isDistortCommandValid(const char* cmd, char** filename_ptr, int* factor_ptr, pthread_mutex_t *print_mutex) {
    const char *expected_command = "distort";
    size_t command_length = strlen(expected_command);
    size_t i = 0;

    // Saltar posibles espais al principi
    while (isspace(cmd[i])) i++;

    // Comparar la palabra "distort", paraula clau
    for (size_t j = 0; j < command_length; j++) {
        if (tolower(cmd[i + j]) != expected_command[j]) {
            return 0; // Comando inválido
        }
    }

    // Saltar posibles espais
    i += command_length;
    while (isspace(cmd[i])) i++;

    // Reservar memoria per el nom del fitxer
    char *filename = malloc(1);
    if (!filename) {
        STRING_printF(print_mutex, STDOUT_FILENO, RED, "Error: Could not allocate memory for filename.\n");
        return 0;
    }

    size_t filename_length = 0;

    // LLegir el nom del fitxer
    while (cmd[i] && !isspace(cmd[i])) {
        filename = realloc(filename, filename_length + 2); 
        if (!filename) { 
            STRING_printF(print_mutex, STDOUT_FILENO, RED, "Error: Could not allocate memory for filename.\n");
            return 0;
        }
        filename[filename_length++] = cmd[i++];
        filename[filename_length] = '\0'; // Terminar el string temporalmente
    }

    // Saltar posibles espais
    while (isspace(cmd[i])) i++;

    // Verificar si hi ha algun número després del nom del fitxer
    if (!isdigit(cmd[i])) {
        free(filename);
        return 0; 
    }

    // Convertir el número en el factor de distorsió
    int factor = atoi(&cmd[i]);

    // Verificar si hi han més caràcters després del número
    while (isdigit(cmd[i])) i++;
    if (cmd[i] != '\0') {
        free(filename);
        return 0; 
    }

    // Asignar valors als punters de sortida
    if (filename_ptr != NULL) {
        *filename_ptr = filename;
    } else {
        free(filename);
    }

    if (factor_ptr != NULL) {
        *factor_ptr = factor;
    }

    return 1;
}

/*********************************************** 
* 
* @Finalidad: Convertir un comando introducido como cadena a su equivalente numérico predefinido. 
*             Verifica la validez de comandos específicos y maneja errores de entrada. 
* 
* @Parámetros: 
* in: cmd = Comando de entrada en formato de cadena. Puede incluir espacios y debe ser 
*           uno de los comandos reconocidos por el sistema.
* in: print_mutex = Mutex utilizado para garantizar exclusión mutua al imprimir mensajes 
*                   de error relacionados con el comando.
* 
* @Retorno: Retorna un entero que corresponde al comando reconocido:
*           CMD_DISTORT = Si el comando es "distort" y es válido.
*           CMD_CONNECT = Si el comando es "connect".
*           CMD_LOGOUT = Si el comando es "logout".
*           CMD_LISTMEDIA = Si el comando es "listmedia".
*           CMD_LISTTEXT = Si el comando es "listtext".
*           CMD_CHECKSTATUS = Si el comando es "checkstatus".
*           CMD_CLEARALL = Si el comando es "clearall".
*           CMD_INVALID = Si el comando no es válido o no está reconocido.
* 
************************************************/
int CMD_changeComandToNumber(char* cmd, pthread_mutex_t *print_mutex) {

    if (strncmp(cmd, "distort", 7) == 0) {
        if (CMD_isDistortCommandValid(cmd, NULL, NULL, print_mutex)) {
            return CMD_DISTORT; 
        } else {
            return CMD_INVALID;
        }
    }

    STRING_removeSpaces(cmd);

    if (strcmp(cmd, "connect") == 0) {
        return CMD_CONNECT;
    } else if (strcmp(cmd, "logout") == 0) {
        return CMD_LOGOUT;
    } else if (strcmp(cmd, "listmedia") == 0) {
        return CMD_LISTMEDIA;
    } else if (strcmp(cmd, "listtext") == 0) {
        return CMD_LISTTEXT;
    } else if (strcmp(cmd, "checkstatus") == 0) {
        return CMD_CHECKSTATUS;
    } else if (strcmp(cmd, "clearall") == 0) {
        return CMD_CLEARALL;
    } else {
        return CMD_INVALID;
    }
}