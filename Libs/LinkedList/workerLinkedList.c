/***********************************************
*
* @Autores: Alexandre Contreras, Armand López.
* 
* @Finalidad: Proveer funciones para gestionar una lista enlazada de workers, diseñada para 
*             manejar elementos del sistema Gotham relacionados con workers de tipo Enigma y Harley.
* 
* @Fecha de creación: 10 de octubre de 2024.
* 
* @Última modificación: 4 de enero de 2025.
* 
************************************************/

#include "workerLinkedList.h"


/***********************************************
*
* @Finalitat: Crear una nova llista enllaçada per a gestionar els elements Worker.
* @Parametres: Cap.
* @Retorn:
*    - `WorkerLinkedList`: Una nova llista enllaçada inicialitzada.
*        - Si s'ha pogut al·locar memòria per al node inicial:
*            - `head` apunta al node inicial.
*            - `previous` apunta també al node inicial.
*            - `error` es configura a `WORKER_LIST_NO_ERROR`.
*        - Si no s'ha pogut al·locar memòria:
*            - `error` es configura a `WORKER_LIST_ERROR_MALLOC`.
*
* @Descripció:
*    - Inicialitza una llista enllaçada per a elements Worker amb un node "fantasma" com a capçalera.
*    - Si la memòria no es pot al·locar per al node inicial, marca l'error corresponent.
*    - Aquesta funció és essencial per a preparar una llista enllaçada abans d'utilitzar-la en altres operacions.
*
************************************************/
WorkerLinkedList WORKER_LINKEDLIST_create() {
    WorkerLinkedList list;
    list.head = (WorkerNode*) malloc(sizeof(WorkerNode));
    if (list.head != NULL) {
        list.head->next = NULL;
        list.previous = list.head;
        list.error = WORKER_LIST_NO_ERROR;
    } else {
        list.error = WORKER_LIST_ERROR_MALLOC;
    }
    return list;
}


/***********************************************
*
* @Finalitat: Afegir un nou element de tipus `WorkerElement` a la llista enllaçada.
* @Parametres:
*    - `WorkerLinkedList *list`: Punter a la llista enllaçada on s'afegirà l'element.
*    - `WorkerElement element`: Element a afegir a la llista.
* @Retorn: Cap.
*
* @Descripció:
*    - Aquesta funció crea un nou node de la llista enllaçada i insereix l'element proporcionat.
*    - El nou node s'insereix després del node referenciat per `previous` dins la llista.
*    - Si l'al·locació de memòria per al nou node falla, es configura `list->error` a `WORKER_LIST_ERROR_MALLOC`.
*    - Si l'operació es completa correctament, s'actualitza el punter `previous` per apuntar al nou node i es configura `list->error` a `WORKER_LIST_NO_ERROR`.
*
************************************************/
void WORKER_LINKEDLIST_add(WorkerLinkedList *list, WorkerElement element) {
    WorkerNode *new_node = (WorkerNode*) malloc(sizeof(WorkerNode));
    if (new_node != NULL) {
        new_node->element = element;
        new_node->next = list->previous->next;
        list->previous->next = new_node;
        list->previous = new_node;
        list->error = WORKER_LIST_NO_ERROR;
    } else {
        list->error = WORKER_LIST_ERROR_MALLOC;
    }
}


/***********************************************
*
* @Finalitat: Eliminar un element de la llista enllaçada a partir de la posició actual.
* @Parametres:
*    - `WorkerLinkedList *list`: Punter a la llista enllaçada de la qual es vol eliminar l'element.
* @Retorn: Cap.
*
* @Descripció:
*    - Aquesta funció elimina el node següent al node referenciat per `previous` dins de la llista enllaçada.
*    - Allibera la memòria associada amb els camps `worker_type` i `ip` de l'element, així com la memòria del node.
*    - Si la llista es troba al final (`isAtEnd`), es configura `list->error` a `WORKER_LIST_ERROR_END`.
*    - Si l'operació és exitosa, es configura `list->error` a `WORKER_LIST_NO_ERROR`.
*
************************************************/
void WORKER_LINKEDLIST_remove(WorkerLinkedList *list) {
    WorkerNode *aux = NULL;
    if (WORKER_LINKEDLIST_isAtEnd(*list)) {
        list->error = WORKER_LIST_ERROR_END;
    } else {
        aux = list->previous->next;
        list->previous->next = list->previous->next->next;
        free(aux->element.worker_type);
        free(aux->element.ip);
        free(aux);
        list->error = WORKER_LIST_NO_ERROR;
    }
}


