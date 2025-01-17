#include "../include/tlb.h"

t_tlb *tlb;

// Inicializar TLB
t_tlb *inicializar_tlb()
{
    tlb = (t_tlb *)malloc(sizeof(t_tlb));
    tlb->entradas = (entrada_tlb *)malloc(sizeof(entrada_tlb) * CANTIDAD_ENTRADAS_TLB);
    tlb->size_tlb = CANTIDAD_ENTRADAS_TLB;
    tlb->size_actual_tlb = 0; // Inicialmente la TLB esta vacia
    if (strcmp(ALGORITMO_TLB, "FIFO") == 0)
    {
        tlb->algoritmo = FIFO;
    }
    else
    {
        tlb->algoritmo = LRU;
    }
    return tlb;
}

// Destruir TLB
void destruir_tlb()
{
    free(tlb->entradas);
    free(tlb);
}

// Busqueda en la TLB
uint32_t buscar_en_tlb(uint32_t pid, uint32_t pagina)
{
    time_t tiempo_actual = time(NULL);
    for (int i = 0; i < tlb->size_actual_tlb; i++)
    {
        if (tlb->entradas[i].pid == pid && tlb->entradas[i].pagina == pagina)
        {
            tlb->entradas[i].tiempo_lru = (uint32_t)tiempo_actual;
            return tlb->entradas[i].marco; // TLB-HIT
        }
    }
    return -1; // TLB-MISS
}

// Reemplazo por FIFO
void reemplazo_algoritmo_FIFO(uint32_t pid, uint32_t pagina, uint32_t marco)
{
    // printf("Reemplazando página %u y marco %u del proceso %u con página %u y marco %u del proceso %u\n", tlb->entradas[0].pagina, tlb->entradas[0].marco, tlb->entradas[0].pid, pagina, marco, pid);

    for (int i = 1; i < tlb->size_actual_tlb; i++)
    {
        tlb->entradas[i - 1] = tlb->entradas[i];
    }

    tlb->entradas[tlb->size_actual_tlb - 1].pid = pid;
    tlb->entradas[tlb->size_actual_tlb - 1].pagina = pagina;
    tlb->entradas[tlb->size_actual_tlb - 1].marco = marco;
}

// Reemplazo por LRU
void reemplazo_algoritmo_LRU(uint32_t pid, uint32_t pagina, uint32_t marco)
{
    int lruIndex = 0;
    time_t tiempo_actual = time(NULL);
    for (int i = 1; i < tlb->size_actual_tlb; i++)
    {
        if (tlb->entradas[i].tiempo_lru < tlb->entradas[lruIndex].tiempo_lru)
        {
            lruIndex = i;
        }
    }
    // printf("Reemplazando página %u y marco %u del proceso %u con página %u y marco %u del proceso %u\n", tlb->entradas[lruIndex].pagina, tlb->entradas[lruIndex].marco, tlb->entradas[lruIndex].pid, pagina, marco, pid);
    tlb->entradas[lruIndex].pid = pid;
    tlb->entradas[lruIndex].pagina = pagina;
    tlb->entradas[lruIndex].marco = marco;
    tlb->entradas[lruIndex].tiempo_lru = tiempo_actual;
}

// Actualizar TLB
void actualizar_TLB(uint32_t pid, uint32_t pagina, uint32_t marco)
{
    time_t tiempo_actual = time(NULL);
    if (tlb->size_actual_tlb < tlb->size_tlb)
    {
        tlb->entradas[tlb->size_actual_tlb].pid = pid;
        tlb->entradas[tlb->size_actual_tlb].pagina = pagina;
        tlb->entradas[tlb->size_actual_tlb].marco = marco;
        if (tlb->algoritmo == LRU)
        {
            tlb->entradas[tlb->size_actual_tlb].tiempo_lru = (uint32_t)tiempo_actual;
        }
        tlb->size_actual_tlb++;
    }
    else
    {
        if (tlb->algoritmo == FIFO)
        {
            reemplazo_algoritmo_FIFO(pid, pagina, marco);
        }
        else
        {
            reemplazo_algoritmo_LRU(pid, pagina, marco);
        }
    }
}

