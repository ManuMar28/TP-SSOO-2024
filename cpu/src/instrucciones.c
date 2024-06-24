#include "../include/instrucciones.h"
#include <ctype.h>

// (Registro, Valor): Asigna al registro el valor pasado como parámetro.
void _set(char *registro, char *valor)
{
    if (revisar_registro(registro))
    {
        *(get_registry8(registro)) = str_to_uint8(valor);
    }
    else
    {
        *(get_registry32(registro)) = str_to_uint32(valor);
    }
}

// (Registro Datos, Registro Dirección): Lee el valor de memoria correspondiente a
// la Dirección Lógica que se encuentra en el Registro Dirección y lo almacena en el Registro Datos.
void _mov_in(char *registro, char *direc_logica, int socket)
{
    uint32_t direccionLogica32;
    t_list *datosRecibidos = list_create();

    if (revisar_registro(direc_logica))
    {
        direccionLogica32 = *(get_registry8(direc_logica));
    }
    else
    {
        direccionLogica32 = *(get_registry32(direc_logica));
    }

    uint32_t tamanio_registro = obtener_tamanio_registro(registro);
    // printf("Tamanio registro antes de serializarlor y enviarlo MOV_IN: %d \n", tamanio_registro);
    t_list *Lista_direccionesFisica = list_create();
    Lista_direccionesFisica = traducir_direccion(pcb_actual->pid, direccionLogica32, TAM_PAGINA, tamanio_registro);
    // printf("tamanio de la lista de direcciones fisicas: %d \n", list_size(Lista_direccionesFisica));
    enviar_valor_mov_in_cpu(Lista_direccionesFisica, socket, pcb_actual->pid);
    datosRecibidos = recibir_dato_de_memoria_movIn(socket, LOGGER_CPU);

    // uint32_t registro_final = 0;
    for (int g = 0; g < list_size(datosRecibidos); g++)
    {
        char *aMostrar = list_get(datosRecibidos, g);
        // printf("Valor YA LEIDO de memoria: %s \n", aMostrar);

        if (strcmp(aMostrar, "") != 0)
        {
            if (revisar_registro(registro))
            {
                //*(get_registry8(registro)) = datoint;
                *(get_registry8(registro)) = str_to_uint8(aMostrar);
                // registro_final = *(get_registry8(registro));
            }
            else
            {
                // *(get_registry32(registro)) = datoint;
                *(get_registry32(registro)) = str_to_uint32(aMostrar);
                // registro_final = *(get_registry32(registro));
            }
        }
    }

    // printf("Registro Obtenido %d \n", registro_final);
    // mem_hexdump(datoObtenido, strlen(datoObtenido));

    for (int i = 0; i < list_size(Lista_direccionesFisica); i++)
    {
        t_direcciones_fisicas *direccionAmostrar = list_get(Lista_direccionesFisica, i);
        char *aMostrar = list_get(datosRecibidos, i);

        log_info(LOGGER_CPU, "PID: %d - Acción: LEER - Dirección Física: %d - Valor: %s", pcb_actual->pid, direccionAmostrar->direccion_fisica, aMostrar);
        free(aMostrar);
    }

    // printf("Registro Obtenido %d \n", registroFina);

    list_destroy_and_destroy_elements(Lista_direccionesFisica, free); // FIJARSE BIEN
}

