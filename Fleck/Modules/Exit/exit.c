/*********************************************** 
* 
* @Autores: Alexandre Contreras, Armand López. 
* 
* @Finalidad: Implementar funciones para la gestión y liberación de memoria dinámica 
*             asociada a las estructuras utilizadas en el sistema de distorsión, 
*             incluyendo configuración, contextos de distorsión y registros. 
* 
* @Fecha de creación: 25 de octubre de 2024. 
* 
* @Última modificación: 4 de enero de 2025. 
* 
************************************************/

#include "exit.h"

/*********************************************** 
* 
* @Finalidad: Liberar la memoria dinámica asociada a las estructuras utilizadas en 
*             el sistema, incluyendo la configuración de Fleck, contextos de distorsión, 
*             workers principales y el registro de distorsiones. 
* 
* @Parámetros: 
* in/out: fleck_config = Puntero a la estructura `FleckConfig` cuya memoria será liberada. 
* in/out: text_context = Puntero al contexto de distorsión de archivos de texto. 
* in/out: media_context = Puntero al contexto de distorsión de archivos de medios. 
* in/out: main_enigma = Puntero a la estructura del worker principal de tipo Enigma. 
* in/out: main_harley = Puntero a la estructura del worker principal de tipo Harley. 
* in/out: distortion_record = Puntero al registro de distorsiones que será liberado. 
* 
* @Retorno: Ninguno. 
* 
************************************************/
void EXIT_freeMemory(FleckConfig *fleck_config, DistortionContext *text_context, DistortionContext *media_context, MainWorker *main_enigma, MainWorker *main_harley, DistortionRecord* distortion_record) {
    //alliberem estructura configuració
    freePointer((void**)&fleck_config->username);
    freePointer((void**)&fleck_config->folder_path);
    freePointer((void**)&fleck_config->gotham_ip);

    //alliberem estructura de propietats del fitxer text a distorsionar
    freePointer((void**)&text_context->filename);
    freePointer((void**)&text_context->md5sum);
    freePointer((void**)&text_context->file_path); 

    //alliberem estructura de propietats del fitxer media a distorsionar
    freePointer((void**)&media_context->filename);
    freePointer((void**)&media_context->md5sum);
    freePointer((void**)&media_context->file_path); 

    //alliberem estructura enigma principal
    freePointer((void**)&main_enigma->ip);

    //alliberem estructura harley principal
    freePointer((void**)&main_harley->ip);

    EXIT_freeDistortionRecord(distortion_record);
}

/*********************************************** 
* 
* @Finalidad: Liberar la memoria dinámica asociada a los campos de una estructura 
*             `DistortionContext`. 
* 
* @Parámetros: 
* in/out: context = Puntero a la estructura `DistortionContext` cuya memoria será liberada. 
* 
* @Retorno: Ninguno. 
* 
************************************************/

void EXIT_cleanupDistortionContext(DistortionContext **context) {
    freePointer((void**)&((*context)->filename));
    freePointer((void**)&((*context)->md5sum));
    freePointer((void**)&((*context)->file_path));
    freePointer((void**)&((*context)->username));
}

/*********************************************** 
* 
* @Finalidad: Liberar la memoria dinámica asociada a las entradas de un registro 
*             de distorsiones (`DistortionRecord`) y reiniciar el contador de distorsiones. 
* 
* @Parámetros: 
* in/out: record = Puntero a la estructura `DistortionRecord` cuya memoria será liberada. 
* 
* @Retorno: Ninguno. 
* 
************************************************/
void EXIT_freeDistortionRecord(DistortionRecord* record) {
    for(int i = 0; i < record->n_distortions; i++) {
        freePointer((void**)&record->distortions[i].filename);
    }
    freePointer((void**)&record->distortions);
    record->n_distortions = 0;
}