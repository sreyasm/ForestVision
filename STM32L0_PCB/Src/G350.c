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

    if (Attempt_GSM_UART_Transmit_Wait(huart, "AT+CLCK=\"SC\",2", 1, 200) != 1)
    {
        return 0;
    }

    if (Attempt_GSM_UART_Transmit_Wait(huart, "AT+CREG=2", 1, 200) != 1)
    {
        return 0;
    }

    if (Attempt_GSM_UART_Transmit_Wait(huart, "AT+CREG=0", 1, 200) != 1)
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

// Function that reads CSQ (which is the code for signal strength) of the GSM module
int GSM_Check_Signal(UART_HandleTypeDef * huart)
{
    // Initialize buffer to send to GSM via UART
    char full_command[GSM_TX_BUFFER_SIZE] = "AT+CSQ\x0D";

    // Attempt to send the command 5 times in the case that the first few attempts fail
    for (int i = 0; i < 5; i++)
    {
        // Initialize the buffer to hold the reply from GSM
        char aRxBuffer[GSM_RX_BUFFER_SIZE] = "";

        HAL_StatusTypeDef stat; // Status variable to ensure HAL functions work correctly

        // Set the DMA to receive the reply from GSM
        stat = HAL_UART_Receive_DMA(huart, (uint8_t *) aRxBuffer, GSM_RX_BUFFER_SIZE);
        if (stat != HAL_OK)
        {
            return 0;
        }

        // Transmit the message to GSM via UART
        stat = HAL_UART_Transmit(huart, (uint8_t *) full_command, (uint16_t) strlen(full_command), GSM_TX_TIMEOUT);
        if (stat != HAL_OK)
        {
            return 0;
        }

        uint32_t tickstart = HAL_GetTick();

        while ((aRxBuffer[0] == 0) && ((HAL_GetTick() - tickstart) < 1000));

        // Wait 0.2 seconds for the rest of the buffer to fill
        HAL_Delay(200);

        // Stop the DMA request
        HAL_UART_DMAStop(huart);

        // Check for the presence of the 'OK' string in the buffer
        // If yes, continue to parse the RX buffer
        if (OK_PRESENCE(aRxBuffer))
        {
            // Grab the "Signal strength" reply from the GSM
            int left = 0;
            while (aRxBuffer[left] != ':')
            {
                left++;
            }

            left += 2;
            int right = left + 1;

            while (aRxBuffer[right] != ',')
            {
                right++;
            }

            // If the signal strength is a single digit
            if (left == (right - 1))
            {
                int signal_strength = aRxBuffer[left] - '0';
                return signal_strength;
            }

            // Else if the signal strength has 2 digits
            else if (left == (right - 2))
            {
                int digit_1 = aRxBuffer[left] - '0';
                int digit_2 = aRxBuffer[left+1] - '0';
                int signal_strength = (digit_1 * 10) + digit_2;

                // Signal strength of 99 effectively means there's no signal (according to Luke)
                if (signal_strength == 99)
                {
                    return 0;
                }
                else
                {
                    return signal_strength;
                }
            }
            else // Signal strength is >2 digits. Should not occur whatsoever
            {
                return 0;
            }
        }
    }

    // If program reaches this point, GSM never seems to reply with 'OK'. Therefore, terminate.
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
    // Initialize buffer to send to GSM via UART
    char full_command[GSM_TX_BUFFER_SIZE] = "";
    strcpy(full_command, command); // Copy message into buffer
    strcat(full_command, "\x0D"); // Append CR to string (0xD)

    // Initialize the buffer to hold the reply from GSM
    char aRxBuffer[GSM_RX_BUFFER_SIZE] = "";

    HAL_StatusTypeDef stat; // Status variable to ensure HAL functions work correctly

    // Set the DMA to receive the reply from GSM
    stat = HAL_UART_Receive_DMA(huart, (uint8_t *) aRxBuffer, GSM_RX_BUFFER_SIZE);
    if (stat != HAL_OK)
    {
        return 0;
    }

    // Transmit the message to GSM via UART
    stat = HAL_UART_Transmit(huart, (uint8_t *) full_command, (uint16_t) strlen(full_command), GSM_TX_TIMEOUT);
    if (stat != HAL_OK)
    {
        return 0;
    }

    uint32_t tickstart = HAL_GetTick();
    while ((aRxBuffer[0] == 0) && ((HAL_GetTick() - tickstart) < 1000));

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
    if (Attempt_GSM_UART_Transmit_Wait(huart, "AT+CMGF=1", 1, 200) != 1)
    {
        return 0;
    }

    if (Attempt_GSM_UART_Transmit_Msg(huart, phone_number, message) != 1)
    {
        return 0;
    }

    return 1;
}

int Attempt_GSM_UART_Transmit_Msg(UART_HandleTypeDef * huart, char * phone_number, char * message)
{
    // Initialize buffer to send to GSM via UART
    char full_command[GSM_TX_BUFFER_SIZE] = "";
    strcpy(full_command, "AT+CMGS=\"");
    strcat(full_command, phone_number);
    strcat(full_command, "\"");

    // At this point, the three lines of code above will make
    // full_command = AT+CMGS="phone_number"

    // Make 3 attempts at sending a text message
    for (int i = 0; i < 3; i++)
    {
        // Command to send a text message
        if (Attempt_GSM_UART_Transmit_Wait(huart, full_command, 0, 200) != 1)
        {
            return 0;
        }

        // Transmit the actual message contents of the SMS
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
    char full_command[GSM_MSG_BUFFER_SIZE] = ""; // Initialize buffer to send to GSM via UART
    strcpy(full_command, message); // Copy message into buffer

    // Append <Ctrl-Z> (0x1A) and CR to string (0xD).
    // <Ctrl-Z> is required here since it is specified in the u-blox AT command
    // reference (link above) after typing in the message to be sent in the SMS.
    strcat(full_command, "\x1A\x0D");

    // Initialize the buffer to hold the reply from GSM
    char aRxBuffer[GSM_RX_BUFFER_SIZE] = "";

    HAL_StatusTypeDef stat; // Status variable to ensure HAL functions work correctly

    // Set the DMA to receive the reply from GSM
    stat = HAL_UART_Receive_DMA(huart, (uint8_t *) aRxBuffer, GSM_RX_BUFFER_SIZE);
    if (stat != HAL_OK)
    {
        return 0;
    }

    // Transmit the message to GSM via UART
    stat = HAL_UART_Transmit(huart, (uint8_t *) full_command, (uint16_t) strlen(full_command), GSM_TX_TIMEOUT);
    if (stat != HAL_OK)
    {
        return 0;
    }

    // Wait until the first character in the buffer is updated
    // Once it does, this means that the reply is coming in.
    // Set timeout for ~7 seconds
    uint32_t tickstart = HAL_GetTick();
    while ((aRxBuffer[0] == 0) && ((HAL_GetTick() - tickstart) < 7000));

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
