/*********************************************** 
* 
* @Autores: Alexandre Contreras, Armand López. 
* 
* @Finalidad: Proveer funciones para la gestión de directorios y archivos, incluyendo 
*             verificación de existencia, listado de archivos por tipo, y operaciones 
*             de movimiento entre directorios privados y compartidos. 
* 
* @Fecha de creación: 28 de octubre de 2024. 
* 
* @Última modificación: 5 de enero de 2025. 
* 
************************************************/

#include "dir.h"

/*********************************************** 
* 
* @Finalidad: Verificar si un archivo específico existe en un directorio dado. 
* 
* @Parámetros: 
* in: folder_path = Ruta del directorio donde se buscará el archivo. 
* in: filename = Nombre del archivo que se desea verificar. 
* in: print_mutex = Mutex para sincronizar los mensajes de impresión en caso de error. 
* 
* @Retorno: 
*           1 = El archivo existe en el directorio. 
*           0 = El archivo no existe o ocurrió un error al acceder al directorio. 
* 
************************************************/
int DIR_fileExistsInFolder(const char *folder_path, const char *filename, pthread_mutex_t *print_mutex) {
    DIR *d = NULL;
    struct dirent *dir;
    char *full_path = NULL;

    // Construir el path al directorio
    if (asprintf(&full_path, ".%s", folder_path) == -1) {
        STRING_printF(print_mutex, STDOUT_FILENO, RED, "Error: Could not allocate memory for full_path.\n");
        return 0;
    }

    d = opendir(full_path);
    if (!d) {
        STRING_printF(print_mutex, STDOUT_FILENO, RED, "Error: Could not open directory.\n");
        free(full_path);
        return 0;
    }

    // Recorrer todos los archivos del directorio
    while ((dir = readdir(d)) != NULL) {
        // Comparar el nombre del archivo
        if (strcmp(dir->d_name, filename) == 0) {
            closedir(d);
            free(full_path);
            return 1;  // Archivo encontrado
        }
    }

    // Archivo no encontrado
    closedir(d);
    free(full_path);
    return 0;
}

/*********************************************** 
* 
* @Finalidad: Mostrar los archivos disponibles en un directorio según el tipo especificado 
*             (e.g., "Text" para archivos `.txt` o "Media" para otros tipos de archivos). 
* 
* @Parámetros: 
* in: type = Tipo de archivo a listar ("Text" para archivos `.txt` o "Media" para otros). 
* in: folder_path = Ruta del directorio donde se buscarán los archivos. 
* in: print_mutex = Mutex para sincronizar los mensajes de impresión. 
* 
* @Retorno: Ninguno. 
* 
************************************************/
void DIR_printTextDirectory(char *type, char *folder_path, pthread_mutex_t *print_mutex) {
    DIR *d = NULL;
    struct dirent *dir;
    int num_files = 0;
    char *full_path = NULL;

    full_path = malloc(strlen(folder_path) + 2);
    if (!full_path) {
        STRING_printF(print_mutex, STDOUT_FILENO, RESET, "Error: Could not allocate memory for full_path.\n");
        return;
    }
    // Construir el path al directorio
    sprintf(full_path, ".%s", folder_path);

    // Abrir el directorio
    d = opendir(full_path);
    if (!d) {
        STRING_printF(print_mutex, STDOUT_FILENO, RED, "Error: Could not open directory %s\n", full_path);
        free(full_path);
        return;
    }

    // Contar archivos
    while ((dir = readdir(d)) != NULL) {
        if (strcmp(dir->d_name, ".") != 0 && strcmp(dir->d_name, "..") != 0) {
            if (strstr(dir->d_name, "_distorted") == NULL) { // Filtrar archivos con "_distorted" al final
                char *extension = FILE_getFileExtension(dir->d_name);
                if ((extension && strcmp(extension, "txt") == 0 && strcmp(type, "Text") == 0) ||
                    (extension && strcmp(extension, "txt") != 0 && strcmp(type, "Media") == 0)) {
                    num_files++;
                }
            }
        }
    }

    rewinddir(d);

    // Mostrar resultados
    if (num_files > 0) {
        STRING_printF(print_mutex, STDOUT_FILENO, RESET, "There are %d %s files available:\n", num_files, type);
        num_files = 0;
        while ((dir = readdir(d)) != NULL) {
            if (strcmp(dir->d_name, ".") != 0 && strcmp(dir->d_name, "..") != 0) {
                if (strstr(dir->d_name, "_distorted") == NULL) { // Filtrar archivos con "_distorted" al final
                    char *extension = FILE_getFileExtension(dir->d_name);
                    if ((extension && strcmp(extension, "txt") == 0 && strcmp(type, "Text") == 0) ||
                        (extension && strcmp(extension, "txt") != 0 && strcmp(type, "Media") == 0)) {
                        num_files++;
                        STRING_printF(print_mutex, STDOUT_FILENO, RESET, "%d. %s\n", num_files, dir->d_name);
                    }
                }
            }
        }
    } else {
        STRING_printF(print_mutex, STDOUT_FILENO, RED, "No files found\n");
    }

    // Limpiar
    closedir(d);
    free(full_path);
}

