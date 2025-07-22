/*********************************************** 
* 
* @Autores: Alexandre Contreras, Armand López. 
* 
* @Finalidad: Proveer funciones para la gestión de archivos, incluyendo construcción 
*             de rutas, manipulación de archivos (copiar, mover, reemplazar), 
*             y validación de integridad mediante hash MD5. 
* 
* @Fecha de creación: 4 de noviembre de 2024. 
* 
* @Última modificación: 6 de enero de 2025. 
* 
************************************************/

#include "file.h"

/*********************************************** 
* 
* @Finalidad: Construir una ruta completa a un archivo dentro de una carpeta privada, 
*             utilizando el nombre de usuario opcionalmente. 
* 
* @Parámetros: 
* in: distortions_folder_path = Ruta base de la carpeta de distorsiones privadas. 
* in: filename = Nombre del archivo. 
* in: username = Nombre del usuario asociado al archivo (puede ser NULL si no se utiliza). 
* 
* @Retorno: 
*           Puntero a una cadena que contiene la ruta completa del archivo en la carpeta privada. 
*           Retorna NULL si ocurre un error al construir la ruta. 
* 
************************************************/
char* FILE_buildPrivateFilePath(char* distortions_folder_path, char* filename, char* username) {
    char* full_path = NULL;  

    if(!username) {
        if (asprintf(&full_path, ".%s/%s", distortions_folder_path, filename) < 0) return NULL;  
    } else {
        if (asprintf(&full_path, ".%s/%s_%s", distortions_folder_path, username, filename) < 0) return NULL; 
    }

    return full_path;  
}

/*********************************************** 
* 
* @Finalidad: Construir una ruta completa a un archivo dentro de la carpeta compartida 
*             de distorsiones en curso, utilizando el nombre de usuario y el nombre del archivo. 
* 
* @Parámetros: 
* in: filename = Nombre del archivo. 
* in: username = Nombre del usuario asociado al archivo. 
* 
* @Retorno: 
*           Puntero a una cadena que contiene la ruta completa del archivo en la carpeta compartida. 
*           Retorna NULL si ocurre un error al construir la ruta. 
* 
************************************************/
char* FILE_buildSharedFilePath(char* filename, char* username) {
    char* global_directory = "../unfinished_distortions";
    char* global_file_path = NULL;

    // Construïm el path del fitxer al directori compartit de distorsions en curs
    if (asprintf(&global_file_path, "%s/%s_%s", global_directory, username, filename) < 0) return NULL;
    return global_file_path; 
}

/*********************************************** 
* 
* @Finalidad: Copiar un archivo desde una ruta de origen a una ruta de destino 
*             utilizando la herramienta `cp` en un proceso hijo. 
* 
* @Parámetros: 
* in: source_path = Ruta completa del archivo de origen. 
* in: destination_path = Ruta completa donde se copiará el archivo de destino. 
* 
* @Retorno: 
*           0 = La copia del archivo se realizó con éxito. 
*          -1 = Ocurrió un error durante el fork o la ejecución de la copia. 
* 
************************************************/
int FILE_copyFile(const char *source_path, const char *destination_path) {
    pid_t pid;

    pid = fork();  // Fem un fork per generar el procés fill
    if (pid == -1) {
        perror("fork failed");
        return -1;
    }

    // Codi del procés fill
    if (pid == 0) {
        // Executem la comanda 'cp' per copiar el fitxer
        execlp("cp", "cp", source_path, destination_path, (char *)NULL);

        // Si arribem a aquest punt significa que no s'ha substituït el codi del procés fill pel programa 'cp'
        return -1;
    } else {
        // Codi del procés pare
        wait(NULL);  // Esperem a que el procés fill acabi
    }

    return 0;  // Còpia realitzada amb èxit
}

/*********************************************** 
* 
* @Finalidad: Mover un archivo desde una ruta de origen a una ruta de destino 
*             utilizando la herramienta `mv` en un proceso hijo. 
* 
* @Parámetros: 
* in: source_path = Ruta completa del archivo de origen. 
* in: destination_path = Ruta completa donde se moverá el archivo de destino. 
* 
* @Retorno: 
*           0 = El archivo se movió con éxito. 
*          -1 = Ocurrió un error durante el fork o la ejecución del comando `mv`. 
* 
************************************************/
int FILE_moveFile(const char *source_path, const char *destination_path) {
    IO_printFormat(1, LAVENDER "Moving file from %s to %s\n" RESET, source_path, destination_path);
    pid_t pid;

    pid = fork();  // Fem un fork per generar el procés fill
    if (pid == -1) {
        perror("fork failed");
        return -1;
    }

    // Codi del procés fill
    if (pid == 0) {
        // Executem la comanda 'mv' per moure el fitxer
        execlp("mv", "mv", source_path, destination_path, (char *)NULL);
        
        // Si arribem a aquest punt significa que no s'ha substituït el codi del procés fill pel programa 'mv'
        perror("execlp failed");
        return -1;
    } else {
        // Codi del procés pare
        wait(NULL); // Esperem a que el procés fill acabi
    }

    return 0;  // Fitxer mogut amb èxit
}

