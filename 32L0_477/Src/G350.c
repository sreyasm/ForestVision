#include "G350.h"

// Initialization function for the GSM.
// This function returns 1 if the GSM was initialized properly, returns 0 otherwise
// Possible reasons where Initialization could fail:
//     1. No SIM Card inserted
// On the contrary, Init doesn't fail if GSM has no signal.
// Currently, the only way to find out if it has no signal is
// to attempt to send a SMS via the GSM_Send_Text function.
// Refer below for how that works.
int GSM_Init(UART_HandleTypeDef * huart)
{
    if (Attempt_GSM_UART_Transmit_Wait(huart, "AT", 1, 200) != 1)
    {
        return 0;
    }

    if (Attempt_GSM_UART_Transmit_Wait(huart, "ATE0", 1, 200) != 1)
    {
        return 0;
    }

    if (Attempt_GSM_UART_Transmit_Wait(huart, "AT+CMEE=2", 1, 200) != 1)
    {
        return 0;
    }

    if (Attempt_GSM_UART_Transmit_Wait(huart, "AT+CGMI", 1, 200) != 1)
    {
        return 0;
    }

    if (Attempt_GSM_UART_Transmit_Wait(huart, "AT+CGMM", 1, 200) != 1)
    {
        return 0;
    }

    if (Attempt_GSM_UART_Transmit_Wait(huart, "AT+CGMR", 1, 200) != 1)
    {
        return 0;
    }

    if (Attempt_GSM_UART_Transmit_Wait(huart, "ATI9", 1, 200) != 1)
    {
        return 0;
    }

    if (Attempt_GSM_UART_Transmit_Wait(huart, "AT+CLCK=\"SC\",2", 1, 200) != 1)
    {
        return 0;
    }

    if (Attempt_GSM_UART_Transmit_Wait(huart, "AT+CPIN?", 1, 200) != 1)
    {
        return 0;
    }

    if (Attempt_GSM_UART_Transmit_Wait(huart, "AT+UPSV?", 1, 200) != 1)
    {
        return 0;
    }

    if (Attempt_GSM_UART_Transmit_Wait(huart, "AT+CCLK?", 1, 200) != 1)
    {
        return 0;
    }

    if (Attempt_GSM_UART_Transmit_Wait(huart, "AT+CGSN", 1, 200) != 1)
    {
        return 0;
    }

    if (Attempt_GSM_UART_Transmit_Wait(huart, "AT+COPS?", 1, 200) != 1)
    {
        return 0;
    }

    if (Attempt_GSM_UART_Transmit_Wait(huart, "AT+CREG=2", 1, 200) != 1)
    {
        return 0;
    }

    if (Attempt_GSM_UART_Transmit_Wait(huart, "AT+CREG?", 1, 200) != 1)
    {
        return 0;
    }

    if (Attempt_GSM_UART_Transmit_Wait(huart, "AT+CREG=0", 1, 200) != 1)
    {
        return 0;
    }

    if (Attempt_GSM_UART_Transmit_Wait(huart, "AT+CSQ", 1, 200) != 1)
    {
        return 0;
    }

    return 1;
}

// This function calls GSM_UART_Transmit_Wait 5 times in case the GSM for some reason fails
// to reply with 'OK' the first few times. This is to ensure that the microcontroller makes
// multiple attempts to send a command to the GSM before it reports that it fails.
int Attempt_GSM_UART_Transmit_Wait(UART_HandleTypeDef * huart, char * command, int ok_check, int wait_duration)
{
    // Attempt to sent the command 5 times
    for (int i = 0; i < 5; i++)
    {
        if (GSM_UART_Transmit_Wait(huart, command, ok_check, wait_duration) == 1)
        {
            // If the command was received properly by the GSM, return 1
            return 1;
        }
    }

    // If program reaches this point, GSM never sends an 'OK'
    // or there is a problem with the UART transmission. Hence, return 0
    return 0;
}

// This function transmits a single AT command with a wait_duration parameter.
// The wait_duration parameter tells the function how much time to wait (in milliseconds)
// before stopping the DMA to read the buffer. Too short of a wait period would not
// be enough time for the DMA to successfully transfer the data to the buffer.
// For reference, a wait_duration of 1000 would equate to waiting 1 second.
// The ok_check parameter is to specify if you want to check for the 'OK' string in the RX buffer,
// specify it as 1 if you want to check for it, 0 otherwise.
int GSM_UART_Transmit_Wait(UART_HandleTypeDef * huart, char * command, int ok_check, int wait_duration)
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

    // Wait the specified amount of duration (in milliseconds).
    HAL_Delay(wait_duration);

    // Stop the DMA request
    HAL_UART_DMAStop(huart);

    if (ok_check == 1)
    {
        // Check for the presence of the 'OK' string in the buffer
        int resp = OK_PRESENCE(aRxBuffer);
        return resp;
    }
    else // No checking for 'OK' in the buffer
    {
        return 1;
    }
}

