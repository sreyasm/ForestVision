/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "app_x-cube-ai.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define CAM_BUF (320*240)
#define CROP_BUF (100*100*3)
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

CRC_HandleTypeDef hcrc;

DCMI_HandleTypeDef hdcmi;
DMA_HandleTypeDef hdma_dcmi;

I2C_HandleTypeDef hi2c4;

UART_HandleTypeDef huart4;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_DCMI_Init(void);
static void MX_I2C4_Init(void);
static void MX_UART4_Init(void);
static void MX_CRC_Init(void);
/* USER CODE BEGIN PFP */
static void Camera_Config(void);
void RGB24_to_Float_Asym(void *pSrc, void *pDst, uint32_t pixels);
void ImageResize(uint8_t *srcImage, uint32_t srcW, uint32_t srcH,
                 uint32_t pixelSize, uint32_t roiX, uint32_t roiY,
                 uint32_t roiW, uint32_t roiH,  uint8_t *dstImage,
                 uint32_t dstW, uint32_t dstH);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

// AI and initial capture Buffer -- used for capturing original image
// and then as activations buffer for ML API
AI_ALIGNED(4)
ai_u8 initCapAndActivations[AI_NETWORK_DATA_ACTIVATIONS_SIZE];

AI_ALIGNED(4)
ai_u8 ai_in_data[AI_NETWORK_IN_1_SIZE_BYTES];

AI_ALIGNED(4)
ai_u8 ai_out_data[AI_NETWORK_OUT_1_SIZE_BYTES];

uint8_t my_bmp_header[] = {
		  0x42,0x4D,0x36,0x58,0x02,0x00,0x00,0x00,0x00,0x00, // ID=BM, Filsize=(240x320x2+66)
		  0x42,0x00,0x00,0x00,0x28,0x00,0x00,0x00,           // Offset=66d, Headerlen=40d
		  0x40,0x01,0x00,0x00,0xF0,0x00,0x00,0x00,0x01,0x00, // W=320d, H=240d (landscape)
		  0x10,0x00,0x03,0x00,0x00,0x00,0x00,0x58,0x02,0x00, // 16bpp, bitfields, Data=(240x320x2)
		  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,           // nc
		  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,			 // nc
		  0x00,0xF8,0x00,0x00,0xE0,0x07,0x00,0x00,			 // Color Table
		  0x1F,0x00,0x00,0x00								 // Color Table
};
uint8_t BMP_HEADER_100x100[]={
  0x42,0x4D,0x66,0x75,0x00,0x00,0x00,0x00,0x00,0x00, // ID=BM, Filsize=(100x100x3+54)
  0x36,0x00,0x00,0x00,0x28,0x00,0x00,0x00,           // Offset=54d, Headerlen=40d
  0x64,0x00,0x00,0x00,0x64,0x00,0x00,0x00,0x01,0x00, // W=100d, H=100d (landscape)
  0x18,0x00,0x00,0x00,0x00,0x00,0x30,0x75,0x00,0x00, // 24bpp, unkomprimiert, Data=(100x100x3)
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,           // nc
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};          // nc

