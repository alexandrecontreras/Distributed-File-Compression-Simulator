/***********************************************
*
* @Autores: Alexandre Contreras, Armand López.
* 
* @Finalidad: Proveer las funciones necesarias para manejar una lista enlazada de flecks 
*             (estructuras que representan clientes conectados en el sistema Gotham).
* 
* @Fecha de creación: 10 de octubre de 2024.
* 
* @Última modificación: 4 de enero de 2025.
* 
************************************************/

#include "fleckLinkedList.h"

/***********************************************
*
* @Finalitat: Crea i inicialitza una llista enllaçada buida de tipus `FleckLinkedList`.
* @Parametres: Cap.
* @Retorn:
*    - Un objecte `FleckLinkedList` inicialitzat.
*    - Si la reserva de memòria per al node inicial (capçalera) falla, s'estableix un codi d'error en la llista.
*
* @Descripció:
*    - Aquesta funció inicialitza una llista enllaçada buida. El primer node (capçalera) s'assigna a memòria dinàmica i el seu punter `next` s'estableix a `NULL`.
*    - També inicialitza els camps `previous` (apuntant a la capçalera) i `error` (indicant l'estat de l'operació).
*    - En cas que no es pugui al·locar memòria per al node inicial, el codi d'error de la llista es marca com `FLECK_LIST_ERROR_MALLOC`.
*
************************************************/
FleckLinkedList FLECK_LINKEDLIST_create() {
    FleckLinkedList list;
    list.head = (FleckNode*) malloc(sizeof(FleckNode));
    if (list.head != NULL) {
        list.head->next = NULL;
        list.previous = list.head;
        list.error = FLECK_LIST_NO_ERROR;
    } else {
        list.error = FLECK_LIST_ERROR_MALLOC;
    }
    return list;
}


/***********************************************
*
* @Finalitat: Afegeix un nou element a la llista enllaçada passada per referència.
* @Parametres:
*    - `FleckLinkedList *list`: Punter a la llista on s'afegirà l'element.
*    - `FleckElement element`: Element a afegir a la llista.
* @Retorn: Cap.
*
* @Descripció:
*    - Aquesta funció crea un nou node, hi emmagatzema l'element passat per paràmetre i l'afegeix després del node apuntat pel camp `previous` de la llista.
*    - Si la reserva de memòria per al nou node és exitosa, el camp `next` del node anterior s'actualitza per apuntar al nou node i `previous` s'actualitza per apuntar al nou node.
*    - En cas que la reserva de memòria falli, es marca el camp `error` de la llista com `FLECK_LIST_ERROR_MALLOC`.
*
************************************************/
void FLECK_LINKEDLIST_add(FleckLinkedList *list, FleckElement element) {
    FleckNode *new_node = (FleckNode*) malloc(sizeof(FleckNode));
    if (new_node != NULL) {
        new_node->element = element;
        new_node->next = list->previous->next;
        list->previous->next = new_node;
        list->previous = new_node;
        list->error = FLECK_LIST_NO_ERROR;
    } else {
        list->error = FLECK_LIST_ERROR_MALLOC;
    }
}


/***********************************************
*
* @Finalitat: Elimina el node actual apuntat per `previous->next` de la llista enllaçada.
* @Parametres:
*    - `FleckLinkedList *list`: Punter a la llista enllaçada des de la qual s'eliminarà el node.
* @Retorn: Cap.
*
* @Descripció:
*    - Aquesta funció elimina el node actual apuntat per `previous->next` a la llista enllaçada.
*    - Si la llista està al final (`previous->next == NULL`), el camp `error` es marca com `FLECK_LIST_ERROR_END` i no es fa cap acció.
*    - Si hi ha un node per eliminar:
*        - Es desa un punter temporal (`aux`) per al node actual.
*        - Es fa que `previous->next` apunti al següent node.
*        - Es fa `free` de la memòria associada a les dades dinàmiques (`username` i `ip`) i del node en si.
*        - Es marca el camp `error` com `FLECK_LIST_NO_ERROR`.
*    - Aquesta funció assegura que no hi hagi fuites de memòria per als camps dinàmics dels elements de la llista.
*
************************************************/
void FLECK_LINKEDLIST_remove(FleckLinkedList *list) {
    FleckNode *aux = NULL;
    if (FLECK_LINKEDLIST_isAtEnd(*list)) {
        list->error = FLECK_LIST_ERROR_END;
    } else {
        aux = list->previous->next;
        list->previous->next = list->previous->next->next;
        free(aux->element.username);
        free(aux->element.ip);
        free(aux);
        list->error = FLECK_LIST_NO_ERROR;
    }
}


