#include "../include/pcb.h"

/*  siempre que trabaje con PCBs va a ser de esta forma
typedef struct
{
    uint32_t pid;
    t_estado_proceso estado;
    uint32_t quantum;
    uint64_t tiempo_q;
    t_contexto_ejecucion *contexto_ejecucion;
} t_pcb;
*/

// Inicializar Registros

void inicializar_contexto_y_registros(t_pcb *pcb)
{
    pcb->contexto_ejecucion = malloc(sizeof(t_contexto_ejecucion));
    pcb->contexto_ejecucion->registros = malloc(sizeof(t_registros));
    memset(pcb->contexto_ejecucion->registros, 0, sizeof(t_registros));
    pcb->contexto_ejecucion->motivo_desalojo = SIN_MOTIVO;
    pcb->contexto_ejecucion->motivo_finalizacion = FINALIZACION_SIN_MOTIVO;
}

// Funciones PCB

t_pcb *crear_pcb(uint32_t pid, t_estado_proceso estado, uint32_t quantum)
{
    t_pcb *pcb = malloc(sizeof(t_pcb));
    pcb->pid = pid;
    pcb->estado = estado;
    pcb->quantum = quantum;
    pcb->tiempo_q = 0;
    pcb->recursos_asignados = list_create();
    inicializar_contexto_y_registros(pcb);
    return pcb;
}

void destruir_pcb(t_pcb *pcb)
{
    list_destroy_and_destroy_elements(pcb->recursos_asignados, free);
    free(pcb->contexto_ejecucion->registros);
    free(pcb->contexto_ejecucion);
    free(pcb);
}

// Funciones Serializacion

t_buffer *crear_buffer_pcb(t_pcb *pcb)
{
    t_buffer *buffer = malloc(sizeof(t_buffer));

    size_t tam_registros = sizeof(uint32_t) +
                           sizeof(uint8_t) * 4 +
                           sizeof(uint32_t) * 6;

    buffer->size = sizeof(uint32_t) +
                   sizeof(t_estado_proceso) +
                   sizeof(uint32_t) +
                   sizeof(uint64_t) +
                   tam_registros +
                   sizeof(t_motivo_desalojo) +
                   sizeof(t_motivo_finalizacion);

    buffer->stream = malloc(buffer->size);
    int desplazamiento = 0;

    memcpy(buffer->stream + desplazamiento, &(pcb->pid), sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);

    memcpy(buffer->stream + desplazamiento, &(pcb->estado), sizeof(t_estado_proceso));
    desplazamiento += sizeof(t_estado_proceso);

    memcpy(buffer->stream + desplazamiento, &(pcb->quantum), sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);

    memcpy(buffer->stream + desplazamiento, &(pcb->tiempo_q), sizeof(uint64_t));
    desplazamiento += sizeof(uint64_t);

    memcpy(buffer->stream + desplazamiento, pcb->contexto_ejecucion->registros, tam_registros);
    desplazamiento += tam_registros;

    memcpy(buffer->stream + desplazamiento, &(pcb->contexto_ejecucion->motivo_desalojo), sizeof(t_motivo_desalojo));
    desplazamiento += sizeof(t_motivo_desalojo);

    memcpy(buffer->stream + desplazamiento, &(pcb->contexto_ejecucion->motivo_finalizacion), sizeof(t_motivo_finalizacion));

    return buffer;
}

t_paquete *crear_paquete_PCB(t_pcb *pcb)
{
    t_paquete *paquete = malloc(sizeof(t_paquete));
    paquete->codigo_operacion = PCB;
    paquete->buffer = crear_buffer_pcb(pcb);
    return paquete;
}

void deserializar_pcb(t_buffer *buffer, t_pcb *pcb)
{
    void *stream = buffer->stream;
    int desplazamiento = 0;

    memcpy(&(pcb->pid), stream + desplazamiento, sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);

    memcpy(&(pcb->estado), stream + desplazamiento, sizeof(t_estado_proceso));
    desplazamiento += sizeof(t_estado_proceso);

    memcpy(&(pcb->quantum), stream + desplazamiento, sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);

    memcpy(&(pcb->tiempo_q), stream + desplazamiento, sizeof(uint64_t));
    desplazamiento += sizeof(uint64_t);

    memcpy(pcb->contexto_ejecucion->registros, stream + desplazamiento, sizeof(t_registros));
    desplazamiento += sizeof(t_registros);

    memcpy(&(pcb->contexto_ejecucion->motivo_desalojo), stream + desplazamiento, sizeof(t_motivo_desalojo));
    desplazamiento += sizeof(t_motivo_desalojo);

    memcpy(&(pcb->contexto_ejecucion->motivo_finalizacion), stream + desplazamiento, sizeof(t_motivo_finalizacion));
}

// Funciones de Envio y Recepcion

void enviar_pcb(t_pcb *pcb, int socket_cliente)
{
    t_paquete *paquete = crear_paquete_PCB(pcb);
    enviar_paquete(paquete, socket_cliente);
    eliminar_paquete(paquete);
}

void recibir_pcb(t_pcb *pcb, int socket_cliente)
{
    t_paquete *paquete = recibir_paquete(socket_cliente);
    deserializar_pcb(paquete->buffer, pcb);
    eliminar_paquete(paquete);
}

uint32_t str_to_uint32(char *str)
{
    char *endptr;
    uint32_t result = (uint32_t)strtoul(str, &endptr, 10);

    // Comprobar si hubo errores durante la conversión
    if (*endptr != '\0')
    {
        fprintf(stderr, "Error en la conversión de '%s' a uint32_t.\n", str);
        exit(EXIT_FAILURE);
    }

    return result;
}

uint8_t str_to_uint8(char *str)
{
    char *endptr;
    uint8_t result = (uint8_t)strtoul(str, &endptr, 10);

    // Comprobar si hubo errores durante la conversión
    if (*endptr != '\0')
    {
        fprintf(stderr, "Error en la conversión de '%s' a uint8_t.\n", str);
        exit(EXIT_FAILURE);
    }

    return result;
}

char *estado_to_string(t_estado_proceso estado)
{
    switch (estado)
    {
    case NUEVO:
        return "NUEVO";
    case LISTO:
        return "LISTO";
    case EJECUTANDO:
        return "EJECUTANDO";
    case BLOQUEADO:
        return "BLOQUEADO";
    case FINALIZADO:
        return "TERMINADO";
    default:
        return "ERROR";
    }
}