#ifndef MAIN_H_
#define MAIN_H_

#include <stdlib.h>
#include <stdio.h>

#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>
#include <readline/readline.h>

#include "../../utils/include/hello.h"
#include "../../utils/include/sockets_server.h"
#include "../../utils/include/sockets_client.h"
#include "../../utils/include/sockets_utils.h"
#include "../../utils/include/sockets_common.h"
#include "../../utils/include/pcb.h"
#include "./comunicaciones.h"

void inicializar_config(void);
void iniciar_conexiones();
void escuchar_interrupt();

#endif