uint8_t BMP_HEADER_320x240[]={
  0x42,0x4D,0x36,0x84,0x03,0x00,0x00,0x00,0x00,0x00, // ID=BM, Filsize=(320x240x3+54)
  0x36,0x00,0x00,0x00,0x28,0x00,0x00,0x00,           // Offset=54d, Headerlen=40d
  0x40,0x01,0x00,0x00,0xF0,0x00,0x00,0x00,0x01,0x00, // W=320d, H=240d (landscape)
  0x18,0x00,0x00,0x00,0x00,0x00,0x00,0x84,0x03,0x00, // 24bpp, unkomprimiert, Data=(320x240x3)
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,           // nc
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};          // nc

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */
  

  /* Enable I-Cache---------------------------------------------------------*/
  SCB_EnableICache();

  /* Enable D-Cache---------------------------------------------------------*/
  SCB_EnableDCache();

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_DCMI_Init();
  MX_I2C4_Init();
  MX_UART4_Init();
  MX_CRC_Init();
  MX_X_CUBE_AI_Init();
  /* USER CODE BEGIN 2 */

  // init
  Camera_Config();
  aiInit(initCapAndActivations);
  uint8_t RGB888[CAM_BUF*3] = {0};
  HAL_Delay(2000);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1) {
	  // image capture
	  HAL_DCMI_Start_DMA(&hdcmi,DCMI_MODE_SNAPSHOT,(uint32_t) &initCapAndActivations,
		  		(uint32_t) CAM_BUF);
	  HAL_Delay(800);
	  if(HAL_DCMI_GetState(&hdcmi) != HAL_DCMI_STATE_READY) {
		  HAL_DCMI_Stop(&hdcmi);
		  uint8_t error = 'E';
		  HAL_UART_Transmit(&huart4,&error,1,200);
	  }
	  else {
		  // convert to RGB888 and flip R and B
		  int pix = 0;

		  for(int i = 0;i < (CAM_BUF*2);i+=2) {
			  RGB888[pix] = ((initCapAndActivations[i+1]&0xF8));  // 5bit red
			  RGB888[pix+1] = (((initCapAndActivations[i]&0xE0)>>3) |
					  ((initCapAndActivations[i+1]&0x7)<<5));  // 6bit green
			  RGB888[pix+2] = ((initCapAndActivations[i]&0x1F)<<3);  // 5bit blue
			  pix += 3;
		  }

		  /*HAL_UART_Transmit(&huart4, BMP_HEADER_320x240,sizeof(BMP_HEADER_320x240),2000);
	  	  for(int i = 0;i < CAM_BUF*3;i++) {
	      	  HAL_UART_Transmit(&huart4,&RGB888[i],1,2000);
	  	  }*/

		  ImageResize(RGB888, 320, 240, 3, 0, 0, 0, 0, ai_in_data, 100, 100);

		  /*HAL_UART_Transmit(&huart4, BMP_HEADER_100x100,sizeof(BMP_HEADER_100x100),2000);
	  	  for(int i = 0;i < CROP_BUF;i++) {
	      	  HAL_UART_Transmit(&huart4,&ai_in_data[i],1,2000);
	  	  }*/


		  // scale pixels [0,1]
		  RGB24_to_Float_Asym((void*)ai_in_data,(void*)ai_in_data,(uint32_t)CROP_BUF/3);

		  // run inference
		  aiRun((void*)ai_in_data,(void*)ai_out_data);
		  int* percent = (int*) ai_out_data;
	  	  uint8_t result = (*percent) > 0 ? 'F' : 'N';
	  	  HAL_UART_Transmit(&huart4,&result,1,2000);
	  	  HAL_GPIO_WritePin(LED2_GPIO_Port,LED2_Pin,GPIO_PIN_SET);
	  }
	  HAL_Delay(10000);
  	  HAL_GPIO_WritePin(LED2_GPIO_Port,LED2_Pin,GPIO_PIN_RESET);
  	  HAL_NVIC_SystemReset();
    /* USER CODE END WHILE */

  MX_X_CUBE_AI_Process();
    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Configure the main internal regulator output voltage 
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 216;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Activate the Over-Drive mode 
  */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_7) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_UART4|RCC_PERIPHCLK_I2C4;
  PeriphClkInitStruct.Uart4ClockSelection = RCC_UART4CLKSOURCE_PCLK1;
  PeriphClkInitStruct.I2c4ClockSelection = RCC_I2C4CLKSOURCE_PCLK1;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  HAL_RCC_MCOConfig(RCC_MCO1, RCC_MCO1SOURCE_HSI, RCC_MCODIV_1);
}

/**
  * @brief CRC Initialization Function
  * @param None
  * @retval None
  */