/***********************************************
*
* @Finalidad: Verificar si un archivo distorsionado específico existe en un directorio dado.
*
* @Parámetros:
* in: folder_path = Ruta del directorio donde se buscará el archivo distorsionado.
* in: file_name = Nombre del archivo original que se desea verificar.
*
* @Retorno:
*           1 = El archivo distorsionado no existe en el directorio.
*           0 = El archivo distorsionado existe en el directorio.
*           -1 = Error al intentar acceder al directorio.
*
************************************************/
int DIR_checkDistortedFile(char *folder_path, char *file_name, pthread_mutex_t *print_mutex) {
    DIR *d = NULL;
    struct dirent *dir;
    char *full_path = NULL;

    full_path = malloc(strlen(folder_path) + 2); // Para "." + terminador
    if (!full_path) {
        STRING_printF(print_mutex, STDOUT_FILENO, RED, "Error: Could not allocate memory for full_path.\n");
        return -1; // Error de memoria
    }
    sprintf(full_path, ".%s", folder_path);

    // Abrir el directorio
    d = opendir(full_path);
    if (!d) {
        STRING_printF(print_mutex, STDOUT_FILENO, RED, "Error: Could not open directory %s\n", full_path);
        free(full_path);
        return -1; // Error al abrir el directorio
    }

    // Construir el nombre del archivo distorsionado
    char *distorted_file = malloc(strlen(file_name) + 11); // "_distorted" + terminador
    if (!distorted_file) {
        STRING_printF(print_mutex, STDOUT_FILENO ,RED, "Error: Could not allocate memory for distorted_file.\n");
        closedir(d);
        free(full_path);
        return -1; // Error de memoria
    }
    sprintf(distorted_file, "%s_distorted", file_name);

    // Buscar el archivo distorsionado
    while ((dir = readdir(d)) != NULL) {
        if (strcmp(dir->d_name, distorted_file) == 0) {
            free(distorted_file);
            closedir(d);
            free(full_path);
            return 0; // Archivo distorsionado encontrado
        }
    }

    // Archivo distorsionado no encontrado
    free(distorted_file);
    closedir(d);
    free(full_path);
    return 1;
}

/*********************************************** 
* 
* @Finalidad: Verificar si un directorio especificado existe en el sistema de archivos. 
* 
* @Parámetros: 
* in: path = Ruta del directorio que se desea verificar. 
* 
* @Retorno: 
*           1 = El directorio existe. 
*           0 = El directorio no existe o ocurrió un error al intentar acceder a él. 
* 
************************************************/
int DIR_directoryExists(const char* path) {
    struct stat info;
    // Comprovem si path és vàlid i extraiem informació d'aquest
    if (stat(path, &info) != 0) {
        // stat() fallit, el path no existeix
        return 0; // fals
    }
    return 1; 
}

/*********************************************** 
* 
* @Finalidad: Mover un archivo desde una carpeta compartida a una carpeta privada especificada. 
* 
* @Parámetros: 
* in: filename = Nombre del archivo que se desea mover. 
* in: username = Nombre del usuario asociado al archivo. 
* in: private_path = Ruta completa de la carpeta privada de destino. 
* 
* @Retorno: 
*           1 = El archivo se movió correctamente. 
*           0 = Ocurrió un error al intentar mover el archivo. 
* 
************************************************/
int DIR_moveFileToPrivateFolder(char* filename, char* username, char* private_path) {
    char* global_path = FILE_buildSharedFilePath(filename, username);
    if(!global_path) return 0;

    IO_printFormat(STDOUT_FILENO, LAVENDER "Moving file from %s to %s\n" RESET, global_path, private_path);
    int mv_status = rename(global_path, private_path);
    free(global_path);

    if (mv_status != 0) {
        IO_printFormat(STDOUT_FILENO, RED "ERROR: failed to move file to private folder. Reason: %s\n" RESET, strerror(errno));
        return 0;  
    }

    return 1;
}

/*********************************************** 
* 
* @Finalidad: Mover un archivo desde una carpeta privada a una carpeta compartida especificada. 
* 
* @Parámetros: 
* in: filename = Nombre del archivo que se desea mover. 
* in: username = Nombre del usuario asociado al archivo. 
* in: private_path = Ruta completa de la carpeta privada de origen. 
* 
* @Retorno: 
*           1 = El archivo se movió correctamente. 
*           0 = Ocurrió un error al intentar mover el archivo. 
* 
************************************************/
int DIR_moveFileToSharedFolder(char* filename, char* username, char* private_path) {
    char* global_path = FILE_buildSharedFilePath(filename, username);

    if(!global_path) return 0;

    int mv_status = rename(private_path, global_path);
    IO_printFormat(STDOUT_FILENO, LAVENDER "Moving file from %s to %s\n" RESET, private_path, global_path);
    free(global_path);

    if (mv_status != 0) {
        IO_printFormat(STDOUT_FILENO, RED "ERROR: failed to move file to shared folder. Reason: %s\n" RESET, strerror(errno));
        return 0;  
    }

    return 1; 
}