// This function scans the string to see if the word 'OK' is in the string.
// Returns 1 if there is, 0 if not.
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

// This function handles the commands needed to instruct the GSM to send a text message.
// NOTE: Only call this function after GSM_Init runs successfully
// (and by run successfully I mean if and only if GSM_Init returns a 1)
// This function returns 1 if it manages to successfully transmit a message,
// returns 0 if it fails. As mentioned before, if the GSM has NO signal despite
// having a SIM Card inserted, this function WILL fail.
// Also, make sure phone_number has a "+1" at the front. Otherwise, it will fail.
int GSM_Send_Text(UART_HandleTypeDef * huart, char * phone_number, char * message)
{
    if (Attempt_GSM_UART_Transmit_Wait(huart, "AT+CSCA?", 1, 200) != 1)
    {
        return 0;
    }

    if (Attempt_GSM_UART_Transmit_Wait(huart, "AT+CNMI?", 1, 200) != 1)
    {
        return 0;
    }

    if (Attempt_GSM_UART_Transmit_Wait(huart, "AT+CMGF=1", 1, 200) != 1)
    {
        return 0;
    }

    /*
    if (Attempt_GSM_UART_Transmit_Msg(huart, full_command, 0, 200) != 1)
    {
        return 0;
    }

    // Transmit the actual message contents of the SMS to be sent
    if (Attempt_GSM_UART_Transmit_Msg(huart, message, 1) != 1)
    {
        return 0;
    }*/

    if (Attempt_GSM_UART_Transmit_Msg(huart, phone_number, message) != 1)
    {
        return 0;
    }

    return 1;
}

int Attempt_GSM_UART_Transmit_Msg(UART_HandleTypeDef * huart, char * phone_number, char * message)
{
    // Get string length + 10, since we need to prepend the proper
    // AT command to the buffer before transmitting it to the GSM
    int command_length = ((int) strlen(phone_number)) + 10;

    // Initialize buffer to send to GSM via UART
    char full_command[command_length];
    strcpy(full_command, "AT+CMGS=\"");
    strcat(full_command, phone_number);
    strcat(full_command, "\"");

    // At this point, the three lines of code above will make
    // full_command = AT+CMGS="phone_number"

    // Make 5 attempts at sending a text message
    for (int i = 0; i < 5; i++)
    {
        // Command to send a text message
        if (Attempt_GSM_UART_Transmit_Wait(huart, full_command, 0, 200) != 1)
        {
            return 0;
        }

        // Transmit the actual message contents of the SMS to be sent
        if (GSM_UART_Transmit_Msg(huart, message, 1) == 1)
        {
            return 1;
        }
    }

    return 0;
}

// This function is similar to GSM_UART_Transmit_Wait, except the following:
//     1. Instead of specifying the amount of time to wait before disabling the
//        DMA and read the buffer, this function will wait until the buffer updates
//        (meaning the reply has been received) before disabling the DMA and reading the buffer
//     2. Instead of appending just CR to the string, we will also append <Ctrl-Z>.
//        For a full explanation of exactly why the <Ctrl-Z> is needed, go to:
//          https://www.u-blox.com/sites/default/files/u-blox-CEL_ATCommands_%28UBX-13002752%29.pdf
//        and go to page 228, under '11.15 Send message +CMGS', read the 11.15.1 Description.
int GSM_UART_Transmit_Msg(UART_HandleTypeDef * huart, char * message, int ok_check)
{
    // Get string length + 2, since we need to append <Ctrl-Z> and CR (carriage return) to the string
    int length = ((int) strlen(message)) + 2;

    char full_command[length]; // Initialize buffer to send to GSM via UART
    strcpy(full_command, message); // Copy message into buffer

    // Append <Ctrl-Z> (0x1A) and CR to string (0xD).
    // <Ctrl-Z> is required here since it is specified in the u-blox AT command
    // reference (link above) after typing in the message to be sent in the SMS.
    strcat(full_command, "\x1A\x0D");

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

    // Wait until the first character in the buffer is updated
    // Once it does, this means that the reply is coming in.
    while(aRxBuffer[0] == 0);

    // Wait 0.2 seconds for the rest of the buffer to fill up
    HAL_Delay(200);

    // Stop the DMA request
    HAL_UART_DMAStop(huart);

    if (ok_check == 1)
    {
        // Check for the presence of the 'OK' string in the buffer
        int resp = OK_PRESENCE(aRxBuffer);
        return resp;
    }
    else // No checking for 'OK' in the buffer
    {
        return 1;
    }
}