/***********************************************
*
* @Finalitat: Obtenir l'element actual de la llista enllaçada.
* @Parametres:
*    - `WorkerLinkedList *list`: Punter a la llista enllaçada de la qual es vol obtenir l'element.
* @Retorn:
*    - `WorkerElement`: Element actual de la llista. Si la llista està al final, l'element retornat pot ser indeterminat.
*
* @Descripció:
*    - Aquesta funció retorna l'element del node actual a la llista, que es troba immediatament després de `previous`.
*    - Si la llista està al final (determinat per `WORKER_LINKEDLIST_isAtEnd`), s'estableix `list->error` a `WORKER_LIST_ERROR_END`.
*    - Si l'operació és exitosa, s'estableix `list->error` a `WORKER_LIST_NO_ERROR` i es retorna l'element.
*    - Aquesta funció no modifica la posició del punter `previous`.
*
************************************************/
WorkerElement WORKER_LINKEDLIST_get(WorkerLinkedList *list) {
    WorkerElement element;
    if (WORKER_LINKEDLIST_isAtEnd(*list)) {
        list->error = WORKER_LIST_ERROR_END;
    } else {
        element = list->previous->next->element;
        list->error = WORKER_LIST_NO_ERROR;
    }
    return element;
}


/***********************************************
*
* @Finalitat: Obtenir un punter a l'element actual de la llista enllaçada.
* @Parametres:
*    - `WorkerLinkedList *list`: Punter a la llista enllaçada de la qual es vol obtenir el punter a l'element.
* @Retorn:
*    - `WorkerElement*`: Punter a l'element actual de la llista. Si la llista està al final, retorna `NULL`.
*
* @Descripció:
*    - Aquesta funció retorna un punter a l'element del node actual a la llista, que es troba immediatament després de `previous`.
*    - Si la llista està al final (determinat per `WORKER_LINKEDLIST_isAtEnd`), retorna `NULL` i s'estableix `list->error` a `WORKER_LIST_ERROR_END`.
*    - Si l'operació és exitosa, s'estableix `list->error` a `WORKER_LIST_NO_ERROR` i es retorna el punter a l'element.
*    - Aquesta funció no modifica la posició del punter `previous`.
*    - És útil quan es desitja accedir directament a l'element per modificar-lo sense crear una còpia.
*
************************************************/
WorkerElement* WORKER_LINKEDLIST_getPointer(WorkerLinkedList *list) {
    if (WORKER_LINKEDLIST_isAtEnd(*list)) {
        list->error = WORKER_LIST_ERROR_END;
        return NULL;
    } else {
        list->error = WORKER_LIST_NO_ERROR;
        return &(list->previous->next->element);  
    }
}


/***********************************************
*
* @Finalitat: Comprovar si la llista enllaçada està buida.
* @Parametres:
*    - `WorkerLinkedList list`: La llista enllaçada que es vol verificar.
* @Retorn:
*    - `int`: Retorna 1 si la llista està buida (no conté nodes amb elements); en cas contrari, retorna 0.
*
* @Descripció:
*    - Aquesta funció comprova si el primer node després del node "fantasma" (`head`) de la llista enllaçada existeix.
*    - Una llista es considera buida si no hi ha nodes després del node "fantasma" (`head->next == NULL`).
*    - Aquesta funció no modifica la llista ni els seus nodes.
*    - És útil per validar l'estat inicial de la llista o abans d'operacions que requereixen nodes existents.
*
************************************************/
int WORKER_LINKEDLIST_isEmpty(WorkerLinkedList list) {
    return list.head->next == NULL;
}


/***********************************************
*
* @Finalitat: Posicionar el punter `previous` de la llista enllaçada al node "fantasma" (`head`).
* @Parametres:
*    - `WorkerLinkedList *list`: Punter a la llista enllaçada que es vol modificar.
* @Retorn: Cap.
*
* @Descripció:
*    - Aquesta funció reinicia la posició del punter `previous` de la llista al node "fantasma" (`head`).
*    - Això permet iterar sobre la llista des de l'inici en operacions subseqüents.
*    - No afecta els nodes ni els elements emmagatzemats a la llista.
*    - Útil quan cal començar a recórrer la llista des del principi.
*
************************************************/
void WORKER_LINKEDLIST_goToHead(WorkerLinkedList *list) {
    list->previous = list->head;
}


/***********************************************
*
* @Finalitat: Avançar el punter `previous` al següent node de la llista enllaçada.
* @Parametres:
*    - `WorkerLinkedList *list`: Punter a la llista enllaçada que es vol modificar.
* @Retorn: Cap.
*
* @Descripció:
*    - Aquesta funció mou el punter `previous` al següent node de la llista.
*    - Si el punter `previous` ja es troba al final de la llista, es retorna un codi d'error.
*    - L'atribut `error` de la llista s'actualitza amb:
*        - `WORKER_LIST_NO_ERROR` si l'operació s'ha completat amb èxit.
*        - `WORKER_LIST_ERROR_END` si `previous` ja estava al final de la llista.
*
************************************************/
void WORKER_LINKEDLIST_next(WorkerLinkedList *list) {
    if (WORKER_LINKEDLIST_isAtEnd(*list)) {
        list->error = WORKER_LIST_ERROR_END;
    } else {
        list->previous = list->previous->next;
        list->error = WORKER_LIST_NO_ERROR;
    }
}


