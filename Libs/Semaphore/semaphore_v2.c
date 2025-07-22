
/***********************************************
*
* @Autores: Alexandre Contreras, Armand López.
* 
* @Finalidad: Implementar funciones para gestionar semáforos en sistemas Unix/Linux, 
*             incluyendo creación, inicialización, y operaciones básicas como 
*             `wait` y `signal`.
* 
* @Fecha de creación: 11 de diciembre de 2024.
* 
* @Última modificación: 4 de enero de 2025.
* 
************************************************/

#include "semaphore_v2.h"

/*********************************************** 
* 
* @Finalidad: Crear o adjuntar un semáforo identificado por una clave específica. 
*             Indica si el semáforo fue creado o si ya existía. 
* 
* @Parámetros: 
* in/out: sem = Puntero a la estructura `semaphore` donde se almacenará el identificador del semáforo. 
* in: key = Clave única utilizada para identificar el semáforo. 
* out: created = Puntero a un entero que indica si el semáforo fue creado (1) o si ya existía (0). 
* 
* @Retorno: 
*           0 = Operación exitosa. 
*          -1 = Error al crear o adjuntar el semáforo (se imprime el error en `stderr`). 
* 
************************************************/
int SEM_constructor_with_name(semaphore *sem, key_t key, int *created) {
    assert(sem != NULL);
    assert(created != NULL);

    sem->shmid = semget(key, 1, IPC_CREAT | IPC_EXCL | 0600);
    if (sem->shmid < 0) {
        if (errno == EEXIST) {
            *created = 0;
            sem->shmid = semget(key, 1, 0600);
            if (sem->shmid < 0) {
                perror("Failed to attach to existing semaphore");
                return -1;
            }
        } else {
            perror("Failed to create or attach to semaphore");
            return -1;
        }
    } else {
        *created = 1;
    }
    return 0;
}

/*********************************************** 
* 
* @Finalidad: Crear un semáforo único utilizando `IPC_PRIVATE`. 
* 
* @Parámetros: 
* in/out: sem = Puntero a la estructura `semaphore` donde se almacenará el identificador del semáforo. 
* 
* @Retorno: 
*           0 = Operación exitosa. 
*           <0 = Error al crear el semáforo (retorna el código de error de `semget`). 
* 
************************************************/
int SEM_constructor(semaphore *sem) {
    assert(sem != NULL);
    sem->shmid = semget(IPC_PRIVATE, 1, IPC_CREAT | 0600);
    if (sem->shmid < 0) return sem->shmid;
    return 0;
}

/*********************************************** 
* 
* @Finalidad: Inicializar un semáforo con un valor especificado. 
* 
* @Parámetros: 
* in: sem = Puntero a la estructura `semaphore` que contiene el identificador del semáforo. 
* in: v = Valor inicial que se asignará al semáforo. 
* 
* @Retorno: 
*           0 = Operación exitosa. 
*          -1 = Error al inicializar el semáforo (retorna el código de error de `semctl`). 
* 
************************************************/
int SEM_init(const semaphore *sem, const int v) {
    unsigned short _v[1] = {v};
    assert(sem != NULL);
    return semctl(sem->shmid, 0, SETALL, _v);
}

/*********************************************** 
* 
* @Finalidad: Eliminar un semáforo del sistema. 
* 
* @Parámetros: 
* in: sem = Puntero a la estructura `semaphore` que contiene el identificador del semáforo a eliminar. 
* 
* @Retorno: 
*           0 = Operación exitosa. 
*          -1 = Error al eliminar el semáforo (retorna el código de error de `semctl`). 
* 
************************************************/
int SEM_destructor(const semaphore *sem) {
    assert(sem != NULL);
    return semctl(sem->shmid, 0, IPC_RMID, NULL);
}

/*********************************************** 
* 
* @Finalidad: Realizar una operación de espera (decremento) en un semáforo, bloqueando 
*             si su valor es cero. 
* 
* @Parámetros: 
* in: sem = Puntero a la estructura `semaphore` que contiene el identificador del semáforo. 
* 
* @Retorno: 
*           0 = Operación exitosa. 
*          -1 = Error al realizar la operación (retorna el código de error de `semop`). 
* 
************************************************/
int SEM_wait(const semaphore *sem) {
    struct sembuf o = {0, -1, SEM_UNDO};
    assert(sem != NULL);
    return semop(sem->shmid, &o, 1);
}

/*********************************************** 
* 
* @Finalidad: Incrementar el valor de un semáforo, desbloqueando procesos en espera si los hay. 
* 
* @Parámetros: 
* in: sem = Puntero a la estructura `semaphore` que contiene el identificador del semáforo. 
* 
* @Retorno: 
*           0 = Operación exitosa. 
*          -1 = Error al realizar la operación (retorna el código de error de `semop`). 
* 
************************************************/
int SEM_signal(const semaphore *sem) {
    struct sembuf o = {0, 1, SEM_UNDO};
    assert(sem != NULL);
    return semop(sem->shmid, &o, 1);
}
