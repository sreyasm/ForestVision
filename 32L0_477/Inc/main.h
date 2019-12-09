/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l0xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdbool.h>
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
void set_tim2(int);
void stop_tim2();
void UART_send(uint8_t *);

#define IS_PCB 1
#define MAX_NODE 10
#define SELF_ID 2
#define UPDATE_PERIOD 10 //sec
#define TIMEOUT_PERIOD 30 //sec
struct timeout_arr{
	uint8_t router_ID;
	uint32_t timeout;
	bool active;
};
struct timeout_arr timeout[MAX_NODE];

struct routing_table{
	uint8_t dest_ID;
	uint8_t active;
	uint8_t next_hop;
	uint8_t num_hop;
	uint8_t signal; //From 0 - 99
	uint8_t battery; //From 0 - 100
	uint8_t fire; // 0 or 1
	uint8_t route[MAX_NODE];
};
struct routing_table rt[MAX_NODE];
struct routing_table recv_rt[MAX_NODE];
uint8_t sender_ID;

uint8_t self_ID;
uint8_t self_signal;
uint8_t self_battery;
uint8_t self_fire;

uint8_t req_ACK_UUID;
uint8_t resp_ACK_UUID;

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define GSM_TX_Pin GPIO_PIN_2
#define GSM_TX_GPIO_Port GPIOA
#define GSM_RX_Pin GPIO_PIN_3
#define GSM_RX_GPIO_Port GPIOA
#define GSM_RST_Pin GPIO_PIN_4
#define GSM_RST_GPIO_Port GPIOA
#define DEVLED_R_Pin GPIO_PIN_5
#define DEVLED_R_GPIO_Port GPIOA
#define LED2_Pin GPIO_PIN_0
#define LED2_GPIO_Port GPIOB
#define LED1_Pin GPIO_PIN_2
#define LED1_GPIO_Port GPIOB
#define BAT_SCL_Pin GPIO_PIN_10
#define BAT_SCL_GPIO_Port GPIOB
#define BAT_SDA_Pin GPIO_PIN_11
#define BAT_SDA_GPIO_Port GPIOB
#define LORA_NSS_Pin GPIO_PIN_12
#define LORA_NSS_GPIO_Port GPIOB
#define LORA_SCK_Pin GPIO_PIN_13
#define LORA_SCK_GPIO_Port GPIOB
#define LORA_MISO_Pin GPIO_PIN_14
#define LORA_MISO_GPIO_Port GPIOB
#define LORA_MOSI_Pin GPIO_PIN_15
#define LORA_MOSI_GPIO_Port GPIOB
#define LORA_INT_Pin GPIO_PIN_8
#define LORA_INT_GPIO_Port GPIOA
#define LORA_INT_EXTI_IRQn EXTI4_15_IRQn
#define LORA_EN_Pin GPIO_PIN_9
#define LORA_EN_GPIO_Port GPIOA
#define LORA_RST_Pin GPIO_PIN_10
#define LORA_RST_GPIO_Port GPIOA
#define SWDIO_Pin GPIO_PIN_13
#define SWDIO_GPIO_Port GPIOA
#define SWCLK_Pin GPIO_PIN_14
#define SWCLK_GPIO_Port GPIOA
#define DEVLED_G_Pin GPIO_PIN_4
#define DEVLED_G_GPIO_Port GPIOB
#define L0_TX_Pin GPIO_PIN_6
#define L0_TX_GPIO_Port GPIOB
#define L0_RX_Pin GPIO_PIN_7
#define L0_RX_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