/***********************************************
*
* @Finalitat: Comprovar si el punter `previous` de la llista enllaçada es troba al final de la llista.
* @Parametres:
*    - `WorkerLinkedList list`: Llista enllaçada que es vol consultar.
* @Retorn:
*    - `1` si el punter `previous` es troba al final de la llista.
*    - `0` si no es troba al final de la llista.
*
* @Descripció:
*    - Aquesta funció verifica si el node apuntat per `previous` és l'últim node de la llista.
*    - El final de la llista es determina quan el camp `next` del node apuntat per `previous` és `NULL`.
*
************************************************/
int WORKER_LINKEDLIST_isAtEnd(WorkerLinkedList list) {
    return list.previous->next == NULL;
}


/***********************************************
*
* @Finalitat: Destrueix una llista enllaçada de tipus `WorkerLinkedList`, alliberant tots els nodes i la memòria associada.
* @Parametres:
*    - `WorkerLinkedList *list`: Punter a la llista enllaçada que es vol destruir.
* @Retorn: Cap.
*
* @Descripció:
*    - Aquesta funció allibera tots els nodes d'una llista enllaçada, incloent-hi els camps dinàmics `worker_type` i `ip` dels elements dels nodes.
*    - El primer node (fantasma) es tracta de manera especial per evitar intentar alliberar camps que no són vàlids.
*    - Un cop alliberada tota la memòria, es reinicialitzen els punts `head` i `previous` a `NULL` i es reinicia el codi d'error a `WORKER_LIST_NO_ERROR`.
*
************************************************/
void WORKER_LINKEDLIST_destroy(WorkerLinkedList *list) {
    WorkerNode *aux;
    int is_phantom = 1;

    while (list->head != NULL) {
        aux = list->head;
        list->head = list->head->next;
        if(!is_phantom) { //per no intentar alliberar camps del node fantasma
            free(aux->element.worker_type);
            free(aux->element.ip);
        }
        is_phantom = 0; 
        free(aux);
    }
    list->head = NULL;
    list->previous = NULL;
    list->error = WORKER_LIST_NO_ERROR;
}

/***********************************************
*
* @Finalitat: Obté el codi d'error associat a una llista enllaçada de tipus `WorkerLinkedList`.
* @Parametres:
*    - `WorkerLinkedList list`: Llista enllaçada de la qual es vol obtenir el codi d'error.
* @Retorn:
*    - Enter que representa el codi d'error actual de la llista.
*
* @Descripció:
*    - Aquesta funció retorna el valor de l'atribut `error` de la llista enllaçada passada com a paràmetre.
*    - El codi d'error indica l'últim estat o problema detectat a la llista (per exemple, problemes de memòria o accés fora de límits).
*    - No modifica la llista.
*
************************************************/
int WORKER_LINKEDLIST_getErrorCode(WorkerLinkedList list) {
    return list.error;
}


/***********************************************
*
* @Finalitat: Calcula la longitud (nombre d'elements) d'una llista enllaçada de tipus `WorkerLinkedList`.
* @Parametres:
*    - `WorkerLinkedList list`: Llista enllaçada de la qual es vol determinar la longitud.
* @Retorn:
*    - Enter que indica el nombre d'elements presents a la llista.
*
* @Descripció:
*    - Aquesta funció recorre la llista enllaçada des del node fantasma fins al final, comptant cada node.
*    - Utilitza funcions de la biblioteca de la llista per moure's pels nodes de manera segura.
*    - La llista original no es modifica, ja que es passa una còpia de la mateixa com a paràmetre.
*
************************************************/
int WORKER_LINKEDLIST_length(WorkerLinkedList list) {
    int length = 0;
    WORKER_LINKEDLIST_goToHead(&list);  
    while (!WORKER_LINKEDLIST_isAtEnd(list)) {
        length++;
        WORKER_LINKEDLIST_next(&list);  
    }
    return length;
}


/***********************************************
*
* @Finalitat: Mou el punter de la llista a un índex específic dins d'una llista enllaçada de tipus `WorkerLinkedList`.
* @Parametres:
*    - `WorkerLinkedList *list`: Punter a la llista enllaçada que es vol modificar.
*    - `int index`: Índex al qual es vol moure el punter de la llista.
* @Retorn: Cap.
*
* @Descripció:
*    - Aquesta funció mou el punter `previous` de la llista a l'element situat a l'índex especificat.
*    - Comprova si l'índex és vàlid. Si no ho és, marca un error amb el codi `WORKER_LIST_ERROR_INDEX_OUT_OF_BOUNDS` i surt.
*    - Si l'índex és vàlid, reinicia la llista al cap i la recorre fins a arribar a l'índex desitjat utilitzant la funció `WORKER_LINKEDLIST_next`.
*    - En cas d'èxit, actualitza el codi d'error a `WORKER_LIST_NO_ERROR`.
*
************************************************/
void WORKER_LINKEDLIST_gotoIndex(WorkerLinkedList *list, int index) {
    if (index < 0 || index >= WORKER_LINKEDLIST_length(*list)) {
        list->error = WORKER_LIST_ERROR_INDEX_OUT_OF_BOUNDS;
        return;
    }

    WORKER_LINKEDLIST_goToHead(list);
    for (int i = 0; i < index; i++) {
        WORKER_LINKEDLIST_next(list);  
    }

    list->error = WORKER_LIST_NO_ERROR; 
}