//(Registro Dirección, Registro Datos): Lee el valor del Registro Datos y lo escribe en
// la dirección física de memoria obtenida a partir de la Dirección Lógica almacenada en el Registro Dirección.
void _mov_out(char *direc_logica, char *registro, int socket)
{
    uint32_t direccionLogica32;
    uint32_t valorObtenido;

    if (revisar_registro(direc_logica))
    {
        direccionLogica32 = *(get_registry8(direc_logica));
    }
    else
    {
        direccionLogica32 = *(get_registry32(direc_logica));
    }

    if (revisar_registro(registro))
    {
        valorObtenido = *(get_registry8(registro));
    }
    else
    {
        valorObtenido = *(get_registry32(registro));
    }

    // printf("Valor a escribir antes de serializarlor y enviarlo MOV_OUT: %d \n", valorObtenido);
    uint32_t tamanio_registro = obtener_tamanio_registro(registro);
    // printf("Tamanio registro antes de serializarlor y enviarlo MOV_OUT: %d \n", tamanio_registro);
    t_list *Lista_direccionesFisica = traducir_direccion(pcb_actual->pid, direccionLogica32, TAM_PAGINA, tamanio_registro);

    enviar_valor_mov_out_cpu(Lista_direccionesFisica, valorObtenido, socket, pcb_actual->pid);

    for (int i = 0; i < list_size(Lista_direccionesFisica); i++)
    {
        t_direcciones_fisicas *direccionAmostrar = list_get(Lista_direccionesFisica, i);
        // int *valor_parcial_a_pasar = malloc(4);
        // memset(valor_parcial_a_pasar, 0, 4);
        // memcpy(valor_parcial_a_pasar, &valorObtenido, direccionAmostrar->tamanio);
        if (i == 0)
        {
            log_info(LOGGER_CPU, "PID: %d - Acción: ESCRIBIR - Dirección Física: %d - Valor: %d", pcb_actual->pid, direccionAmostrar->direccion_fisica, valorObtenido);
        }
    }

    list_destroy_and_destroy_elements(Lista_direccionesFisica, free); // FIJARSE BIEN
}

// (Registro Destino, Registro Origen): Suma al Registro Destino el
// Registro Origen y deja el resultado en el Registro Destino.
void _sum(char *registro_destino, char *registro_origen)
{
    uint8_t *destino8 = NULL;
    uint32_t *destino32 = NULL;
    uint8_t *origen8 = NULL;
    uint32_t *origen32 = NULL;
    int is_destino_8bit = revisar_registro(registro_destino);
    int is_origen_8bit = revisar_registro(registro_origen);

    if (is_destino_8bit)
    {
        destino8 = get_registry8(registro_destino);
    }
    else
    {
        destino32 = get_registry32(registro_destino);
    }

    if (is_origen_8bit)
    {
        origen8 = get_registry8(registro_origen);
    }
    else
    {
        origen32 = get_registry32(registro_origen);
    }

    if (is_destino_8bit && is_origen_8bit)
    {
        *destino8 = *destino8 + *origen8;
    }
    else if (!is_destino_8bit && !is_origen_8bit)
    {
        *destino32 = *destino32 + *origen32;
    }
    else
    {
        // Manejo de error si los registros son de tamaños diferentes
        log_error(LOGGER_CPU, "Error: registros de diferentes tamaños no pueden ser sumados.");
    }
}

// (Registro Destino, Registro Origen): Resta al Registro Destino
// el Registro Origen y deja el resultado en el Registro Destino
void _sub(char *registro_destino, char *registro_origen)
{
    uint8_t *destino8 = NULL;
    uint32_t *destino32 = NULL;
    uint8_t *origen8 = NULL;
    uint32_t *origen32 = NULL;
    int is_destino_8bit = revisar_registro(registro_destino);
    int is_origen_8bit = revisar_registro(registro_origen);

    if (is_destino_8bit)
    {
        destino8 = get_registry8(registro_destino);
    }
    else
    {
        destino32 = get_registry32(registro_destino);
    }

    if (is_origen_8bit)
    {
        origen8 = get_registry8(registro_origen);
    }
    else
    {
        origen32 = get_registry32(registro_origen);
    }

    if (is_destino_8bit && is_origen_8bit)
    {
        *destino8 = *destino8 - *origen8;
    }
    else if (!is_destino_8bit && !is_origen_8bit)
    {
        *destino32 = *destino32 - *origen32;
    }
    else
    {
        // Manejo de error si los registros son de tamaños diferentes
        log_error(LOGGER_CPU, "Error: registros de diferentes tamaños no pueden ser restados.");
    }
}