/*int FILE_removeFile(const char *file_path) {
    if(!file_path) return -1;

    pid_t pid;
    pid = fork();  // Fem un fork per generar el procés fill
    if (pid == -1) {
        return -1;
    }

    // Codi del procés fill
    if (pid == 0) {
        // Executem la comanda 'rm' per eliminar el fitxer
        execlp("rm", "rm", file_path, (char *)NULL);

        // Si arribem a aquest punt significa que no s'ha substituït el codi del procés fill pel programa 'rm'
        return -1;
    } else {
        // Codi del procés pare
        wait(NULL);  // Esperem a que el procés fill acabi
    }

    return 0;  // Fitxer eliminat amb èxit
}*/

/*********************************************** 
* 
* @Finalidad: Obtener el tamaño de un archivo especificado por su ruta. 
* 
* @Parámetros: 
* in: full_path = Ruta completa del archivo cuyo tamaño se desea calcular. 
* 
* @Retorno: 
*           >= 0 = Tamaño del archivo en bytes. 
*           -1 = Ocurrió un error al intentar abrir el archivo. 
* 
************************************************/
int FILE_getFileSize(const char *full_path) {
    int file_fd; 
    int size; 

    file_fd = open(full_path, O_RDONLY);
    if(file_fd < 0) {
        return -1;
    }

    // Calculamos el tamaño del fichero
    size = lseek(file_fd, 0, SEEK_END);
    // Volvemos al principio del fichero
    lseek(file_fd, 0, SEEK_SET);

    close(file_fd);
    return size; 
}

/*********************************************** 
* 
* @Finalidad: Obtener la extensión de un archivo a partir de su nombre. 
* 
* @Parámetros: 
* in: filename = Nombre del archivo del cual se desea obtener la extensión. 
* 
* @Retorno: 
*           Puntero a una cadena que contiene la extensión del archivo (sin el punto). 
*           Cadena vacía (`""`) si el archivo no tiene extensión. 
* 
************************************************/
char *FILE_getFileExtension(const char *filename) {
    char *dot = strrchr(filename, '.'); 
    if (!dot || dot == filename) {
        return "";  
    }
    return dot + 1;  
}

/*********************************************** 
* 
* @Finalidad: Determinar el tipo de archivo (e.g., "Media", "Text" o "Unknown") 
*             en función de su extensión. 
* 
* @Parámetros: 
* in: filename = Nombre del archivo cuyo tipo se desea determinar. 
* in: print_mutex = Mutex para sincronizar los mensajes de impresión en caso de error. 
* 
* @Retorno: 
*           Puntero a una cadena dinámica que contiene el tipo de archivo:
*           - `"Media"` si la extensión coincide con audio o imágenes conocidas.
*           - `"Text"` si la extensión es `.txt`.
*           - `"Unknown"` si no se reconoce el tipo o si ocurre un error. 
* 
************************************************/
char* FILE_determineFileType(const char *filename, pthread_mutex_t *print_mutex) {
    if (!filename) {
        return strdup("Unknown");
    }

    // Obtenim l'extensió
    const char *extension = FILE_getFileExtension(filename);

    // Copiem l'extensió a memòria dinàmica i la convertim a minúscules
    size_t ext_len = strlen(extension);
    char *lowercase_ext = malloc(ext_len + 1);
    if (!lowercase_ext) {
        STRING_printF(print_mutex, STDOUT_FILENO, RED, "Error: Reserving memory for extension\n");
        return strdup("Unknown");
    }

    for (size_t i = 0; i < ext_len; i++) {
        lowercase_ext[i] = tolower(extension[i]);
    }
    lowercase_ext[ext_len] = '\0';

    // Comprova si l'extensió està a les llistes de media
    for (int j = 0; j < audioExtensionsSize; j++) {
        if (strcmp(lowercase_ext, audioExtensions[j]) == 0) {
            free(lowercase_ext);
            return strdup("Media");
        }
    }

    for (int j = 0; j < imageExtensionsSize; j++) {
        if (strcmp(lowercase_ext, imageExtensions[j]) == 0) {
            free(lowercase_ext);
            return strdup("Media");
        }
    }

    // Comprova si és un fitxer de text
    if (strcmp(lowercase_ext, "txt") == 0) {
        free(lowercase_ext);
        return strdup("Text");
    }

    // Si no coincideix amb cap tipus conegut
    free(lowercase_ext);
    return strdup("Unknown");
}

