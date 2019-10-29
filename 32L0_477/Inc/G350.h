#include <stdint.h>
#include <string.h>
#include "stm32l0xx_hal.h"

int GSM_Init(UART_HandleTypeDef *);
int GSM_UART_Transmit(UART_HandleTypeDef *, char *, int, int);
int OK_PRESENCE(char *);
int GSM_Send_Text(UART_HandleTypeDef *, char *, char *);
int GSM_UART_Transmit2(UART_HandleTypeDef *, char *, int, int);