t_list *traducir_direccion(uint32_t pid, uint32_t logicalAddress, uint32_t pageSize, uint32_t tamanio_registro)
{
    // printf("El número de direccion logica es: %d\n", logicalAddress);
    t_list *listaDirecciones = list_create();
    t_direcciones_fisicas *direccion = malloc(sizeof(t_direcciones_fisicas));
    uint32_t pagina = logicalAddress / pageSize;
    uint32_t offset = logicalAddress - pagina * pageSize;
    uint32_t tamanioAleer = 0;
    uint32_t tamanioAleer2 = 0;
    bool ocupaMasDeUnaPagina = false;

    uint32_t cantidadPaginas = obtenerCantidadPaginas(logicalAddress, pageSize, tamanio_registro);

    // printf("Cantidad de paginas: %d\n", cantidadPaginas);

    if (cantidadPaginas > 1)
    {
        ocupaMasDeUnaPagina = true;
    }
    uint32_t marco = 0;
    if (CANTIDAD_ENTRADAS_TLB > 0)
    {
        marco = buscar_en_tlb(pid, pagina);
    }
    if (marco != -1 && CANTIDAD_ENTRADAS_TLB != 0)
    {
        // TLB Hit
        log_info(LOGGER_CPU, "PID: %d - TLB HIT - Pagina: %d", pid, pagina);
        direccion->direccion_fisica = marco * pageSize + offset;
        log_info(LOGGER_CPU, "PID: %d - OBTENER MARCO - Pagina: %d - MARCO: %d", pid, pagina, marco);

        for (int i = 0; i < tamanio_registro; i++)
        {
            if (direccion->direccion_fisica / pageSize == (direccion->direccion_fisica + i) / pageSize)
            {
                tamanioAleer++;
            }
        }
        direccion->tamanio = tamanioAleer;
        // printf("direccion: %d\n", direccion->direccion_fisica);
        // printf("tamanio direccion: %d\n", direccion->tamanio);
        list_add(listaDirecciones, direccion);
    }
    else
    {
        // TLB Miss
        // printf("TLB Miss\n");
        // pido marco a memoria enviando la pagina y el pid y me devuelve un marco
        enviar_Pid_Pagina_Memoria(pid, pagina);
        marco = recibir_marco_memoria(fd_cpu_memoria);
        log_info(LOGGER_CPU, "PID: %d - TLB MISS - Pagina: %d", pid, pagina);
        log_info(LOGGER_CPU, "PID: %d - OBTENER MARCO - Pagina: %d - MARCO: %d", pid, pagina, marco);
        if (CANTIDAD_ENTRADAS_TLB > 0)
        {
            actualizar_TLB(pid, pagina, marco);
        }
        direccion->direccion_fisica = marco * pageSize + offset;

        int i = 0;
        while (direccion->direccion_fisica / pageSize == (direccion->direccion_fisica + i) / pageSize && i < tamanio_registro)
        {
            tamanioAleer++;
            i++;
        }
        direccion->tamanio = tamanioAleer;
        list_add(listaDirecciones, direccion);
        // printf("direccion: %d\n", direccion->direccion_fisica);
        // printf("tamanio direccion: %d\n", direccion->tamanio);
        cantidadPaginas++;
    }

    uint32_t paginaSig;
    uint32_t offset2;
    uint32_t marco2;
    int contador = 0;
    int contadorBreak = 0;
    uint32_t tamanioAux = tamanio_registro - direccion->tamanio;
    uint32_t tamanioAuxTotal = direccion->tamanio;
    if (ocupaMasDeUnaPagina)
    {
        while (contador <= cantidadPaginas)
        {
            t_direcciones_fisicas direccionEsdiguientes;

            for (int i = 0; i < tamanio_registro; i++)
            {
                if ((logicalAddress / pageSize != (logicalAddress + i) / pageSize) && contadorBreak == 0 && tamanioAuxTotal != tamanio_registro)
                {
                    paginaSig = (logicalAddress + tamanioAuxTotal) / pageSize;
                    offset2 = (logicalAddress + tamanioAuxTotal) - paginaSig * pageSize;
                    marco2 = 0;
                    if (CANTIDAD_ENTRADAS_TLB > 0)
                    {
                        marco2 = buscar_en_tlb(pid, paginaSig);
                    }

                    if (marco2 != -1 && CANTIDAD_ENTRADAS_TLB > 0)
                    {
                        // TLB Hit
                        log_info(LOGGER_CPU, "PID: %d - TLB HIT - Pagina: %d", pid, paginaSig);
                        direccionEsdiguientes.direccion_fisica = marco2 * pageSize + offset2;
                        log_info(LOGGER_CPU, "PID: %d - OBTENER MARCO - Pagina: %d - MARCO: %d", pid, paginaSig, marco2);
                        for (int j = 0; j < tamanioAux; j++)
                        {
                            if (direccionEsdiguientes.direccion_fisica / pageSize == (direccionEsdiguientes.direccion_fisica + j) / pageSize)
                            {
                                tamanioAleer2++;
                            }
                        }
                        direccionEsdiguientes.tamanio = tamanioAleer2;
                        tamanioAux = tamanioAux - direccionEsdiguientes.tamanio;
                        
                        t_direcciones_fisicas *copia = malloc(sizeof(t_direcciones_fisicas));
                        copia->direccion_fisica = direccionEsdiguientes.direccion_fisica;
                        copia->tamanio = direccionEsdiguientes.tamanio;
                        list_add(listaDirecciones, copia);

                        contadorBreak++;
                    }
                    else
                    {
                        // TLB Miss
                        log_info(LOGGER_CPU, "PID: %d - TLB MISS - Pagina: %d", pid, paginaSig);
                        // pido marco a memoria enviando la pagina y el pid y me devuelve un marco
                        enviar_Pid_Pagina_Memoria(pid, paginaSig);
                        marco2 = recibir_marco_memoria(fd_cpu_memoria);
                        log_info(LOGGER_CPU, "PID: %d - OBTENER MARCO - Página: %d - MARCO: %d", pid, paginaSig, marco2);

                        if (CANTIDAD_ENTRADAS_TLB > 0)
                        {
                            actualizar_TLB(pid, paginaSig, marco2);
                        }
                        direccionEsdiguientes.direccion_fisica = marco2 * pageSize + offset2;
                        for (int j = 0; j < tamanioAux; j++)
                        {
                            if (direccionEsdiguientes.direccion_fisica / pageSize == (direccionEsdiguientes.direccion_fisica + j) / pageSize)
                            {
                                tamanioAleer2++;
                            }
                        }

                        direccionEsdiguientes.tamanio = tamanioAleer2;
                        tamanioAux = tamanioAux - direccionEsdiguientes.tamanio;

                        t_direcciones_fisicas *copia = malloc(sizeof(t_direcciones_fisicas));
                        copia->direccion_fisica = direccionEsdiguientes.direccion_fisica;
                        copia->tamanio = direccionEsdiguientes.tamanio;
                        list_add(listaDirecciones, copia);

                        contadorBreak++;
                    }
                    tamanioAuxTotal = tamanioAuxTotal + tamanioAleer2;
                }
            }
            tamanioAleer2 = 0;
            contadorBreak = 0;
            contador++;
        }
    }

    // ESTO ES PARA MOSTRAR LAS DIRECCIONES
    /*
    for (int i = 0; i < list_size(listaDirecciones); i++)
    {
        t_direcciones_fisicas *direccionAmostrar = list_get(listaDirecciones, i);
        // printf("Direccion Fisica: %d\n", direccionAmostrar->direccion_fisica);
        // printf("Tamanio: %d\n", direccionAmostrar->tamanio);
    }*/

    return listaDirecciones;
}

