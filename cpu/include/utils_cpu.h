#ifndef UTILS_CPU_H_
#define UTILS_CPU_H_

#include <stdlib.h>
#include <stdio.h>

#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>
#include <readline/readline.h>

#include "../../utils/include/sockets_server.h"
#include "../../utils/include/sockets_client.h"
#include "../../utils/include/sockets_utils.h"
#include "../../utils/include/sockets_common.h"
#include "../../utils/include/pcb.h"
#include "../../utils/include/contexto.h"
#include "./comunicaciones.h"

void ejecutar_ciclo_instruccion();
t_instruccion *fetch(int pid, int pc);
void execute(t_instruccion *instruccion);
void pedir_instruccion_memoria(int pid, int pc, int socket);
t_instruccion *deserializar_instruccion(int socket);

#endif