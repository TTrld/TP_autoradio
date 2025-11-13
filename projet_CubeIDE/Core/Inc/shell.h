/*
 * shell.h
 *
 *  Created on: 7 juin 2019
 *      Author: Laurent Fiack
 */

#ifndef INC_LIB_SHELL_SHELL_H_
#define INC_LIB_SHELL_SHELL_H_

#include <stdint.h>
#include "cmsis_os.h"

#define UART_DEVICE huart2
#define ARGC_MAX 8
#define BUFFER_SIZE 40
#define SHELL_FUNC_LIST_MAX_SIZE 64

// Forward declaration du type h_shell_t
typedef struct h_shell_struct h_shell_t;

// Structure pour stocker les informations des fonctions du shell
struct shell_func_t {
    char c;
    int (*func)(h_shell_t *shell, int argc, char **argv);
    char *description;
};

// Structure contenant les variables du shell
struct h_shell_struct {
    int shell_func_list_size;
    struct shell_func_t *shell_func_list;
    char print_buffer[BUFFER_SIZE];
    char cmd_buffer[BUFFER_SIZE];
    SemaphoreHandle_t sem_uart_rx;
    char received_char;  // Caractère reçu par interruption
};

// Fonctions publiques
void shell_init(h_shell_t *shell);
void shell_uart_rx_callback(void);
int shell_add(h_shell_t *shell, char c, int (*pfunc)(h_shell_t *shell, int argc, char **argv), char *description);
int shell_run(h_shell_t *shell);
int shell_uart_write(char *s, uint16_t size);

#endif /* INC_LIB_SHELL_SHELL_H_ */