static void MX_CRC_Init(void)
{

  /* USER CODE BEGIN CRC_Init 0 */

  /* USER CODE END CRC_Init 0 */

  /* USER CODE BEGIN CRC_Init 1 */

  /* USER CODE END CRC_Init 1 */
  hcrc.Instance = CRC;
  hcrc.Init.DefaultPolynomialUse = DEFAULT_POLYNOMIAL_ENABLE;
  hcrc.Init.DefaultInitValueUse = DEFAULT_INIT_VALUE_ENABLE;
  hcrc.Init.InputDataInversionMode = CRC_INPUTDATA_INVERSION_NONE;
  hcrc.Init.OutputDataInversionMode = CRC_OUTPUTDATA_INVERSION_DISABLE;
  hcrc.InputDataFormat = CRC_INPUTDATA_FORMAT_BYTES;
  if (HAL_CRC_Init(&hcrc) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CRC_Init 2 */

  /* USER CODE END CRC_Init 2 */

}

/**
  * @brief DCMI Initialization Function
  * @param None
  * @retval None
  */
static void MX_DCMI_Init(void)
{

  /* USER CODE BEGIN DCMI_Init 0 */

  /* USER CODE END DCMI_Init 0 */

  /* USER CODE BEGIN DCMI_Init 1 */

  /* USER CODE END DCMI_Init 1 */
  hdcmi.Instance = DCMI;
  hdcmi.Init.SynchroMode = DCMI_SYNCHRO_HARDWARE;
  hdcmi.Init.PCKPolarity = DCMI_PCKPOLARITY_FALLING;
  hdcmi.Init.VSPolarity = DCMI_VSPOLARITY_HIGH;
  hdcmi.Init.HSPolarity = DCMI_HSPOLARITY_HIGH;
  hdcmi.Init.CaptureRate = DCMI_CR_ALL_FRAME;
  hdcmi.Init.ExtendedDataMode = DCMI_EXTEND_DATA_8B;
  hdcmi.Init.JPEGMode = DCMI_JPEG_DISABLE;
  hdcmi.Init.ByteSelectMode = DCMI_BSM_ALL;
  hdcmi.Init.ByteSelectStart = DCMI_OEBS_ODD;
  hdcmi.Init.LineSelectMode = DCMI_LSM_ALL;
  hdcmi.Init.LineSelectStart = DCMI_OELS_ODD;
  if (HAL_DCMI_Init(&hdcmi) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN DCMI_Init 2 */

  /* USER CODE END DCMI_Init 2 */

}

/**
  * @brief I2C4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C4_Init(void)
{

  /* USER CODE BEGIN I2C4_Init 0 */

  /* USER CODE END I2C4_Init 0 */

  /* USER CODE BEGIN I2C4_Init 1 */

  /* USER CODE END I2C4_Init 1 */
  hi2c4.Instance = I2C4;
  hi2c4.Init.Timing = 0x20404768;
  hi2c4.Init.OwnAddress1 = 0;
  hi2c4.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c4.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c4.Init.OwnAddress2 = 0;
  hi2c4.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c4.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c4.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c4) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Analogue filter 
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c4, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Digital filter 
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c4, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C4_Init 2 */

  /* USER CODE END I2C4_Init 2 */

}

/**
  * @brief UART4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_UART4_Init(void)
{

  /* USER CODE BEGIN UART4_Init 0 */

  /* USER CODE END UART4_Init 0 */

  /* USER CODE BEGIN UART4_Init 1 */

  /* USER CODE END UART4_Init 1 */
  huart4.Instance = UART4;
  huart4.Init.BaudRate = 115200;
  huart4.Init.WordLength = UART_WORDLENGTH_8B;
  huart4.Init.StopBits = UART_STOPBITS_1;
  huart4.Init.Parity = UART_PARITY_NONE;
  huart4.Init.Mode = UART_MODE_TX_RX;
  huart4.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart4.Init.OverSampling = UART_OVERSAMPLING_16;
  huart4.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart4.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart4) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN UART4_Init 2 */

  /* USER CODE END UART4_Init 2 */

}

/** 
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void) 
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA2_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA2_Stream1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream1_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOE, LED1_Pin|CAM_RET_Pin|CAM_PWDN_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : PF6 */
  GPIO_InitStruct.Pin = GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF3_TIM10;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  /*Configure GPIO pin : LED3_Pin */
  GPIO_InitStruct.Pin = LED3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED3_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LED2_Pin */
  GPIO_InitStruct.Pin = LED2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED2_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LED1_Pin CAM_RET_Pin CAM_PWDN_Pin */
  GPIO_InitStruct.Pin = LED1_Pin|CAM_RET_Pin|CAM_PWDN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pin : PA8 */
  GPIO_InitStruct.Pin = GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF0_MCO;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */
