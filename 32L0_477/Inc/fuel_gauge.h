#ifndef __FUEL_GAUGE_H
#define __FUEL_GAUGE_H

#include <stdint.h>
#include <string.h>
#include "stm32l0xx_hal.h"

void FG_I2C_Setup(I2C_HandleTypeDef *);
uint16_t FG_I2C_Read_Voltage(I2C_HandleTypeDef *);
uint16_t FG_I2C_Read_SOC(I2C_HandleTypeDef *);

#endif /* __FUEL_GAUGE_H */
