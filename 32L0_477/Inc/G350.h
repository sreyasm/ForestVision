#ifndef __G350_H
#define __G350_H

#include <stdint.h>
#include <string.h>
#include "stm32l0xx_hal.h"

#define GSM_TX_TIMEOUT 30
#define GSM_TX_BUFFER_SIZE 25
#define GSM_RX_BUFFER_SIZE 50
#define GSM_MSG_BUFFER_SIZE 100
#define TWILIO_PHONE_NUMBER "+12512610341"

int GSM_Init(UART_HandleTypeDef *);
int Attempt_GSM_UART_Transmit_Wait(UART_HandleTypeDef *, char *, int, int);
int GSM_Check_Signal(UART_HandleTypeDef *);
int GSM_UART_Transmit_Wait(UART_HandleTypeDef *, char *, int, int);
int OK_PRESENCE(char *);
int GSM_Send_Text(UART_HandleTypeDef *, char *, char *);
int Attempt_GSM_UART_Transmit_Msg(UART_HandleTypeDef *, char *, char *);
int GSM_UART_Transmit_Msg(UART_HandleTypeDef *, char *, int);

#endif /* __G350_H */
