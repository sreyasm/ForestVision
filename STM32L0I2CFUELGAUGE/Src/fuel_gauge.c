#include "fuel_gauge.h"

void FG_I2C_Setup(I2C_HandleTypeDef * hi2c)
{
    // Initialize buffers
    uint8_t r_buffer_byte;
    uint8_t r_buffer_word[2];
    uint8_t w_buffer_byte;
    uint8_t w_buffer_word[2];

    // UNSEAL
    w_buffer_word[0] = 0x00;
    w_buffer_word[1] = 0x80;
    HAL_I2C_Mem_Write(hi2c, 0xAA, 0x00, 1, w_buffer_word, 2, 100);
    HAL_I2C_Mem_Write(hi2c, 0xAA, 0x00, 1, w_buffer_word, 2, 100);

    // Send SET_CFGUPDATE subcommand, Control(0x0013)
    w_buffer_word[0] = 0x13;
    w_buffer_word[1] = 0x00;
    HAL_I2C_Mem_Write(hi2c, 0xAA, 0x00, 1, w_buffer_word, 2, 100);

    // Confirm CFGUPDATE mode by polling Flags() register
    while(1)
    {
        HAL_I2C_Mem_Read(hi2c, 0xAA, 0x06, 1, &r_buffer_byte, 1, 100);
        if ((r_buffer_byte & 0x10) != 0) // If bit 4 is set, break
        {
            break;
        }
    }

    // Write 0x00 using BlockDataControl() command (0x61)
    w_buffer_byte = 0x00;
    HAL_I2C_Mem_Write(hi2c, 0xAA, 0x61, 1, &w_buffer_byte, 1, 100);

    // Write 0x52 using the DataBlockClass() command (0x3E)
    w_buffer_byte = 0x52;
    HAL_I2C_Mem_Write(hi2c, 0xAA, 0x3E, 1, &w_buffer_byte, 1, 100);

    // Write the block offset location using DataBlock() command (0x3F)
    w_buffer_byte = 0x00;
    HAL_I2C_Mem_Write(hi2c, 0xAA, 0x3F, 1, &w_buffer_byte, 1, 100);

    HAL_Delay(200);

    // Read the 1-byte checksum using the BlockDataChecksum() command (0x60)
    HAL_I2C_Mem_Read(hi2c, 0xAA, 0x60, 1, &r_buffer_byte, 1, 100);
    uint8_t tmp_checksum = 0xFF - r_buffer_byte;

    // ---------- ACTUAL CONFIGURATION STARTS HERE ----------

    // Read the Design Capacity
    HAL_I2C_Mem_Read(hi2c, 0xAA, 0x4A, 1, r_buffer_word, 2, 100);
    tmp_checksum = tmp_checksum - r_buffer_word[0] - r_buffer_word[1];

    // Read the Design Energy
    HAL_I2C_Mem_Read(hi2c, 0xAA, 0x4C, 1, r_buffer_word, 2, 100);
    tmp_checksum = tmp_checksum - r_buffer_word[0] - r_buffer_word[1];

    // Read the Terminate Voltage
    HAL_I2C_Mem_Read(hi2c, 0xAA, 0x50, 1, r_buffer_word, 2, 100);
    tmp_checksum = tmp_checksum - r_buffer_word[0] - r_buffer_word[1];

    // Read the Taper Rate
    HAL_I2C_Mem_Read(hi2c, 0xAA, 0x5B, 1, r_buffer_word, 2, 100);
    tmp_checksum = tmp_checksum - r_buffer_word[0] - r_buffer_word[1];

    // Define new Configuration Values
    uint8_t new_DC[2] = {0x19, 0xC8};
    uint8_t new_DE[2] = {0x5F, 0x64};
    uint8_t new_TV[2] = {0x07, 0xD0};
    uint8_t new_TR[2] = {0x00, 0x84};

    tmp_checksum = tmp_checksum + new_DC[0] + new_DC[1] + new_DE[0] + new_DE[1];
    tmp_checksum = tmp_checksum + new_TV[0] + new_TV[1] + new_TR[0] + new_TR[1];

    // Write the new Design Capacity
    HAL_I2C_Mem_Write(hi2c, 0xAA, 0x4A, 1, new_DC, 2, 100);

    // Write the new Design Energy
    HAL_I2C_Mem_Write(hi2c, 0xAA, 0x4C, 1, new_DE, 2, 100);

    // Write the new Terminate Voltage
    HAL_I2C_Mem_Write(hi2c, 0xAA, 0x50, 1, new_TV, 2, 100);

    // Write the new Taper Rate
    HAL_I2C_Mem_Write(hi2c, 0xAA, 0x5B, 1, new_TR, 2, 100);

    tmp_checksum = 0xFF - tmp_checksum;

    // Write new checksum
    HAL_I2C_Mem_Write(hi2c, 0xAA, 0x60, 1, &tmp_checksum, 1, 100);

    // Exit CFGUPDATE mode by sending SOFT_RESET subcommand, Control(0x0042)
    w_buffer_word[0] = 0x42;
    w_buffer_word[1] = 0x00;
    HAL_I2C_Mem_Write(hi2c, 0xAA, 0x00, 1, w_buffer_word, 2, 100);

    // Confirm CFGUPDATE has been exited by polling Flags() register
    while(1)
    {
        HAL_I2C_Mem_Read(hi2c, 0xAA, 0x06, 1, &r_buffer_byte, 1, 100);
        if ((r_buffer_byte & 0x10) == 0) // If bit 4 is set, break
        {
            break;
        }
    }

    // Return to SEALED mode
    w_buffer_word[0] = 0x20;
    w_buffer_word[1] = 0x00;
    HAL_I2C_Mem_Write(hi2c, 0xAA, 0x00, 1, w_buffer_word, 2, 100);
}

uint16_t FG_I2C_Read_Voltage(I2C_HandleTypeDef * hi2c)
{
    uint8_t buffer[2];
    HAL_I2C_Mem_Read(hi2c, 0xAA, 0x04, 1, buffer, 2, 100);

    uint16_t full_buffer = ((uint16_t) buffer[1] << 8) | buffer[0];
    return full_buffer;
}

uint16_t FG_I2C_Read_SOC(I2C_HandleTypeDef * hi2c)
{
    uint8_t buffer[2];
    HAL_I2C_Mem_Read(hi2c, 0xAA, 0x1C, 1, buffer, 2, 100);

    uint16_t full_buffer = ((uint16_t) buffer[1] << 8) | buffer[0];
    return full_buffer;
}