// (Registro, Instrucción): Si el valor del registro es distinto de cero,
// actualiza el program counter al número de instrucción pasada por parámetro.
void _jnz(char *registro, char *instruccion)
{
    uint8_t *regis8 = NULL;
    uint32_t *regis32 = NULL;
    int is_regis_8bit = revisar_registro(registro);

    if (is_regis_8bit)
    {
        regis8 = get_registry8(registro);
        if (*regis8 != 0)
        {
            pcb_actual->contexto_ejecucion->registros->program_counter = str_to_uint32(instruccion);
        }
        else
        {
            log_warning(LOGGER_CPU, "El registro %s es igual a cero, no se actualiza el IP", registro);
        }
    }
    else
    {
        regis32 = get_registry32(registro);
        if (*regis32 != 0)
        {
            pcb_actual->contexto_ejecucion->registros->program_counter = str_to_uint32(instruccion);
        }
        else
        {
            log_warning(LOGGER_CPU, "El registro %s es igual a cero, no se actualiza el IP", registro);
        }
    }
}

void _resize(char *tamanioAReasignar)
{
    uint32_t tamanioAReasignarNum = str_to_uint32(tamanioAReasignar);
    enviar_resize_a_memoria(pcb_actual, tamanioAReasignarNum);
    // recibo el codigo de operacion de la memoria
    op_cod cop;
    recv(fd_cpu_memoria, &cop, sizeof(op_cod), 0);
    switch (cop)
    {
    case RESIZE_OK:
        log_debug(LOGGER_CPU, "Se redimensiono correctamente");
        break;
    case MISMO_TAMANIO:
        log_debug(LOGGER_CPU, "El tamanio es el mismo");
        break;
    case OUT_OF_MEMORY:
        log_debug(LOGGER_CPU, "No hay memoria suficiente");
        pcb_actual->contexto_ejecucion->motivo_desalojo = INTERRUPCION_OUT_OF_MEMORY;
        pcb_actual->contexto_ejecucion->motivo_finalizacion = OUT_OF_MEMORY;
        esSyscall = true;
        break;
    default:
        log_error(LOGGER_CPU, "Recibi un dato no valido");
        break;
    }
}

void _copy_string(char *tamanio, int socket_cliente) // tamanio es el tamaño del string que se va a leer y escribir
{
    uint32_t direc_logica_escritura = *(get_registry32("DI"));
    uint32_t direc_logica_lectura = *(get_registry32("SI"));
    uint32_t tamanio_string = str_to_uint32(tamanio);

    t_list *lista_direccionesFisica_lectura = traducir_direccion(pcb_actual->pid, direc_logica_lectura, TAM_PAGINA, tamanio_string);
    t_list *lista_direccionesFisica_escritura = traducir_direccion(pcb_actual->pid, direc_logica_escritura, TAM_PAGINA, tamanio_string);
    enviar_datos_copy_string(lista_direccionesFisica_escritura, lista_direccionesFisica_lectura, tamanio_string, socket_cliente, pcb_actual->pid);
    list_destroy_and_destroy_elements(lista_direccionesFisica_lectura, free);   // FIJARSE BIEN
    list_destroy_and_destroy_elements(lista_direccionesFisica_escritura, free); // FIJARSE BIEN
}

void _wait(char *recurso, int cliente_socket)
{
    enviar_recurso(pcb_actual, recurso, cliente_socket, PEDIDO_WAIT);
}

void _signal(char *recurso, int cliente_socket)
{
    enviar_recurso(pcb_actual, recurso, cliente_socket, PEDIDO_SIGNAL);
}

void _io_gen_sleep(char *interfaz, char *unidades_de_trabajo, int cliente_socket)
{
    int unidades_de_trabajoNum = atoi(unidades_de_trabajo);
    enviar_interfaz_IO(pcb_actual, interfaz, unidades_de_trabajoNum, cliente_socket, IO_GEN_SLEEP);
}

