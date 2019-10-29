#include "G350.h"

int GSM_Init(UART_HandleTypeDef * huart)
{
    int stat;

    stat = GSM_UART_Transmit(huart, "AT", 200, 1);
    if (stat != 1)
    {
        return 0;
    }

    stat = GSM_UART_Transmit(huart, "ATE0", 200, 1);
    if (stat != 1)
    {
        return 0;
    }

    stat = GSM_UART_Transmit(huart, "AT+CMEE=2", 200, 1);
    if (stat != 1)
    {
        return 0;
    }

    stat = GSM_UART_Transmit(huart, "AT+CGMI", 200, 1);
    if (stat != 1)
    {
        return 0;
    }

    stat = GSM_UART_Transmit(huart, "AT+CGMM", 200, 1);
    if (stat != 1)
    {
        return 0;
    }

    stat = GSM_UART_Transmit(huart, "AT+CGMR", 200, 1);
    if (stat != 1)
    {
        return 0;
    }

    stat = GSM_UART_Transmit(huart, "ATI9", 200, 1);
    if (stat != 1)
    {
        return 0;
    }

    stat = GSM_UART_Transmit(huart, "AT+CLCK=\"SC\",2", 200, 1);
    if (stat != 1)
    {
        return 0;
    }

    stat = GSM_UART_Transmit(huart, "AT+CPIN?", 200, 1);
    if (stat != 1)
    {
        return 0;
    }

    stat = GSM_UART_Transmit(huart, "AT+UPSV?", 200, 1);
    if (stat != 1)
    {
        return 0;
    }

    stat = GSM_UART_Transmit(huart, "AT+CCLK?", 200, 1);
    if (stat != 1)
    {
        return 0;
    }

    stat = GSM_UART_Transmit(huart, "AT+CGSN", 200, 1);
    if (stat != 1)
    {
        return 0;
    }

    stat = GSM_UART_Transmit(huart, "AT+COPS?", 200, 1);
    if (stat != 1)
    {
        return 0;
    }

    stat = GSM_UART_Transmit(huart, "AT+CREG=2", 200, 1);
    if (stat != 1)
    {
        return 0;
    }

    stat = GSM_UART_Transmit(huart, "AT+CREG?", 200, 1);
    if (stat != 1)
    {
        return 0;
    }

    stat = GSM_UART_Transmit(huart, "AT+CREG=0", 200, 1);
    if (stat != 1)
    {
        return 0;
    }

    stat = GSM_UART_Transmit(huart, "AT+CSQ", 200, 1);
    if (stat != 1)
    {
        return 0;
    }

    return 1;
}

int GSM_UART_Transmit(UART_HandleTypeDef * huart, char * command, int duration, int check)
{
    // Get string length + 1, since we need to append CR (carriage return) to the string
    int length = ((int) strlen(command)) + 1;

    // Initialize buffer to send to GSM via UART
    char full_command[length];
    strcpy(full_command, command); // Copy message into buffer
    strcat(full_command, "\x0D"); // Append CR to string (0xD)

    // Initialize the buffer to hold the reply from GSM
    char aRxBuffer[50] = "";

    HAL_StatusTypeDef stat; // Status variable to ensure HAL functions work correctly

    // Set the DMA to receive the reply from GSM
    stat = HAL_UART_Receive_DMA(huart, (uint8_t *) aRxBuffer, 50);
    if (stat != HAL_OK)
    {
        return 0;
    }

    // Transmit the message to GSM via UART
    stat = HAL_UART_Transmit(huart, (uint8_t *) full_command, length, 30);
    if (stat != HAL_OK)
    {
        return 0;
    }

    // Wait
    HAL_Delay(duration);

    // Stop the DMA request
    HAL_UART_DMAStop(huart);

    if (check == 1)
    {
        // Check for the presence of the 'OK' string in the buffer
        int resp = OK_PRESENCE(aRxBuffer);
        return resp;
    }
    else
    {
        return 1;
    }
}

int OK_PRESENCE(char * buffer)
{
    int length = (int) strlen(buffer);
    for (int i = 0; i < length-1; i++)
    {
        if (buffer[i] == 'O' && buffer[i+1] == 'K')
        {
            return 1;
        }
    }

    return 0;
}

int GSM_Send_Text(UART_HandleTypeDef * huart, char * ph_number, char * message)
{
    int stat;

    stat = GSM_UART_Transmit(huart, "AT+CSCA?", 200, 1);
    if (stat != 1)
    {
        return 0;
    }

    stat = GSM_UART_Transmit(huart, "AT+CNMI?", 200, 1);
    if (stat != 1)
    {
        return 0;
    }

    stat = GSM_UART_Transmit(huart, "AT+CMGF=1", 200, 1);
    if (stat != 1)
    {
        return 0;
    }

    // Get string length + 1, since we need to append CR (carriage return) to the string
    int command_length = ((int) strlen(ph_number)) + 10;

    // Initialize buffer to send to GSM via UART
    char full_command[command_length];
    strcpy(full_command, "AT+CMGS=\"");
    strcat(full_command, ph_number);
    strcat(full_command, "\"");

    stat = GSM_UART_Transmit(huart, full_command, 200, 0);
    if (stat != 1)
    {
        return 0;
    }

    stat = GSM_UART_Transmit2(huart, message, 10000, 0);
    if (stat != 1)
    {
        return 0;
    }

    return 0;
}

int GSM_UART_Transmit2(UART_HandleTypeDef * huart, char * command, int duration, int check)
{
    // Get string length + 1, since we need to append CR (carriage return) to the string
    int length = ((int) strlen(command)) + 2;

    // Initialize buffer to send to GSM via UART
    char full_command[length];
    strcpy(full_command, command); // Copy message into buffer
    strcat(full_command, "\x1A\x0D"); // Append CR to string (0xD)

    // Initialize the buffer to hold the reply from GSM
    char aRxBuffer[50] = "";

    HAL_StatusTypeDef stat; // Status variable to ensure HAL functions work correctly

    // Set the DMA to receive the reply from GSM
    stat = HAL_UART_Receive_DMA(huart, (uint8_t *) aRxBuffer, 50);
    if (stat != HAL_OK)
    {
        return 0;
    }

    // Transmit the message to GSM via UART
    stat = HAL_UART_Transmit(huart, (uint8_t *) full_command, length, 30);
    if (stat != HAL_OK)
    {
        return 0;
    }

    // Wait
    HAL_Delay(duration);

    // Stop the DMA request
    HAL_UART_DMAStop(huart);

    if (check == 1)
    {
        // Check for the presence of the 'OK' string in the buffer
        int resp = OK_PRESENCE(aRxBuffer);
        return resp;
    }
    else
    {
        return 1;
    }
}
