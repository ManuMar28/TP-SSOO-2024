#ifndef COMUNICACIONES_H_
#define COMUNICACIONES_H_

#include <stdlib.h>
#include <stdio.h>

#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>
#include <readline/readline.h>

#include "../../utils/include/hello.h"
#include "../../utils/include/sockets_client.h"
#include "../../utils/include/sockets_server.h"

int procesar_conexion(int server_fd);
void iterator(char* value);

#endif