void _io_stdin_read(char *interfaz, char *direc_logica, char *tamanio, int cliente_socket)
{
    uint32_t direccionLogica32;

    if (revisar_registro(direc_logica))
    {
        direccionLogica32 = *(get_registry8(direc_logica));
        // printf("Direccion 1 recibido: %d\n", direccionLogica32);
    }
    else
    {

        direccionLogica32 = *(get_registry32(direc_logica));
        // printf("Direccion recibido: %d\n", direccionLogica32);
    }

    uint32_t tamanioMaximoAingresar;
    if (revisar_registro(tamanio))
    {
        tamanioMaximoAingresar = *(get_registry8(tamanio));
        // printf("TamMaximo 1 recibido: %d\n", tamanioMaximoAingresar);
    }
    else
    {

        tamanioMaximoAingresar = *(get_registry32(tamanio));
        // printf("TamMaximo recibido: %d\n", tamanioMaximoAingresar);
    }

    t_list *Lista_direccionesFisica = traducir_direccion(pcb_actual->pid, direccionLogica32, TAM_PAGINA, tamanioMaximoAingresar);

    enviar_interfaz_IO_stdin(pcb_actual, interfaz, Lista_direccionesFisica, cliente_socket, IO_STDIN_READ);

    void liberar_direccion_fisica(void *direccion)
    {
        free((t_direcciones_fisicas *)direccion);
    }

    // printf("Tamanio de la lista de direcciones fisicas: %d \n", list_size(Lista_direccionesFisica));
    list_destroy_and_destroy_elements(Lista_direccionesFisica, liberar_direccion_fisica); // FIJARSE BIEN
}

void _io_stdout_write(char *interfaz, char *direc_logica, char *tamanio, int cliente_socket)
{
    uint32_t direccionLogica32;

    if (revisar_registro(direc_logica))
    {
        direccionLogica32 = *(get_registry8(direc_logica));
        // printf("Direccion 1 recibido: %d\n", direccionLogica32);
    }
    else
    {

        direccionLogica32 = *(get_registry32(direc_logica));
        // printf("Direccion recibido: %d\n", direccionLogica32);
    }

    uint32_t tamanioMaximoAingresar;
    if (revisar_registro(tamanio))
    {
        tamanioMaximoAingresar = *(get_registry8(tamanio));
        // printf("TamMaximo 1 recibido: %d\n", tamanioMaximoAingresar);
    }
    else
    {

        tamanioMaximoAingresar = *(get_registry32(tamanio));
        // printf("TamMaximo recibido: %d\n", tamanioMaximoAingresar);
    }

    t_list *Lista_direccionesFisica = traducir_direccion(pcb_actual->pid, direccionLogica32, TAM_PAGINA, tamanioMaximoAingresar);

    enviar_interfaz_IO_stdout(pcb_actual, interfaz, Lista_direccionesFisica, cliente_socket, IO_STDOUT_WRITE);

    // printf("Tamanio de la lista de direcciones fisicas: %d \n", list_size(Lista_direccionesFisica));
    list_destroy_and_destroy_elements(Lista_direccionesFisica, free); // FIJARSE BIEN
}

void _io_fs_create(char *interfaz, char *nombre_archivo, int cliente_socket)
{
    enviar_fs_create_delete(pcb_actual, interfaz, nombre_archivo, cliente_socket, IO_FS_CREATE);
}

void _io_fs_delete(char *interfaz, char *nombre_archivo, int cliente_socket)
{
    enviar_fs_create_delete(pcb_actual, interfaz, nombre_archivo, cliente_socket, IO_FS_DELETE);
}

void _io_fs_truncate(char *interfaz, char *nombre_archivo, char *tamanio, int cliente_socket)
{
    uint32_t tamanioTruncar;
    if (revisar_registro(tamanio))
    {
        tamanioTruncar = *(get_registry8(tamanio));
    }
    else
    {

        tamanioTruncar = *(get_registry32(tamanio));
    }

    enviar_fs_truncate(pcb_actual, interfaz, nombre_archivo, tamanioTruncar, cliente_socket);
}

void _io_fs_write(char *interfaz, char *nombre_archivo, char *direc_logica, char *tamanio, char *puntero_archivo, int cliente_socket)
{
    // enviar_interfaz_IO(pcb_actual, interfaz, 0, cliente_socket, IO_FS_WRITE);
}

void _io_fs_read(char *interfaz, char *nombre_archivo, char *direc_logica, char *tamanio, char *puntero_archivo, int cliente_socket)
{
    // enviar_interfaz_IO(pcb_actual, interfaz, 0, cliente_socket, IO_FS_READ);
}

// ENVIOS