static void Camera_Config(void) {
	uint8_t reset = 0x80;
	HAL_I2C_Mem_Write(&hi2c4,0x60,0x12,I2C_MEMADD_SIZE_8BIT,&reset,1,2000);
	HAL_Delay(200);

	static uint8_t OV9655_QVGA_TAB[][2]=
	{
	  {0x00,0x00},{0x01,0x80},{0x02,0x80},{0x03,0x02},{0x04,0x03},{0x09,0x01},
	  {0x0b,0x57},{0x0e,0x61},{0x0f,0x40},{0x11,0x01},{0x12,0x62},{0x13,0xc7},
	  {0x14,0x3a},{0x16,0x24},{0x17,0x18},{0x18,0x04},{0x19,0x01},{0x1a,0x81},
	  {0x1e,0x10},{0x24,0x3c},{0x25,0x36},{0x26,0x72},{0x27,0x08},{0x28,0x08},
	  {0x29,0x15},{0x2a,0x00},{0x2b,0x00},{0x2c,0x08},{0x32,0x12},{0x33,0x00},
	  {0x34,0x3f},{0x35,0x00},{0x36,0x3a},{0x38,0x72},{0x39,0x57},{0x3a,0xcc},
	  {0x3b,0x04},{0x3d,0x99},{0x3e,0x02},{0x3f,0xc1},{0x40,0xc0},{0x41,0x41},
	  {0x42,0xc0},{0x43,0x0a},{0x44,0xf0},{0x45,0x46},{0x46,0x62},{0x47,0x2a},
	  {0x48,0x3c},{0x4a,0xfc},{0x4b,0xfc},{0x4c,0x7f},{0x4d,0x7f},{0x4e,0x7f},
	  {0x4f,0x98},{0x50,0x98},{0x51,0x00},{0x52,0x28},{0x53,0x70},{0x54,0x98},
	  {0x58,0x1a},{0x59,0x85},{0x5a,0xa9},{0x5b,0x64},{0x5c,0x84},{0x5d,0x53},
	  {0x5e,0x0e},{0x5f,0xf0},{0x60,0xf0},{0x61,0xf0},{0x62,0x00},{0x63,0x00},
	  {0x64,0x02},{0x65,0x20},{0x66,0x00},{0x69,0x0a},{0x6b,0x5a},{0x6c,0x04},
	  {0x6d,0x55},{0x6e,0x00},{0x6f,0x9d},{0x70,0x21},{0x71,0x78},{0x72,0x11},
	  {0x73,0x01},{0x74,0x10},{0x75,0x10},{0x76,0x01},{0x77,0x02},{0x7A,0x12},
	  {0x7B,0x08},{0x7C,0x16},{0x7D,0x30},{0x7E,0x5e},{0x7F,0x72},{0x80,0x82},
	  {0x81,0x8e},{0x82,0x9a},{0x83,0xa4},{0x84,0xac},{0x85,0xb8},{0x86,0xc3},
	  {0x87,0xd6},{0x88,0xe6},{0x89,0xf2},{0x8a,0x24},{0x8c,0x80},{0x90,0x7d},
	  {0x91,0x7b},{0x9d,0x02},{0x9e,0x02},{0x9f,0x7a},{0xa0,0x79},{0xa1,0x40},
	  {0xa4,0x50},{0xa5,0x68},{0xa6,0x4a},{0xa8,0xc1},{0xa9,0xef},{0xaa,0x92},
	  {0xab,0x04},{0xac,0x80},{0xad,0x80},{0xae,0x80},{0xaf,0x80},{0xb2,0xf2},
	  {0xb3,0x20},{0xb4,0x20},{0xb5,0x00},{0xb6,0xaf},{0xb6,0xaf},{0xbb,0xae},
	  {0xbc,0x7f},{0xbd,0x7f},{0xbe,0x7f},{0xbf,0x7f},{0xbf,0x7f},{0xc0,0xaa},
	  {0xc1,0xc0},{0xc2,0x01},{0xc3,0x4e},{0xc6,0x05},{0xc7,0x81},{0xc9,0xe0},
	  {0xca,0xe8},{0xcb,0xf0},{0xcc,0xd8},{0xcd,0x93},{0x12,0x63},{0x40,0x10},
	  {0x15,0x08}
	};
	for(int i = 0;i < sizeof(OV9655_QVGA_TAB)/2;i++) {
		HAL_I2C_Mem_Write(&hi2c4,0x60,OV9655_QVGA_TAB[i][0],I2C_MEMADD_SIZE_8BIT,&(OV9655_QVGA_TAB[i][1]),1,2000);
		HAL_Delay(2);
	}
}

