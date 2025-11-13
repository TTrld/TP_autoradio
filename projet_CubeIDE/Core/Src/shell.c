/*
 * shell.c
 *
 *  Created on: 7 juin 2019
 *      Author: Laurent Fiack
 */

#include "shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "usart.h"
#include "gpio.h"

static h_shell_t *shell_instance = NULL;

// Fonction de lecture UART avec attente sur sémaphore
static char uart_read(h_shell_t *shell) {
    // Attendre que le callback donne le sémaphore
    xSemaphoreTake(shell->sem_uart_rx, portMAX_DELAY);
    return shell->received_char;
}

// Fonction d'écriture UART
int shell_uart_write(char *s, uint16_t size) {
    HAL_UART_Transmit(&UART_DEVICE, (uint8_t*)s, size, HAL_MAX_DELAY);
    return size;
}

// Fonction help
static int sh_help(h_shell_t *shell, int argc, char **argv) {
    int i;
    for (i = 0; i < shell->shell_func_list_size; i++) {
        int size = snprintf(shell->print_buffer, BUFFER_SIZE, "%c: %s\r\n",
                            shell->shell_func_list[i].c,
                            shell->shell_func_list[i].description);
        shell_uart_write(shell->print_buffer, size);
    }
    return 0;
}

// Callback appelé depuis l'interruption UART
void shell_uart_rx_callback(void) {
    BaseType_t higher_priority_task_woken = pdFALSE;

    if (shell_instance != NULL) {
        // Donner le sémaphore pour débloquer la tâche shell
        xSemaphoreGiveFromISR(shell_instance->sem_uart_rx, &higher_priority_task_woken);
        portYIELD_FROM_ISR(higher_priority_task_woken);
    }
}

// Initialisation du shell
void shell_init(h_shell_t *shell) {
    shell_instance = shell;

    // Allocation dynamique de la liste des fonctions
    shell->shell_func_list = (struct shell_func_t *)malloc(sizeof(struct shell_func_t) * SHELL_FUNC_LIST_MAX_SIZE);
    if (shell->shell_func_list == NULL) {
        while (1); // Erreur critique
    }
    shell->shell_func_list_size = 0;

    // Création du sémaphore binaire
    shell->sem_uart_rx = xSemaphoreCreateBinary();
    if (shell->sem_uart_rx == NULL) {
        while (1); // Erreur critique
    }

    // Message de bienvenue
    int size = snprintf(shell->print_buffer, BUFFER_SIZE, "\r\n\r\n===== Monsieur Shell v0.2 =====\r\n");
    shell_uart_write(shell->print_buffer, size);

    // Ajout de la commande help
    shell_add(shell, 'h', sh_help, "Help");

    // Démarrer la première réception UART en interruption
    HAL_UART_Receive_IT(&UART_DEVICE, (uint8_t*)&shell->received_char, 1);
}

// Ajout d'une commande au shell
int shell_add(h_shell_t *shell, char c, int (*pfunc)(h_shell_t *shell, int argc, char **argv), char *description) {
    if (shell->shell_func_list_size < SHELL_FUNC_LIST_MAX_SIZE) {
        shell->shell_func_list[shell->shell_func_list_size].c = c;
        shell->shell_func_list[shell->shell_func_list_size].func = pfunc;
        shell->shell_func_list[shell->shell_func_list_size].description = description;
        shell->shell_func_list_size++;
        return 0;
    }
    return -1;
}

// Exécution d'une commande
static int shell_exec(h_shell_t *shell, char *buf) {
    int i;
    char c = buf[0];
    int argc;
    char *argv[ARGC_MAX];
    char *p;

    for (i = 0; i < shell->shell_func_list_size; i++) {
        if (shell->shell_func_list[i].c == c) {
            argc = 1;
            argv[0] = buf;

            // Parsing des arguments
            for (p = buf; *p != '\0' && argc < ARGC_MAX; p++) {
                if (*p == ' ') {
                    *p = '\0';
                    argv[argc++] = p + 1;
                }
            }
            return shell->shell_func_list[i].func(shell, argc, argv);
        }
    }

    // Commande non trouvée
    int size = snprintf(shell->print_buffer, BUFFER_SIZE, "%c: no such command\r\n", c);
    shell_uart_write(shell->print_buffer, size);
    return -1;
}

// Boucle principale du shell
int shell_run(h_shell_t *shell) {
    static char backspace[] = "\b \b";
    static char prompt[] = "> ";
    int reading = 0;
    int pos = 0;

    while (1) {
        shell_uart_write(prompt, 2);
        reading = 1;
        pos = 0;

        while (reading) {
            // Lecture d'un caractère (bloquant sur sémaphore)
            char c = uart_read(shell);

            // Relancer immédiatement la réception pour le prochain caractère
            HAL_UART_Receive_IT(&UART_DEVICE, (uint8_t*)&shell->received_char, 1);

            int size;

            switch (c) {
            case '\r':
            case '\n':
                size = snprintf(shell->print_buffer, BUFFER_SIZE, "\r\n");
                shell_uart_write(shell->print_buffer, size);
                shell->cmd_buffer[pos] = '\0';

                if (pos > 0) {
                    shell_exec(shell, shell->cmd_buffer);
                }

                reading = 0;
                pos = 0;
                break;

            case '\b':
            case 127: // DEL
                if (pos > 0) {
                    pos--;
                    shell_uart_write(backspace, 3);
                }
                break;

            default:
                if (pos < BUFFER_SIZE - 1 && c >= 32 && c <= 126) {
                    shell_uart_write(&c, 1);
                    shell->cmd_buffer[pos++] = c;
                }
                break;
            }
        }
    }
    return 0;
}