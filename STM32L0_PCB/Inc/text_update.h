#ifndef __TEXT_UPDATE_H
#define __TEXT_UPDATE_H

#include <stdint.h>
#include <string.h>
#include "stm32l0xx_hal.h"
#include "G350.h"
#include "fuel_gauge.h"

int text_update(UART_HandleTypeDef *, I2C_HandleTypeDef *, int, int, int);

#endif /* __TEXT_UPDATE_H */
