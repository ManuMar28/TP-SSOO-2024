#ifndef SOCKETS_SERVER_H_
#define SOCKETS_SERVER_H_

#include "./gestor.h"
#include "./sockets_client.h"
#include "./sockets_common.h"
#include "./contexto.h"

void *recibir_buffer(int *, int);
int iniciar_servidor(t_log *logger, const char *name, char *ip, char *puerto);
int esperar_cliente(int, t_log *logger);
void iterator(char *value);
t_paquete *recibir_paquete(int socket_cliente);
t_interrupcion *recibir_interrupcion(int);
t_interrupcion *deserializar_interrupcion(t_buffer *buffer);
void recibir_mensaje(int, t_log *logger);
int recibir_operacion(int);

#endif /* SOCKETS_SERVER_H_ */