/*********************************************** 
* 
* @Finalidad: Calcular el hash MD5 de un archivo especificado utilizando el comando `md5sum`. 
* 
* @Parámetros: 
* in: file_path = Ruta completa del archivo cuyo MD5 se desea calcular. 
* 
* @Retorno: 
*           Puntero a una cadena dinámica que contiene el hash MD5 del archivo. 
*           Retorna NULL si ocurre un error durante la operación (e.g., fallo en el fork, 
*           error en la ejecución de `md5sum` o problemas con la pipe). 
* 
************************************************/
char* FILE_calculateMD5(const char *file_path) {
    int pipe_fd[2];  
    pid_t pid;

    // Creem una pipe
    if (pipe(pipe_fd) == -1) {
        return NULL;
    }

    pid = fork();  // Fem un fork per generar el procés fill on carregarem el programa md5sum
    if (pid == -1) {
        perror("fork failed");
        close(pipe_fd[0]); 
        close(pipe_fd[1]);
        return NULL;
    }

    // Codi del procés fill
    if (pid == 0) {
        close(pipe_fd[0]);  // Tanquem l'extrem de lectura de la pipe

        // Redirigim la sortida estàndard (pantalla) a l'extrem d'escriptura de la pipe 
        if (dup2(pipe_fd[1], STDOUT_FILENO) == -1) {
            close(pipe_fd[1]);
            return NULL;
        }
        close(pipe_fd[1]);  // Tanquem pipe d'escriptura una vegada duplicat

        // Carreguem programa md5sum i executem la comanda md5sum sobre el fitxer
        execlp("md5sum", "md5sum", file_path, (char *)NULL);

        // Si arribem a aquest punt significa que no s'ha substituït el codi del procés fill pel programa md5sum al procés fill
        return NULL;
    } else {
        // Codi del procés pare
        close(pipe_fd[1]);  // Tanquem extrem d'escriptura

        // Llegim md5sum de la pipe
        char *md5sum = IO_readUntil(pipe_fd[0], ' ');  

        close(pipe_fd[0]);  // Tanquem extrem de lectura de la pipe
        wait(NULL);  // Esperem a que acabi procés fill

        return md5sum; // Retornem md5sum
    }
}

/*********************************************** 
* 
* @Finalidad: Comparar el hash MD5 de un archivo con un hash MD5 esperado para verificar 
*             si ambos coinciden. 
* 
* @Parámetros: 
* in: original_md5 = Cadena que contiene el hash MD5 esperado. 
* in: file_path = Ruta completa del archivo cuyo hash MD5 se calculará y comparará. 
* 
* @Retorno: 
*           1 = Los hashes MD5 coinciden. 
*           0 = Los hashes MD5 no coinciden o ocurrió un error al calcular el MD5 del archivo. 
* 
************************************************/
int FILE_compareMD5(char* original_md5, char* file_path) {
    char* reassembled_file_md5 = NULL;

    // Calculem md5sum del fitxer reconstruït
    reassembled_file_md5 = FILE_calculateMD5(file_path);
    if (!reassembled_file_md5) {
        return 0;
    }
    int md5_match = !strcmp(original_md5, reassembled_file_md5)? 1 : 0;
    free(reassembled_file_md5);
    // Comparem md5 esperat amb md5 del fitxer rebut
    return md5_match;
}

/*********************************************** 
* 
* @Finalidad: Reemplazar el contenido de un archivo de destino con el contenido de un 
*             archivo de origen, utilizando un proceso hijo y el comando `cat`. 
* 
* @Parámetros: 
* in: source_path = Ruta completa del archivo de origen cuyo contenido se copiará. 
* in: destination_path = Ruta completa del archivo de destino que será sobrescrito. 
* 
* @Retorno: 
*           0 = El archivo de destino fue reemplazado con éxito. 
*          -1 = Ocurrió un error durante el fork, la apertura del archivo de destino, 
*               la redirección de salida o la ejecución del comando `cat`. 
* 
************************************************/
int FILE_replaceFile(const char *source_path, const char *destination_path) {
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork failed");
        return -1;
    }

    if (pid == 0) {
        int dest_fd = open(destination_path, O_WRONLY | O_TRUNC);
        if (dest_fd == -1) {
            perror("Failed to open destination file");
            return -1;
        }

        // Redirigim la sortida estàndard (pantalla) al fitxer destí
        if (dup2(dest_fd, STDOUT_FILENO) == -1) {
            perror("dup2 failed");
            close(dest_fd);
            return -1;
        }
        close(dest_fd);

        execlp("cat", "cat", source_path, (char *)NULL);
        perror("execlp failed");
    } else {
        if (wait(NULL) == -1) {
            perror("wait failed");
            return -1;
        }
    }

    return 0;
}


