/**
  * @brief  Performs pixel conversion from 8-bits integer to float simple precision with asymmetric normalization, i.e. in the range [0,+1]
  * @param  pSrc     Pointer to source buffer
  * @param  pDst     Pointer to destination buffer
  * @param  pixels   Number of pixels
  * @retval void     None
  */
void RGB24_to_Float_Asym(void *pSrc, void *pDst, uint32_t pixels)
{
  struct rgb
  {
    uint8_t r, g, b;
  };
  struct rgbf
  {
    float r, g, b;
  };
  struct rgb *pivot = (struct rgb *) pSrc;
  struct rgbf *dest = (struct rgbf *) pDst;
  for (int i = 0; i < pixels; i++)
  {
    dest[i].r = (((float)(pivot[i].b)) / 255.0F);
    dest[i].g = (((float)(pivot[i].g)) / 255.0F);
    dest[i].b = (((float)(pivot[i].r)) / 255.0F);
  }

  /*==> NN input data in the range [0 , +1]*/
}
/**
  * @brief  Performs image (or selected Region Of Interest) resizing using bilinear interpolation
  * @param  srcImage     Pointer to source image buffer
  * @param  srcW         Source image width
  * @param  srcH         Source image height
  * @param  pixelSize    Number of bytes per pixel
  * @param  roiX         Region Of Interest x starting location
  * @param  roiY         Region Of Interest y starting location
  * @param  roiW         Region Of Interest width
  * @param  roiH         Region Of Interest height
  * @param  dstImage     Pointer to destination image buffer
  * @param  dstW         Destination image width
  * @param  dstH         Destination image height
  * @retval void         None
  */
void ImageResize(uint8_t *srcImage, uint32_t srcW, uint32_t srcH,
                 uint32_t pixelSize, uint32_t roiX, uint32_t roiY,
                 uint32_t roiW, uint32_t roiH,  uint8_t *dstImage,
                 uint32_t dstW, uint32_t dstH)
{
  int32_t srcStride;
  float widthRatio;
  float heightRatio;

  int32_t maxWidth;
  int32_t maxHeight;

  float srcX, srcY, dX1, dY1, dX2, dY2;
  int32_t dstX1, srcY1, dstX2, srcY2;

  uint8_t *tmp1, *tmp2;
  uint8_t *p1, *p2, *p3, *p4;

  int32_t offset1;
  int32_t offset2;

  srcStride = pixelSize * srcW;

  widthRatio = ((roiW ? roiW : srcW) / (float) dstW);
  heightRatio = ((roiH ? roiH : srcH) / (float) dstH);

  /* Get horizontal and vertical limits. */
  maxWidth = (roiW ? roiW : srcW) - 1;
  maxHeight = (roiH ? roiH : srcH) - 1;

  for (int32_t y = 0; y < dstH; y++)
  {
    /* Get Y from source. */
    srcY = ((float) y * heightRatio) + roiY;
    srcY1 = (int32_t) srcY;
    srcY2 = (srcY1 == maxHeight) ? srcY1 : srcY1 + 1;
    dY1 = srcY - (float) srcY1;
    dY2 = 1.0f - dY1;

    /* Calculates the pointers to the two needed lines of the source. */
    tmp1 = srcImage + srcY1 * srcStride;
    tmp2 = srcImage + srcY2 * srcStride;

    for (int32_t x = 0; x < dstW; x++)
    {
      /* Get X from source. */
      srcX = x * widthRatio + roiX;
      dstX1 = (int32_t) srcX;
      dstX2 = (dstX1 == maxWidth) ? dstX1 : dstX1 + 1;
      dX1 = srcX - /*(float32)*/dstX1;
      dX2 = 1.0f - dX1;

      /* Calculates the four points (p1,p2, p3, p4) of the source. */
      offset1 = dstX1 * pixelSize;
      offset2 = dstX2 * pixelSize;
      p1 = tmp1 + offset1;
      p2 = tmp1 + offset2;
      p3 = tmp2 + offset1;
      p4 = tmp2 + offset2;
      /* For each channel, interpolate the four points. */
      for (int32_t ch = 0; ch < pixelSize; ch++, dstImage++, p1++, p2++, p3++, p4++)
      {
        *dstImage = (uint8_t)(dY2 * (dX2 * (*p1) + dX1 * (*p2)) + dY1 * (dX2 * (*p3) + dX1 * (*p4)));
      }
    }
  }
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