/***********************************************
*
* @Finalitat: Obté l'element emmagatzemat al node actual apuntat per `previous->next` de la llista enllaçada.
* @Parametres:
*    - `FleckLinkedList *list`: Punter a la llista enllaçada des de la qual s'obtindrà l'element.
* @Retorn:
*    - Un `FleckElement` que conté l'element del node actual si aquest existeix.
*    - Si la llista està al final, retorna un element no inicialitzat.
*
* @Descripció:
*    - Aquesta funció obté l'element del node actual apuntat per `previous->next` a la llista enllaçada.
*    - Si la llista està al final (`previous->next == NULL`):
*        - Es marca el camp `error` com `FLECK_LIST_ERROR_END`.
*        - No es realitza cap operació sobre l'element.
*    - Si hi ha un node disponible:
*        - Es copia l'element emmagatzemat al node a una variable de tipus `FleckElement`.
*        - Es marca el camp `error` com `FLECK_LIST_NO_ERROR`.
*    - Aquesta funció no modifica la llista enllaçada ni el node actual, només retorna l'element.
*
************************************************/
FleckElement FLECK_LINKEDLIST_get(FleckLinkedList *list) {
    FleckElement element;
    if (FLECK_LINKEDLIST_isAtEnd(*list)) {
        list->error = FLECK_LIST_ERROR_END;
    } else {
        element = list->previous->next->element;
        list->error = FLECK_LIST_NO_ERROR;
    }
    return element;
}


/***********************************************
*
* @Finalitat: Comprova si la llista enllaçada està buida.
* @Parametres:
*    - `FleckLinkedList list`: Llista enllaçada a comprovar.
* @Retorn:
*    - `1` si la llista està buida (és a dir, no hi ha nodes després del capçal).
*    - `0` si la llista no està buida.
*
* @Descripció:
*    - Aquesta funció determina si la llista enllaçada conté elements.
*    - La llista es considera buida si el node capçal (`head`) no apunta a cap altre node (`head->next == NULL`).
*    - Si `head->next` apunta a un node, la llista conté almenys un element.
*    - La funció no modifica la llista.
*
************************************************/
int FLECK_LINKEDLIST_isEmpty(FleckLinkedList list) {
    return list.head->next == NULL;
}


/***********************************************
*
* @Finalitat: Posiciona el punter `previous` de la llista enllaçada al capçal de la llista.
* @Parametres:
*    - `FleckLinkedList *list`: Punter a la llista enllaçada.
* @Retorn: Cap.
*
* @Descripció:
*    - Aquesta funció restableix la posició del punter `previous` al capçal (`head`) de la llista.
*    - És útil quan es desitja reiniciar el recorregut de la llista des del principi.
*    - La funció no modifica els elements ni l'estructura de la llista, només ajusta el punter `previous`.
*
************************************************/
void FLECK_LINKEDLIST_goToHead(FleckLinkedList *list) {
    list->previous = list->head;
}


