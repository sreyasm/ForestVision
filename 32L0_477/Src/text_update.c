#include "text_update.h"

// This code has access to the routing table (a.k.a rt)
int text_update(UART_HandleTypeDef * huart, char * phone_number)
{
    char full_msg[GSM_MSG_BUFFER_SIZE] = "";
    int idx = 0;

    for (int i = 0; i < MAX_NODE; i++)
    {
        if (rt[i].active == 1)
        {
            if (idx != 0)
            {
                full_msg[idx++] = '.';
                full_msg[idx++] = ' ';
            }

            if (rt[i].dest_ID < 10)
            {
                full_msg[idx++] = rt[i].dest_ID + '0';
            }
            else
            {
                int id_dig1 = rt[i].dest_ID / 10;
                int id_dig2 = rt[i].dest_ID % 10;
                full_msg[idx++] = id_dig1 + '0';
                full_msg[idx++] = id_dig2 + '0';
            }

            full_msg[idx++] = ',';
            full_msg[idx++] = ' ';

            if (rt[i].fire == 1)
            {
                full_msg[idx++] = 'F';
                full_msg[idx++] = 'I';
                full_msg[idx++] = 'R';
                full_msg[idx++] = 'E';
            }
            else if (rt[i].fire == 0)
            {
                full_msg[idx++] = 'C';
                full_msg[idx++] = 'l';
                full_msg[idx++] = 'e';
                full_msg[idx++] = 'a';
                full_msg[idx++] = 'r';
            }
            else
            {
            	full_msg[idx++] = 'E';
            	full_msg[idx++] = 'r';
            	full_msg[idx++] = 'r';
            	full_msg[idx++] = 'o';
            	full_msg[idx++] = 'r';
            }

            full_msg[idx++] = ',';
            full_msg[idx++] = ' ';

            if (rt[i].battery < 10)
            {
                full_msg[idx++] = rt[i].battery + '0';
            }
            else if (rt[i].battery < 100)
            {
                uint16_t bat_dig1 = rt[i].battery / 10;
                uint16_t bat_dig2 = rt[i].battery % 10;
                full_msg[idx++] = bat_dig1 + '0';
                full_msg[idx++] = bat_dig2 + '0';
            }
            else // Battery percentage is at 100%
            {
                full_msg[idx++] = '1';
                full_msg[idx++] = '0';
                full_msg[idx++] = '0';
            }
        }
    }

    full_msg[idx++] = '\0';

    if (GSM_Send_Text(huart, phone_number, full_msg) == 1)
    {
        return 1;
    }

    return 0;
}
