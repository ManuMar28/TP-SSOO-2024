#ifndef IO_H_
#define IO_H_

#include "./gestor.h"
#include "./pcb.h"
#include "./memoria.h"
typedef enum
{
    GENERICA,
    STDIN,
    STDOUT,
    DIALFS
} t_tipo_interfaz;

typedef struct
{
    char *nombre_interfaz;
    uint32_t tamanio_nombre_interfaz;
    t_tipo_interfaz tipo_interfaz;
} t_interfaz;

typedef struct
{
    int unidades_de_trabajo;
    uint32_t pidPcb;
    char *nombre_interfaz;
} t_interfaz_gen;

typedef struct
{
    uint32_t pidPcb;
    t_list *direccionesFisicas;
    char *nombre_interfaz;
} t_interfaz_stdin;

typedef struct
{
    uint32_t pidPcb;
    t_list *direccionesFisicas;
    char *nombre_interfaz;
} t_interfaz_stdout;

typedef struct
{
    int unidades_de_trabajo;
    uint32_t tamanio_nombre_interfaz;
} t_interfaz_DIALFS;

// generica
void enviar_interfaz_IO(t_pcb *pcb_actual, char *interfaz, int unidades_de_trabajo, int socket_cliente, nombre_instruccion instruccion);
void serializar_IO_instruccion(t_paquete *paquete, t_pcb *pcb_actual, int unidades_de_trabajo, char *interfaz, nombre_instruccion instruccion);
void recibir_pcb_para_interfaz(t_pcb *pcb, int socket_cliente, char **nombre_interfaz, int *unidades_de_trabajo, nombre_instruccion *instruccion);
void deserializar_pcb_para_interfaz(t_pcb *pcb, t_buffer *buffer, char **nombre_interfaz, int *unidades_de_trabajo, nombre_instruccion *instruccion);
void enviar_datos_interfaz(t_interfaz *interfaz, int socket_server, op_cod codigo_operacion);
t_interfaz *recibir_datos_interfaz(int socket_cliente);
t_paquete *crear_paquete_interfaz(t_interfaz *interfaz, op_cod codigo_operacion);
t_buffer *crear_buffer_interfaz(t_interfaz *interfaz);
t_interfaz *deserializar_interfaz_recibida(t_buffer *buffer);
void enviar_InterfazGenerica(int socket, int unidades_trabajo, uint32_t pid, char *nombre_interfaz);
t_paquete *crear_paquete_InterfazGenerica(t_interfaz_gen *interfaz);
t_buffer *crear_buffer_InterfazGenerica(t_interfaz_gen *interfaz);
t_interfaz_gen *deserializar_InterfazGenerica(t_buffer *buffer);
t_interfaz_gen *recibir_InterfazGenerica(int socket);
void enviar_InterfazGenericaConCodigoOP(int socket, int unidades_trabajo, uint32_t pid, char *nombre_interfaz);
t_paquete *crear_paquete_InterfazGenericaCodOp(t_interfaz_gen *interfaz, op_cod codigo_operacion);

// stdin
void enviar_interfaz_IO_stdin(t_pcb *pcb_actual, char *interfaz, t_list *Lista_direccionesFisica , int socket_cliente, nombre_instruccion IO);
void serializar_IO_instruccion_stdin(t_paquete *paquete, t_pcb *pcb, char *interfaz, t_list *Lista_direccionesFisica, nombre_instruccion IO);
void recibir_pcb_para_interfaz_stdin(t_pcb *pcb, int socket_cliente, char **nombre_interfaz, t_list *direcciones_fisicas , nombre_instruccion *IO);
void deserializar_pcb_para_interfaz_stdin(t_pcb *pcb, t_buffer *buffer, char **nombre_interfaz, t_list *direcciones_fisicas, nombre_instruccion *IO);
t_paquete *crear_paquete_InterfazstdinCodOp(t_interfaz_stdin *interfaz, op_cod codigo_operacion);
t_buffer *crear_buffer_InterfazStdin(t_interfaz_stdin *interfaz);
void enviar_InterfazStdinConCodigoOPaKernel(int socket, t_list *direcciones_fisicas, uint32_t pid, char *nombre_interfaz);
t_interfaz_stdin *recibir_InterfazStdin(int socket);
t_interfaz_stdin *deserializar_InterfazStdin(t_buffer *buffer);
void enviar_dato_stdin(int socket, uint32_t direccionFisica, char *datoRecibido);
t_paquete *crear_paquete_dato_stdin(uint32_t direccionFisica, char *datoRecibido, op_cod codigo_operacion);
t_buffer *crear_buffer_dato_stdin(uint32_t direccionFisica, char *datoRecibido);
void serializarInterfazStdin_de_Kernale_a_Memoria(t_paquete *paquete, t_interfaz_stdin *interfaz);
void enviar_InterfazStdin(int socket, t_list *direcciones_fisicas, uint32_t pid, char *nombre_interfaz);


// stdout

void enviar_InterfazStdout(int socket, t_list *direcciones_fisicas, uint32_t pid, char *nombre_interfaz);
t_interfaz_stdout *recibir_InterfazStdout(int socket_cliente);
t_interfaz_stdout *deserializar_InterfazStdout(t_buffer *buffer);
t_paquete *crear_paquete_InterfazstdoutCodOp(t_interfaz_stdout *interfaz, op_cod codigo_operacion);
t_buffer *crear_buffer_InterfazStdout(t_interfaz_stdout *interfaz);
void enviar_interfaz_IO_stdout(t_pcb *pcb_actual, char *interfaz, t_list *Lista_direccionesFisica, int socket_cliente, nombre_instruccion IO);
void serializarInterfazStdout_de_Kernale_a_Memoria(t_paquete *paquete, t_interfaz_stdout *interfaz);
void enviar_InterfazStdoutConCodigoOPaKernel(int socket, t_list *direcciones_fisicas, uint32_t pid, char *nombre_interfaz);


#endif // IO_H_