uint32_t obtenerCantidadPaginas(uint32_t logicalAddress, uint32_t pageSize, uint32_t tamanio_registro)
{
    int index = 0;
    int auxTamanio = tamanio_registro;
    uint32_t logicalAddressAux = logicalAddress;
    int cantidadPaginas = 1;

    while (index < auxTamanio)
    {
        if (logicalAddressAux / pageSize != (logicalAddressAux + index) / pageSize)
        {
            logicalAddressAux = logicalAddressAux + index;
            auxTamanio = auxTamanio - index;
            index = 0;
            cantidadPaginas++;
        }
        else
        {
            index++;
        }
    }

    return cantidadPaginas;
}

void enviar_Pid_Pagina_Memoria(uint32_t pid_proceso, uint32_t pagina_nueva)
{
    t_paquete *paquete_nueva_pagina = crear_paquete_con_codigo_de_operacion(PEDIDO_MARCO);
    serializar_nueva_pagina(paquete_nueva_pagina, pid_proceso, pagina_nueva);
    enviar_paquete(paquete_nueva_pagina, fd_cpu_memoria);
    eliminar_paquete(paquete_nueva_pagina);
}

void serializar_nueva_pagina(t_paquete *paquete, uint32_t pid_proceso, uint32_t pagina_nueva)
{
    int buffer_size = sizeof(uint32_t) + sizeof(uint32_t);
    void *stream = malloc(buffer_size);
    if (stream == NULL)
    {
        return;
    }

    int offset = 0;
    memcpy(stream + offset, &pid_proceso, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    memcpy(stream + offset, &pagina_nueva, sizeof(uint32_t));

    paquete->buffer->size = buffer_size;
    paquete->buffer->stream = stream;
}

/*uint32_t obtener_valor_direccion_fisica(uint32_t direccion_fisica)
{

    if (direccion_fisica == -1)
    {
        return direccion_fisica;
    }

    enviar_direccion_fisica_memoria(direccion_fisica);

    uint32_t resultado = recibir_direccion_fisica(fd_cpu_memoria);


   //char valor="AX";

   // log_info(cpu_logger_info, "PID: %d - Acción: LEER - Dirección Física: %d - Valor: %d", contexto_actual->pid, df, valor);

    return resultado;
}*/
/*
uint32_t recibir_direccion_fisica(int socket_cliente)
{
    t_paquete *paquete = recibir_paquete(socket_cliente);
    // Creo que "deserializar_direccion_fisica()" no existe. SI EXISTEN
    uint32_t valor_direccion_fisica = deserializar_direccion_fisica(paquete->buffer, direccion_fisica); //????? Estan tratando de obtener el valor de la df con la df??
    eliminar_paquete(paquete);
    return valor_direccion_fisica;
}

void enviar_direccion_fisica_memoria(uint32_t direccion_fisica)
{
    t_paquete *paquete_direccion_fisica = crear_paquete_con_codigo_de_operacion(ENVIAR_DIRECCION_FISICA);
    // Creeria que la funcion "serializar_direccion_fisica()" no existe.   SI EXISTEN
    serializar_direccion_fisica(paquete_direccion_fisica, direccion_fisica);
    enviar_paquete(paquete_direccion_fisica, fd_cpu_memoria);
    eliminar_paquete(paquete_direccion_fisica);
}*/

// Esto no podria usarlo en el comuniaciones.c de CPU??? Al haber TLB-miss le avisa a memoria y esta se la devuelve a CPU (Se me ocurre teorizando en el momento)
uint32_t recibir_marco_memoria(int fd_cpu_memoria)
{
    op_cod cop;
    uint32_t marcoRecibido = -1;

    if(recv(fd_cpu_memoria, &cop, sizeof(op_cod), 0)!= sizeof(op_cod)){
        exit(EXIT_FAILURE);
    }

    
    switch (cop)
    {
    case ENVIAR_MARCO:
        marcoRecibido = recibir_marco(fd_cpu_memoria);
        break;

    default:
        log_error(LOGGER_CPU, "No se pudo recibir el marco");
        break;
    }

    if (marcoRecibido < 0)
    {
        log_error(LOGGER_CPU, "Error al recibir MARCO de MEMORIA");
        return -1;
    }

    return marcoRecibido;
}

uint32_t recibir_marco(int socket_cliente)
{
    t_paquete *paquete = recibir_paquete(socket_cliente);
    uint32_t marco = deserializar_marco(paquete->buffer);
    eliminar_paquete(paquete);
    return marco;
}

uint32_t deserializar_marco(t_buffer *buffer)
{
    uint32_t marco;

    void *stream = buffer->stream;

    memcpy(&(marco), stream, sizeof(uint32_t));

    // printf("Marco deserializado: %d\n", marco);
    return marco;
}
