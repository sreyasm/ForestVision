#ifndef __TEXT_UPDATE_H
#define __TEXT_UPDATE_H

#include <stdint.h>
#include <string.h>
#include "stm32l0xx_hal.h"
#include "G350.h"
#include "fuel_gauge.h"
#include "main.h"

int text_update(UART_HandleTypeDef *, char *);

#endif /* __TEXT_UPDATE_H */
