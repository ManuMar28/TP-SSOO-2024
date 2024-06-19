#include "../include/comunicaciones.h"

static void procesar_conexion_memoria(void *void_args)
{
	t_procesar_conexion_args *args = (t_procesar_conexion_args *)void_args;
	t_log *logger = args->log;
	int cliente_socket = args->fd;
	char *server_name = args->server_name;
	free(args);

	op_cod cop;
	while (cliente_socket != -1)
	{
		if (recv(cliente_socket, &cop, sizeof(op_cod), 0) != sizeof(op_cod))
		{
			log_info(logger, "Se desconecto el cliente!\n");
			return;
		}

		switch (cop)
		{
		case MENSAJE:
			recibir_mensaje(cliente_socket, logger);
			break;

		// ----------------------
		// -- KERNEL - MEMORIA --
		// ----------------------
		case HANDSHAKE_kernel:
			recibir_mensaje(cliente_socket, logger);
			log_info(logger, "Este deberia ser el canal mediante el cual nos comunicamos con el KERNEL");
			break;

		// Recibe PID y PATH del Kernel, y las guarda en conjunto con la lista de instrucciones en el struct PROCESO_MEMORIA
		case INICIALIZAR_PROCESO:
			proceso_memoria = recibir_proceso_memoria(cliente_socket);
			proceso_memoria = iniciar_proceso_path(proceso_memoria);
			break;

		case FINALIZAR_PROCESO:
			uint32_t pid_a_finalizar;
			recibir_finalizar_proceso(&pid_a_finalizar, cliente_socket);
			proceso_memoria = obtener_proceso_pid(pid_a_finalizar);
			if (proceso_memoria == NULL)
			{
				log_error(logger, "No se encontro el proceso con PID %d", pid_a_finalizar);
				break;
			}
			else
			{
				liberar_estructura_proceso_memoria(proceso_memoria);
				break;
			}

		// -------------------
		// -- CPU - MEMORIA --
		// -------------------
		case HANDSHAKE_cpu:
			recibir_mensaje(cliente_socket, logger);
			log_info(logger, "Este deberia ser el canal mediante el cual nos comunicamos con la CPU");
			break;

		case PEDIDO_INSTRUCCION:
			uint32_t pid, pc;
			recibir_pedido_instruccion(&pid, &pc, cliente_socket);
			// log_debug(logger, "Se recibio un pedido de instruccion para el PID %d y PC %d", pid, pc);
			proceso_memoria = obtener_proceso_pid(pid);
			if (proceso_memoria == NULL)
			{
				log_error(logger, "No se encontro el proceso con PID %d", pid);
				break;
			}
			else
			{
				t_instruccion *instruccion = obtener_instruccion_del_proceso_pc(proceso_memoria, pc);
				if (instruccion != NULL)
				{
					enviar_instruccion(cliente_socket, instruccion);
					// log_debug(logger, "Se envia la instruccion a CPU de PC %d para el PID %d y es: %s - %s - %s", pc, pid, instruccion_to_string(instruccion->nombre), instruccion->parametro1, instruccion->parametro2);
				}
				else
				{
					log_error(logger, "No se encontro la instruccion con PC %d para el PID %d", pc, pid);
				}
				break;
			}

		case PEDIDO_RESIZE:
			uint32_t pidResize, tamanio;
			recibir_pedido_resize(&pidResize, &tamanio, cliente_socket);
			log_debug(logger, "Se recibio un pedido de resize para el PID %d y %d tamanio", pidResize, tamanio);
			proceso_memoria = obtener_proceso_pid(pidResize);
			if (proceso_memoria == NULL)
			{
				log_error(logger, "No se encontro el proceso con PID %d", pidResize);
				break;
			}
			else
			{
				op_cod response = resize_proceso_memoria(proceso_memoria, tamanio);
				enviar_respuesta_resize(cliente_socket, response);
				break;
			}

		case PEDIDO_MOV_IN: // Lee el valor del marco y lo devuelve para guardarlo en el registro (se pide la direccion) - recibo direccion fisica
			t_list *direcciones_fisicas_mov_in = list_create();
			t_list *lista_datos_a_leer_mov_in = list_create();
			t_list *lista_datos_leidos_mov_in = list_create();


			char *valor_leido_mov_in;
			char *valorTotalaDeLeerMovIn;
			recibir_mov_in_cpu(cliente_socket, direcciones_fisicas_mov_in);
			int tamanioTotalstrlen = 0;
			int desplazamientoLeer;
			for (int i = 0; i < list_size(direcciones_fisicas_mov_in); i++)
			{
				t_direcciones_fisicas *direccionAmostrar = list_get(direcciones_fisicas_mov_in, i);
				desplazamientoLeer=0;
				for(int j = 0; j < direccionAmostrar->tamanio; j++){
					valor_leido_mov_in = leer_memoria(direccionAmostrar->direccion_fisica + desplazamientoLeer, direccionAmostrar->tamanio);
					printf("Valor leido de memoria en pedacitos: %s \n", valor_leido_mov_in);
					if(strcmp(valor_leido_mov_in, "0") != 0){
						list_add(lista_datos_a_leer_mov_in, strdup(valor_leido_mov_in));
					}
					tamanioTotalstrlen += strlen(valor_leido_mov_in);
					desplazamientoLeer++;
				}
				valorTotalaDeLeerMovIn = malloc(tamanioTotalstrlen + 1);
				memset(valorTotalaDeLeerMovIn, 0, tamanioTotalstrlen + 1);
				valorTotalaDeLeerMovIn = concatenar_lista_de_cadenas(lista_datos_a_leer_mov_in, tamanioTotalstrlen);
				printf("Valor leido de memoria: %s \n", valorTotalaDeLeerMovIn);
				list_add(lista_datos_leidos_mov_in, strdup(valorTotalaDeLeerMovIn));
				list_clean_and_destroy_elements(lista_datos_a_leer_mov_in, free);
			}
			/*for(int g = 0; g < list_size(lista_datos_leidos_mov_in); g++){
				char *aMostrar = list_get(lista_datos_leidos_mov_in, g);
				printf("Valor YA LEIDO de memoria: %s \n", aMostrar);

			}*/

			enviar_dato_movIn(cliente_socket, lista_datos_leidos_mov_in);

			list_clean_and_destroy_elements(direcciones_fisicas_mov_in, free);
			list_clean_and_destroy_elements(lista_datos_a_leer_mov_in, free);
			list_clean_and_destroy_elements(lista_datos_leidos_mov_in, free);

			// TODO devolver valor_leido_mov_in a cpu
			break;

		case PEDIDO_MOV_OUT: // me pasa por parametro un uint32_t y tengo que guardarlo en el marco que me dice
			t_list *direcciones_fisicas_mov_out = list_create();
			uint32_t valor_obtenido_mov_out;
			recibir_mov_out_cpu(direcciones_fisicas_mov_out, &valor_obtenido_mov_out, cliente_socket);
			int numero_recibido = (int)valor_obtenido_mov_out;
			printf("Numero en decimal: %d \n", numero_recibido);
			char* valor_mov_out_binario = decimal_a_binario(numero_recibido);
			printf("Numero en binario: %s \n", valor_mov_out_binario);

			int indice_valor_mov_out_binario = 0;
			int cantidadBits = 0;
			int indice = 0;
			for (int i = 0; i < list_size(direcciones_fisicas_mov_out); i++)
			{
				t_direcciones_fisicas *direccionAmostrar = list_get(direcciones_fisicas_mov_out, i);
				char* valor_parcial_binario = calloc(9, sizeof(char)); // Asegúrate de tener espacio para 8 caracteres y el carácter nulo
				char* valor_parcial_decimal_a_escribir = calloc(direccionAmostrar->tamanio + 1, sizeof(char)); // Asegúrate de tener espacio para direccionAmostrar->tamanio caracteres y el carácter nulo

				int desplazamiento = 0;
				int cantidad_bits_llenados = 0;
				for(int k = 0; k < (direccionAmostrar->tamanio) * 8; k++)
				{
					
					if(indice_valor_mov_out_binario <= strlen(valor_mov_out_binario)){
						if(valor_mov_out_binario[indice_valor_mov_out_binario])
						{
							valor_parcial_binario[k] = valor_mov_out_binario[indice_valor_mov_out_binario]; //Numero binario parcial a pasar
							printf("Bit pasado al valor parcial: %c \n", valor_parcial_binario[k]);
							cantidad_bits_llenados++;
							indice_valor_mov_out_binario++;
							cantidadBits++;
						}
						else
						{
							printf("Valor binario a parcial menor a 8 bits a transformar: %s \n", valor_parcial_binario);
							int decimal_a_mandar = binario_a_decimal(atoi(valor_parcial_binario));
							printf("Valor transformado del binario al decimal menor a 8 bits: %d \n", decimal_a_mandar);
							valor_parcial_decimal_a_escribir = int_to_char(decimal_a_mandar);
							//printf("Valor decimal en char a enviar menor a 8 bits: %s \n", valor_parcial_decimal_a_escribir);
							escribir_memoria_mov_out(direccionAmostrar->direccion_fisica + desplazamiento, 1, valor_parcial_decimal_a_escribir);
							indice_valor_mov_out_binario++;
							break;
						}
						if(cantidad_bits_llenados % 8 == 0 && valor_mov_out_binario[indice_valor_mov_out_binario])
						{
							printf("Valor binario a parcial a transformar: %s \n", valor_parcial_binario);    
							int decimal_a_mandar = binario_a_decimal(atoi(valor_parcial_binario));
							printf("Valor transformado del binario al decimal: %d \n", decimal_a_mandar);
							free(valor_parcial_decimal_a_escribir); // Libera la memoria antigua antes de asignar una nueva
							valor_parcial_decimal_a_escribir = int_to_char(decimal_a_mandar);
								//unsigned char hola = (unsigned char)valor_parcial_decimal_a_escribir;
							//printf("Valor decimal en char a enviar: %s \n", valor_parcial_decimal_a_escribir);
							escribir_memoria_mov_out(direccionAmostrar->direccion_fisica + desplazamiento, 1, valor_parcial_decimal_a_escribir);
							memset(valor_parcial_binario, 0, 9); // Asegúrate de limpiar todo el espacio que has asignado
							desplazamiento++;
							cantidad_bits_llenados = 0;
							
							k = -1;
						//indice_valor_mov_out_binario = 0;
						}
					}
					 
					
					
					

				}
				cantidadBits=0;
											free(valor_parcial_binario);
				//indice_valor_mov_out_binario = 0;
				indice = indice + direccionAmostrar->tamanio;
			} 
			


			/* int valor_mov_out_int = (int)valorObtenido_mov_out;
			log_debug(LOGGER_MEMORIA, "valor int: %d \n", valor_mov_out_int);
			char* valor_mov_out_char = (char)valor_mov_out_int;

			char *valor_entero_a_escribir = int_to_char(valor_leido); //Anda mal el int_to_char de los cojones
			int tamanioAescribir = strlen(valor_entero_a_escribir);*/
			/* char *valor_a_escribir = malloc(2);
			memset(valor_a_escribir, 0, 2);
			valor_a_escribir[0] = valor_mov_out_char;
			valor_a_escribir[1] = '\0';
			log_debug(LOGGER_MEMORIA, "valor a char: %s \n", valor_a_escribir);
			t_direcciones_fisicas *direccionAmostrar = list_get(direcciones_fisicas_mov_out, 0);
			escribir_memoria(direccionAmostrar->direccion_fisica, direccionAmostrar->tamanio, valor_a_escribir);

			int aux = 0;

			void escribir_todo(void *a_escribir){
				t_direcciones_fisicas *dir_fisica = a_escribir;
				escribir_memoria()
			}*/

			/* char *valor_entero_a_escribir = int_to_char(valor_leido); //Anda mal el int_to_char de los cojones
			int tamanioAescribir = strlen(valor_entero_a_escribir); */




			/*
			void *aux = calloc(sizeof(uint32_t), 1);
			memcpy(aux, &valorObtenido_mov_out, sizeof(uint32_t));
			printf("Valor aux MOV_OUT: %p", &aux);

			int k = 0;
			for (int i = 0; i < list_size(direcciones_fisicas_mov_out); i++)
			{
				t_direcciones_fisicas *direccionAmostrar = list_get(direcciones_fisicas_mov_out, i);
				char *valor_parcial_a_pasar = malloc(direccionAmostrar->tamanio + 1);
				memset(valor_parcial_a_pasar, 0, direccionAmostrar->tamanio + 1);
				memcpy(valor_parcial_a_pasar, aux + k, direccionAmostrar->tamanio);
				/* for (int j = 0; j < direccionAmostrar->tamanio; j++)
				{
					if ( k < tamanioAescribir){
						valor_parcial_a_pasar[j] = valor_entero_a_escribir[k];
						k++;
					}

				} 
				valor_parcial_a_pasar[direccionAmostrar->tamanio] = '\0';
				log_debug(LOGGER_MEMORIA, "Cadena parcial a escribir: %s \n", valor_parcial_a_pasar);
				escribir_memoria(direccionAmostrar->direccion_fisica, direccionAmostrar->tamanio, valor_parcial_a_pasar);
				k += direccionAmostrar->tamanio;
			}
			free(aux);
			*/


			list_clean_and_destroy_elements(direcciones_fisicas_mov_out, free);
			break;

		case PEDIDO_COPY_STRING:
			t_list *direcciones_fisicas_escritura = list_create();
			t_list *direcciones_fisicas_lectura = list_create();
			uint32_t tamanio_copy_string;
			recibir_copystring(cliente_socket, direcciones_fisicas_escritura, direcciones_fisicas_lectura, &tamanio_copy_string);
			char *valor_leido_parcial;
			char *valor_leido_completo = malloc(tamanio_copy_string + 1);
			memset(valor_leido_completo, 0, tamanio_copy_string + 1);

			for (int i = 0; i < list_size(direcciones_fisicas_lectura); i++)
			{
				t_direcciones_fisicas *direccion_fisica_actual = list_get(direcciones_fisicas_lectura, i);
				valor_leido_parcial = leer_memoria_IO(direccion_fisica_actual->direccion_fisica, direccion_fisica_actual->tamanio);
				strcat(valor_leido_completo, valor_leido_parcial);
			}

			int contador_del_tamanio_del_valor_leido_completo = 0;			   // Reiniciar z a 0 en cada iteración del bucle externo
			for (int i = 0; i < list_size(direcciones_fisicas_escritura); i++) // Algo aca esta mal
			{

				t_direcciones_fisicas *direccion_fisica_actual = list_get(direcciones_fisicas_escritura, i);
				char *valor_parcial_a_pasar = malloc(direccion_fisica_actual->tamanio + 1);
				memset(valor_parcial_a_pasar, 0, direccion_fisica_actual->tamanio + 1);
				for (int j = 0; j < direccion_fisica_actual->tamanio; j++)
				{
					valor_parcial_a_pasar[j] = valor_leido_completo[contador_del_tamanio_del_valor_leido_completo];
					contador_del_tamanio_del_valor_leido_completo++;
				}

				valor_parcial_a_pasar[direccion_fisica_actual->tamanio] = '\0';

				escribir_memoria(direccion_fisica_actual->direccion_fisica, direccion_fisica_actual->tamanio, valor_parcial_a_pasar);
				free(valor_parcial_a_pasar);

			}

			free(valor_leido_completo);
			list_clean_and_destroy_elements(direcciones_fisicas_escritura, free);
			list_clean_and_destroy_elements(direcciones_fisicas_lectura, free);
			break;

		case PEDIDO_ESCRIBIR_DATO_STDIN:
			t_list *direcciones_fisicas_a_escribir = list_create();
			char *dato_obtenido_stdin;
			dato_obtenido_stdin = recibir_dato_stdin(direcciones_fisicas_a_escribir, cliente_socket);
			int longitud_DATO = strlen(dato_obtenido_stdin);

			int h = 0;
			for (int i = 0; i < list_size(direcciones_fisicas_a_escribir); i++)
			{
				t_direcciones_fisicas *direccionAmostrar = list_get(direcciones_fisicas_a_escribir, i);
				char *valor_parcial_a_pasar = malloc(direccionAmostrar->tamanio + 1);
				memset(valor_parcial_a_pasar, 0, direccionAmostrar->tamanio + 1);
				for (int j = 0; j < direccionAmostrar->tamanio; j++)
				{
					if (h <= longitud_DATO)
						valor_parcial_a_pasar[j] = dato_obtenido_stdin[h];
					h++;
				}
				valor_parcial_a_pasar[direccionAmostrar->tamanio] = '\0';
				escribir_memoria(direccionAmostrar->direccion_fisica, direccionAmostrar->tamanio, valor_parcial_a_pasar);
				free(direccionAmostrar);
			}
			list_clean_and_destroy_elements(direcciones_fisicas_a_escribir, free);
			break;

		case PEDIDO_A_LEER_DATO_STDOUT:
			t_list *direcciones_fisicas_a_leer = list_create();
			t_list *lista_datos_a_leer = list_create();
			char *valor_leido_stdout;
			recibir_direcciones_de_stdout(cliente_socket, direcciones_fisicas_a_leer);
			int tamanio_registroTotal_stdout = 0;
			for (int i = 0; i < list_size(direcciones_fisicas_a_leer); i++)
			{
				t_direcciones_fisicas *direccionAmostrar = list_get(direcciones_fisicas_a_leer, i);
				valor_leido_stdout = leer_memoria_IO(direccionAmostrar->direccion_fisica, direccionAmostrar->tamanio);
				list_add(lista_datos_a_leer, strdup(valor_leido_stdout));
				log_debug(LOGGER_MEMORIA, "Cadena final leída: %s \n", valor_leido_stdout);
				tamanio_registroTotal_stdout += direccionAmostrar->tamanio;
				free(direccionAmostrar);
			}
			char *valorTotalaDeLeer = concatenar_lista_de_cadenas(lista_datos_a_leer, tamanio_registroTotal_stdout);

			// valorTotalaDeLeer= valorTotalaDeLeer + tamanio_registroTotal_stdout;
			// printf("Valor leido de memoria: %s \n", valorTotalaDeLeer);
			enviar_dato_leido(cliente_socket, valorTotalaDeLeer, tamanio_registroTotal_stdout);
			list_clean_and_destroy_elements(direcciones_fisicas_a_leer, free);
			list_clean_and_destroy_elements(lista_datos_a_leer, free);

			break;

		case PEDIDO_MARCO:
			uint32_t pid_proceso, pagina;
			recibir_pedido_marco(&pagina, &pid_proceso, cliente_socket);
			proceso_memoria = obtener_proceso_pid(pid_proceso);
			if (proceso_memoria == NULL)
			{
				log_error(logger, "No se encontro el proceso con PID %d", pid_proceso);
				break;
			}
			else
			{
				uint32_t marco = obtener_marco_de_pagina(proceso_memoria, pagina);
				log_info(logger, "PID: %d - PAGINA: %d - MARCO: %d", pid_proceso, pagina, marco);
				enviar_marco(cliente_socket, marco);
				break;
			}

		// -------------------
		// -- I/O - MEMORIA --
		// -------------------
		case HANDSHAKE_in_out:
			recibir_mensaje(cliente_socket, logger);
			log_info(logger, "Este deberia ser el canal mediante el cual nos comunicamos con el I/O");
			break;
		// ---------------
		// -- ERRORES --
		// ---------------
		case ERROROPCODE:
			log_error(logger, "Cliente desconectado de %s... con cop -1", server_name);
			break;
		default:
			log_error(logger, "Algo anduvo mal en el server de %s, Cod OP: %d", server_name, cop);
			break;
		}
	}

	log_warning(logger, "El cliente se desconecto de %s server", server_name);
	return;
}

int server_escuchar(t_log *logger, char *server_name, int server_socket)
{
	int cliente_socket = esperar_cliente(server_socket, logger);

	if (cliente_socket != -1)
	{
		pthread_t hilo;
		t_procesar_conexion_args *args = malloc(sizeof(t_procesar_conexion_args));
		args->log = logger;
		args->fd = cliente_socket;
		args->server_name = server_name;
		pthread_create(&hilo, NULL, (void *)procesar_conexion_memoria, (void *)args);
		pthread_detach(hilo);
		return 1;
	}
	return 0;
}

void enviar_respuesta_resize(int cliente_socket, op_cod response)
{
	send(cliente_socket, &response, sizeof(op_cod), 0);
}