/***********************************************
*
* @Finalitat: Avança el punter `previous` al següent node de la llista enllaçada.
* @Parametres:
*    - `FleckLinkedList *list`: Punter a la llista enllaçada.
* @Retorn: Cap.
*
* @Descripció:
*    - Aquesta funció mou el punter `previous` al següent node de la llista.
*    - Si la llista ja és al final (no hi ha més nodes per avançar), estableix el codi d'error a `FLECK_LIST_ERROR_END`.
*    - En cas contrari, actualitza `previous` i restableix el codi d'error a `FLECK_LIST_NO_ERROR`.
*    - Útil per recórrer la llista enllaçada node a node.
*
************************************************/
void FLECK_LINKEDLIST_next(FleckLinkedList *list) {
    if (FLECK_LINKEDLIST_isAtEnd(*list)) {
        list->error = FLECK_LIST_ERROR_END;
    } else {
        list->previous = list->previous->next;
        list->error = FLECK_LIST_NO_ERROR;
    }
}


/***********************************************
*
* @Finalitat: Comprovar si el punter `previous` de la llista està apuntant al final de la llista enllaçada.
* @Parametres:
*    - `FleckLinkedList list`: Llista enllaçada a verificar.
* @Retorn:
*    - `1` (cert) si el punter `previous` està al final de la llista (és a dir, no hi ha més nodes).
*    - `0` (fals) si hi ha més nodes disponibles després del node actual.
*
* @Descripció:
*    - Aquesta funció comprova si el camp `next` del node apuntat per `previous` és `NULL`.
*    - Si `previous->next` és `NULL`, significa que no hi ha més nodes enllaçats i la llista ha arribat al final.
*    - És útil per determinar si es pot avançar més en la llista o si s'ha recorregut completament.
*
************************************************/
int FLECK_LINKEDLIST_isAtEnd(FleckLinkedList list) {
    return list.previous->next == NULL;
}


/***********************************************
*
* @Finalitat: Alliberar tota la memòria ocupada per la llista enllaçada, incloent-hi els nodes i els elements associats.
* @Parametres:
*    - `FleckLinkedList *list`: Punter a la llista enllaçada a destruir.
* @Retorn: Cap.
*
* @Descripció:
*    - Aquesta funció recorre tota la llista enllaçada i allibera:
*        1. Els nodes que contenen els elements.
*        2. Els camps dinàmics dels elements com `username` i `ip`.
*    - El primer node ("phantom node") no conté un element vàlid, per això es tracta de manera diferent.
*    - Quan es completa la destrucció, es reseteja la llista:
*        - `head` i `previous` es fixen a `NULL`.
*        - L'estat d'error es fixa a `FLECK_LIST_NO_ERROR`.
*    - Aquesta funció assegura que no hi hagi fuites de memòria relacionades amb la llista enllaçada.
*
************************************************/
void FLECK_LINKEDLIST_destroy(FleckLinkedList *list) {
    FleckNode *aux;
    int is_phantom = 1; 
    while (list->head != NULL) {
        aux = list->head;
        list->head = list->head->next;
        if(!is_phantom) {
            free(aux->element.username);
            free(aux->element.ip);
        }
        is_phantom = 0; 
        free(aux);
    }
    list->head = NULL;
    list->previous = NULL;
    list->error = FLECK_LIST_NO_ERROR;
}


/***********************************************
*
* @Finalitat: Obtenir el codi d'error actual de la llista enllaçada.
* @Parametres:
*    - `FleckLinkedList list`: Llista enllaçada de la qual es vol obtenir el codi d'error.
* @Retorn:
*    - Un enter que representa el codi d'error actual:
*        - `FLECK_LIST_NO_ERROR`: Si no hi ha cap error.
*        - Altres valors definits com a codis d'error (ex. `FLECK_LIST_ERROR_END`, `FLECK_LIST_ERROR_MALLOC`, etc.).
*
* @Descripció:
*    - Aquesta funció retorna l'últim estat d'error de la llista enllaçada.
*    - El codi d'error es pot utilitzar per diagnosticar problemes en operacions anteriors amb la llista.
*    - No modifica l'estat de la llista ni reinicia l'error.
*
************************************************/
int FLECK_LINKEDLIST_getErrorCode(FleckLinkedList list) {
    return list.error;
}