void enviar_recurso(t_pcb *pcb, char *recurso, int cliente_socket, op_cod codigo_operacion)
{
    t_paquete *paquete = crear_paquete_con_codigo_de_operacion(codigo_operacion);
    serializar_recurso(pcb, recurso, paquete);
    enviar_paquete(paquete, cliente_socket);
    eliminar_paquete(paquete);
}

void serializar_recurso(t_pcb *pcb, char *recurso, t_paquete *paquete)
{
    uint32_t tamanio_recurso = strlen(recurso) + 1;

    size_t tam_registros = sizeof(uint32_t) +
                           sizeof(uint8_t) * 4 +
                           sizeof(uint32_t) * 6;

    paquete->buffer->size = sizeof(uint32_t) +
                            sizeof(t_estado_proceso) +
                            sizeof(uint32_t) +
                            sizeof(uint64_t) +
                            tam_registros +
                            sizeof(t_motivo_desalojo) +
                            sizeof(t_motivo_finalizacion) +
                            sizeof(uint32_t) +
                            tamanio_recurso;

    paquete->buffer->stream = malloc(paquete->buffer->size);
    int desplazamiento = 0;

    memcpy(paquete->buffer->stream + desplazamiento, &(pcb->pid), sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);

    memcpy(paquete->buffer->stream + desplazamiento, &(pcb->estado), sizeof(t_estado_proceso));
    desplazamiento += sizeof(t_estado_proceso);

    memcpy(paquete->buffer->stream + desplazamiento, &(pcb->quantum), sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);

    memcpy(paquete->buffer->stream + desplazamiento, &(pcb->tiempo_q), sizeof(uint64_t));
    desplazamiento += sizeof(uint64_t);

    memcpy(paquete->buffer->stream + desplazamiento, pcb->contexto_ejecucion->registros, tam_registros);
    desplazamiento += tam_registros;

    memcpy(paquete->buffer->stream + desplazamiento, &(pcb->contexto_ejecucion->motivo_desalojo), sizeof(t_motivo_desalojo));
    desplazamiento += sizeof(t_motivo_desalojo);

    memcpy(paquete->buffer->stream + desplazamiento, &(pcb->contexto_ejecucion->motivo_finalizacion), sizeof(t_motivo_finalizacion));
    desplazamiento += sizeof(t_motivo_finalizacion);

    memcpy(paquete->buffer->stream + desplazamiento, &tamanio_recurso, sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);

    memcpy(paquete->buffer->stream + desplazamiento, recurso, tamanio_recurso);
}

void enviar_resize_a_memoria(t_pcb *pcb, uint32_t tamanioAReasignar)
{
    t_paquete *paquete = crear_paquete_con_codigo_de_operacion(PEDIDO_RESIZE);
    serializar_resize(pcb, tamanioAReasignar, paquete);
    enviar_paquete(paquete, fd_cpu_memoria);
    eliminar_paquete(paquete);
}

