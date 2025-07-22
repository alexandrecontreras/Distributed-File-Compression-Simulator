/*********************************************** 
* 
* @Autores: Alexandre Contreras y Armand López
* @Propósito: Proveer funciones para la gestión de semáforos en sistemas operativos, incluyendo creación, inicialización, manipulación y eliminación.
* @Fecha de creación: 10 de octubre de 2024
* @Última modificación: 4 de enero de 2025
* 
************************************************/

#ifndef _MOD_SEMAPHORE_H_
#define _MOD_SEMAPHORE_H_

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <assert.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>

typedef struct {
    int shmid;
} semaphore;

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
int SEM_constructor_with_name(semaphore *sem, key_t key, int *created);

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
int SEM_constructor(semaphore *sem);

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
int SEM_init(const semaphore *sem, const int v);

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
int SEM_destructor(const semaphore *sem);

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
int SEM_wait(const semaphore *sem);

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
int SEM_signal(const semaphore *sem);

#endif 
