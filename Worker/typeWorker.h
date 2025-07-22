#ifndef _TYPE_WORKER_CUSTOM_H_
#define _TYPE_WORKER_CUSTOM_H_

#define DEC_WORKER_COUNTER  0
#define INC_WORKER_COUNTER  1
#define ENIGMA_COUNTER      2
#define HARLEY_COUNTER      3

#define STAGE_RECV_FILE     0
#define STAGE_CHECK_MD5     1
#define STAGE_DISTORT       2
#define STAGE_SND_METADATA  3
#define STAGE_SND_FILE      4
#define STAGE_FINISHED      5

#define MAX_CLIENTS 10

//Llibreries pròpies
#include "../Libs/Semaphore/semaphore_v2.h"                   // Per a les funcions de semàfors

typedef struct {
    char* gotham_ip; 
    int gotham_port;
    char* worker_ip; 
    int worker_port; 
    char* folder_path;
    char* worker_type;
} WorkerConfig;

typedef struct {
    int listen_socket;
    int n_clients; 
    int* clients;  
    pthread_mutex_t clients_mutex;
    pthread_t* active_threads;
    int active_thread_count;
    pthread_mutex_t thread_list_mutex;
} WorkerServer;

typedef struct {
    int client_socket;
    WorkerServer* server;
    volatile int* exit_distortion;
    char* distortions_folder_path;  
    semaphore* sWorkerCountMutex;
    char file_type;  
    pthread_mutex_t* print_mutex;       
} DistortionThreadArgsW; 

#endif // _TYPE_WORKER_CUSTOM_H_