void serializar_resize(t_pcb *pcb, uint32_t tamanioAReasignar, t_paquete *paquete)
{
    paquete->buffer->size = sizeof(uint32_t) + sizeof(uint32_t);
    paquete->buffer->stream = malloc(paquete->buffer->size);
    int offset = 0;
    memcpy(paquete->buffer->stream + offset, &(pcb->pid), sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(paquete->buffer->stream + offset, &tamanioAReasignar, sizeof(uint32_t));
}

// Mover todo esto a otro lado obviamente
// Estaba en utis_memoria y lo movi aca de manera preliminar, despues se vera donde lo metemos

char *recibir_valor_mov_in_memoria(int socket)
{
    t_paquete *paquete = recibir_paquete(socket);
    char *valor_recibido = deserializar_valor_mov_in_memoria(paquete->buffer);
    eliminar_paquete(paquete);
    return valor_recibido;
}

char *deserializar_valor_mov_in_memoria(t_buffer *buffer)
{
    char *valor;

    uint32_t long_char;
    void *stream = buffer->stream;
    int desplazamiento = 0;

    memcpy(&(long_char), stream + desplazamiento, sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);

    valor = malloc(long_char);

    memcpy(valor, stream + desplazamiento, long_char);

    return valor;
}

void enviar_valor_mov_in_cpu(t_list *Lista_direccionesFisica, int socket, uint32_t pid)
{
    t_paquete *paquete_mov_in = crear_paquete_con_codigo_de_operacion(PEDIDO_MOV_IN);
    serializar_datos_mov_in(paquete_mov_in, Lista_direccionesFisica, pid);
    // printf("tamaño lista: %d \n", list_size(Lista_direccionesFisica));

    enviar_paquete(paquete_mov_in, socket);
    eliminar_paquete(paquete_mov_in);
}

void serializar_datos_mov_in(t_paquete *paquete, t_list *Lista_direccionesFisica, uint32_t pid)
{
    // Calculamos el tamaño total del buffer necesario
    paquete->buffer->size = list_size(Lista_direccionesFisica) * sizeof(t_direcciones_fisicas) + sizeof(uint32_t);
    // printf("Tamanio del buffer: %d \n", paquete->buffer->size);
    //  Reservamos la memoria para el buffer
    paquete->buffer->stream = malloc(paquete->buffer->size);

    int desplazamiento = 0;

    // Iteramos sobre la lista de datos y serializamos cada elemento
    for (int i = 0; i < list_size(Lista_direccionesFisica); i++)
    {
        t_direcciones_fisicas *dato = list_get(Lista_direccionesFisica, i);
        memcpy(paquete->buffer->stream + desplazamiento, &(dato->direccion_fisica), sizeof(uint32_t));
        desplazamiento += sizeof(uint32_t);
        memcpy(paquete->buffer->stream + desplazamiento, &(dato->tamanio), sizeof(uint32_t));
        desplazamiento += sizeof(uint32_t);
    }
    memcpy(paquete->buffer->stream + desplazamiento, &pid, sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);
}

void enviar_valor_mov_out_cpu(t_list *Lista_direccionesFisica, uint32_t valorObtenido, int socket, uint32_t pid)
{
    t_paquete *paquete_mov_out = crear_paquete_con_codigo_de_operacion(PEDIDO_MOV_OUT);
    serializar_datos_mov_out(paquete_mov_out, Lista_direccionesFisica, valorObtenido, pid);
    enviar_paquete(paquete_mov_out, socket);
    eliminar_paquete(paquete_mov_out);
}

void serializar_datos_mov_out(t_paquete *paquete, t_list *Lista_direccionesFisica, uint32_t valorObtenido, uint32_t pid)
{
    paquete->buffer->size = list_size(Lista_direccionesFisica) * 2 * sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint32_t);
    // Reservamos la memoria para el buffer
    paquete->buffer->stream = malloc(paquete->buffer->size);

    int desplazamiento = 0;

    // Iteramos sobre la lista de datos y serializamos cada elemento
    for (int i = 0; i < list_size(Lista_direccionesFisica); i++)
    {
        t_direcciones_fisicas *dato = list_get(Lista_direccionesFisica, i);
        memcpy(paquete->buffer->stream + desplazamiento, &(dato->direccion_fisica), sizeof(uint32_t));
        desplazamiento += sizeof(uint32_t);
        memcpy(paquete->buffer->stream + desplazamiento, &(dato->tamanio), sizeof(uint32_t));
        desplazamiento += sizeof(uint32_t);
    }

    memcpy(paquete->buffer->stream + desplazamiento, &valorObtenido, sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);

    memcpy(paquete->buffer->stream + desplazamiento, &pid, sizeof(uint32_t));
}

void enviar_datos_copy_string(t_list *Lista_direccionesFisica_escritura, t_list *Lista_direccionesFisica_lectura, uint32_t tamanio, int socket, uint32_t pid)
{
    t_paquete *paquete_copy_string = crear_paquete_con_codigo_de_operacion(PEDIDO_COPY_STRING);
    serializar_datos_copy_string(paquete_copy_string, Lista_direccionesFisica_escritura, Lista_direccionesFisica_lectura, tamanio, pid);
    enviar_paquete(paquete_copy_string, socket);
    eliminar_paquete(paquete_copy_string);
}

