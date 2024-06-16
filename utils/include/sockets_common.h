#ifndef SOCKETS_COMMON_H_
#define SOCKETS_COMMON_H_
typedef enum
{
    HANDSHAKE_consola,
    HANDSHAKE_kernel,
    HANDSHAKE_memoria,
    HANDSHAKE_cpu,
    HANDSHAKE_interrupt,
    HANDSHAKE_dispatch,
    HANDSHAKE_in_out,
    HANDSHAKE_ok_continue,
    ERROROPCODE,
    MENSAJE,
    PAQUETE,
    PCB,
    INICIALIZAR_PROCESO,
    FINALIZAR_PROCESO,
    INTERRUPCION,
    CONTEXTO,
    PEDIDO_RESIZE,
    INSTRUCCION,
    PEDIDO_INSTRUCCION,
    PEDIDO_WAIT,
    PEDIDO_SIGNAL,
    ENVIAR_INTERFAZ,
    CONEXION_INTERFAZ,
    DESCONEXION_INTERFAZ,
    FINALIZACION_INTERFAZ,
    PEDIDO_IO_GEN_SLEEP,
    PEDIDO_IO_STDIN_READ,
    PEDIDO_IO_STDOUT_WRITE,
    PEDIDO_IO_FS_CREATE,
    PEDIDO_IO_FS_DELETE,
    PEDIDO_IO_FS_TRUNCATE,
    PEDIDO_IO_FS_WRITE,
    PEDIDO_IO_FS_READ,
    ENVIAR_PAGINA,
    ENVIAR_DIRECCION_FISICA,
    PEDIDO_MOV_IN,
    PEDIDO_MOV_OUT,
    PEDIDO_MARCO,
    ENVIAR_MARCO,
    ENVIAR_INTERFAZ_STDIN,
    ENVIAR_INTERFAZ_STDOUT,
    RECIBIR_DATO_STDIN,
    FINALIZACION_INTERFAZ_STDIN,
    FINALIZACION_INTERFAZ_STDOUT,
    PEDIDO_ESCRIBIR_DATO_STDIN,
    PEDIDO_A_LEER_DATO_STDOUT,
    RESPUESTA_STDIN,
    RESPUESTA_DATO_STDOUT,
    PEDIDO_COPY_STRING,
    RESPUESTA_DATO_MOVIN,
    MISMO_TAMANIO,
    RESIZE_OK
} op_cod;

#endif // SOCKETS_COMMON_H_