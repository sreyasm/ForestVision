#include "text_update.h"

int text_update(UART_HandleTypeDef * huart, I2C_HandleTypeDef * hi2c, int cameraID, int is_alive, int is_fire)
{
    // Read the battery percentage from the fuel gauge
    uint16_t bat_percent = FG_I2C_Read_SOC(hi2c);

    if (bat_percent)
    {
        asm("nop");
    }

    char full_msg[GSM_SEND_BUFFER_SIZE] = "";

    int i = 0;

    if (cameraID < 10)
    {
        full_msg[i++] = cameraID + '0';
    }
    else
    {
        int id_dig1 = cameraID / 10;
        int id_dig2 = cameraID % 10;
        full_msg[i++] = id_dig1 + '0';
        full_msg[i++] = id_dig2 + '0';
    }

    full_msg[i++] = ',';
    full_msg[i++] = ' ';

    if (is_alive)
    {
        full_msg[i++] = 'A';
        full_msg[i++] = 'l';
        full_msg[i++] = 'i';
        full_msg[i++] = 'v';
        full_msg[i++] = 'e';
    }
    else
    {
        full_msg[i++] = 'D';
        full_msg[i++] = 'e';
        full_msg[i++] = 'a';
        full_msg[i++] = 'd';
    }

    full_msg[i++] = ',';
    full_msg[i++] = ' ';

    if (is_fire)
    {
        full_msg[i++] = 'F';
        full_msg[i++] = 'I';
        full_msg[i++] = 'R';
        full_msg[i++] = 'E';
    }
    else
    {
        full_msg[i++] = 'N';
        full_msg[i++] = 'o';
        full_msg[i++] = 'n';
        full_msg[i++] = 'e';
    }

    full_msg[i++] = ',';
    full_msg[i++] = ' ';

    if (bat_percent < 10) // Battery percentage is 1 digit
    {
        full_msg[i++] = bat_percent + '0';
        full_msg[i++] = '\0';
    }
    else if (bat_percent < 100) // Battery percentage is 2 digits
    {
        uint16_t dig1 = bat_percent / 10;
        uint16_t dig2 = bat_percent % 10;

        full_msg[i++] = dig1 + '0';
        full_msg[i++] = dig2 + '0';
        full_msg[i++] = '\0';
    }
    else // Battery is at 100%
    {
        full_msg[i++] = '1';
        full_msg[i++] = '0';
        full_msg[i++] = '0';
        full_msg[i++] = '\0';
    }

    if (GSM_Init(huart) == 1)
    {
        int i = GSM_Send_Text(huart, "+12067346538", full_msg);
        if (i)
        {
            return 1;
        }
    }

    return 0;
}