void serializar_datos_copy_string(t_paquete *paquete, t_list *Lista_direccionesFisica_escritura, t_list *Lista_direccionesFisica_lectura, uint32_t tamanio, uint32_t pid)
{
    // Calculamos el tamaño total del buffer necesario
    paquete->buffer->size = list_size(Lista_direccionesFisica_escritura) * sizeof(t_direcciones_fisicas) + list_size(Lista_direccionesFisica_lectura) * sizeof(t_direcciones_fisicas) + sizeof(uint32_t) + sizeof(uint32_t);
    // printf("Tamanio del buffer: %d \n", paquete->buffer->size);
    //  Reservamos la memoria para el buffer

    paquete->buffer->stream = malloc(paquete->buffer->size);

    int desplazamiento = 0;

    // Iteramos sobre la lista de datos y serializamos cada elemento
    for (int i = 0; i < list_size(Lista_direccionesFisica_escritura); i++)
    {
        t_direcciones_fisicas *dato = list_get(Lista_direccionesFisica_escritura, i);
        memcpy(paquete->buffer->stream + desplazamiento, &(dato->direccion_fisica), sizeof(uint32_t));
        desplazamiento += sizeof(uint32_t);
        memcpy(paquete->buffer->stream + desplazamiento, &(dato->tamanio), sizeof(uint32_t));
        desplazamiento += sizeof(uint32_t);
    }

    for (int i = 0; i < list_size(Lista_direccionesFisica_lectura); i++)
    {
        t_direcciones_fisicas *dato = list_get(Lista_direccionesFisica_lectura, i);
        memcpy(paquete->buffer->stream + desplazamiento, &(dato->direccion_fisica), sizeof(uint32_t));
        desplazamiento += sizeof(uint32_t);
        memcpy(paquete->buffer->stream + desplazamiento, &(dato->tamanio), sizeof(uint32_t));
        desplazamiento += sizeof(uint32_t);
    }

    memcpy(paquete->buffer->stream + desplazamiento, &tamanio, sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);

    memcpy(paquete->buffer->stream + desplazamiento, &pid, sizeof(uint32_t));
}

t_list *recibir_dato_movIN(int socket_cliente)
{
    t_paquete *paquete = recibir_paquete(socket_cliente);
    t_list *datoObtenido = deserializar_dato_movIN(paquete);
    eliminar_paquete(paquete);
    return datoObtenido;
}

t_list *deserializar_dato_movIN(t_paquete *paquete)
{
    t_list *lista_deserializada = list_create();

    int desplazamiento = 0;
    uint32_t tamanioLista;

    // Copiar el tamaño de la lista desde el stream
    memcpy(&tamanioLista, paquete->buffer->stream + desplazamiento, sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);

    // Iterar sobre cada elemento serializado en el stream
    for (int i = 0; i < tamanioLista; i++)
    {
        // Leer la longitud de la cadena
        uint32_t longitud;
        memcpy(&longitud, paquete->buffer->stream + desplazamiento, sizeof(uint32_t));
        desplazamiento += sizeof(uint32_t);

        // Leer la cadena en sí
        char *dato = malloc(longitud); // Reservar memoria para la cadena
        memcpy(dato, paquete->buffer->stream + desplazamiento, longitud);
        desplazamiento += longitud;

        // Agregar el dato deserializado a la lista
        list_add(lista_deserializada, dato);
    }

    return lista_deserializada;
}

t_list *recibir_dato_de_memoria_movIn(int socket, t_log *logger)
{
    op_cod cop;
    t_list *datoRecibido = list_create();

    recv(socket, &cop, sizeof(op_cod), 0);
    switch (cop)
    {
    case RESPUESTA_DATO_MOVIN:
        datoRecibido = recibir_dato_movIN(socket);
        break;

    default:
        log_error(logger, "No se pudo recibir el dato");
        break;
    }

    if (datoRecibido == NULL)
    {
        log_error(logger, "No se leyo nada de memoria");
        return NULL;
    }

    return datoRecibido;
}

// UTILS
uint32_t *get_registry32(char *registro)
{
    if (strcmp(registro, "EAX") == 0)
        return &(pcb_actual->contexto_ejecucion->registros->eax);
    else if (strcmp(registro, "EBX") == 0)
        return &(pcb_actual->contexto_ejecucion->registros->ebx);
    else if (strcmp(registro, "ECX") == 0)
        return &(pcb_actual->contexto_ejecucion->registros->ecx);
    else if (strcmp(registro, "EDX") == 0)
        return &(pcb_actual->contexto_ejecucion->registros->edx);
    else if (strcmp(registro, "DI") == 0)
        return &(pcb_actual->contexto_ejecucion->registros->di);
    else if (strcmp(registro, "SI") == 0)
        return &(pcb_actual->contexto_ejecucion->registros->si);
    else if (strcmp(registro, "PC") == 0)
        return &(pcb_actual->contexto_ejecucion->registros->program_counter);
    else
    {
        log_error(LOGGER_CPU, "No se reconoce el registro %s", registro);
        return NULL;
    }
}

uint8_t *get_registry8(char *registro)
{
    if (strcmp(registro, "AX") == 0)
        return &(pcb_actual->contexto_ejecucion->registros->ax);
    else if (strcmp(registro, "BX") == 0)
        return &(pcb_actual->contexto_ejecucion->registros->bx);
    else if (strcmp(registro, "CX") == 0)
        return &(pcb_actual->contexto_ejecucion->registros->cx);
    else if (strcmp(registro, "DX") == 0)
        return &(pcb_actual->contexto_ejecucion->registros->dx);
    else
    {
        log_error(LOGGER_CPU, "No se reconoce el registro %s", registro);
        return NULL;
    }
}

char *instruccion_to_string(nombre_instruccion nombre)
{
    switch (nombre)
    {
    case SET:
        return "SET";
    case SUM:
        return "SUM";
    case SUB:
        return "SUB";
    case JNZ:
        return "JNZ";
    case RESIZE:
        return "RESIZE";
    case COPY_STRING:
        return "COPY_STRING";
    case MOV_IN:
        return "MOV_IN";
    case MOV_OUT:
        return "MOV_OUT";
    case WAIT:
        return "WAIT";
    case SIGNAL:
        return "SIGNAL";
    case IO_GEN_SLEEP:
        return "IO_GEN_SLEEP";
    case IO_STDIN_READ:
        return "IO_STDIN_READ";
    case IO_STDOUT_WRITE:
        return "IO_STDOUT_WRITE";
    case IO_FS_CREATE:
        return "IO_FS_CREATE";
    case IO_FS_DELETE:
        return "IO_FS_DELETE";
    case IO_FS_TRUNCATE:
        return "IO_FS_TRUNCATE";
    case IO_FS_WRITE:
        return "IO_FS_WRITE";
    case IO_FS_READ:
        return "IO_FS_READ";
    case EXIT:
        return "EXIT";
    default:
        return "DESCONOCIDA";
    }
}

bool revisar_registro(char *registro)
{
    if (strcmp(registro, "AX") == 0)
        return true;
    else if (strcmp(registro, "BX") == 0)
        return true;
    else if (strcmp(registro, "CX") == 0)
        return true;
    else if (strcmp(registro, "DX") == 0)
        return true;
    else if (strcmp(registro, "EAX") == 0)
        return false;
    else if (strcmp(registro, "EBX") == 0)
        return false;
    else if (strcmp(registro, "ECX") == 0)
        return false;
    else if (strcmp(registro, "EDX") == 0)
        return false;
    else if (strcmp(registro, "DI") == 0)
        return false;
    else if (strcmp(registro, "SI") == 0)
        return false;
    else if (strcmp(registro, "PC") == 0)
        return false;
    else
    {
        log_error(LOGGER_CPU, "No se reconoce el registro %s", registro);
        return false;
    }
}

uint32_t obtener_tamanio_registro(char *registro)
{
    if (strcmp(registro, "AX") == 0)
        return 1;
    else if (strcmp(registro, "BX") == 0)
        return 1;
    else if (strcmp(registro, "CX") == 0)
        return 1;
    else if (strcmp(registro, "DX") == 0)
        return 1;
    else if (strcmp(registro, "EAX") == 0)
        return 4;
    else if (strcmp(registro, "EBX") == 0)
        return 4;
    else if (strcmp(registro, "ECX") == 0)
        return 4;
    else if (strcmp(registro, "EDX") == 0)
        return 4;
    else
    {
        log_error(LOGGER_CPU, "No se reconoce el registro %s", registro);
        return -1;
    }
}