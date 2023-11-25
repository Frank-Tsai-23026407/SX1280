/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
SPI_HandleTypeDef hspi1;

TIM_HandleTypeDef htim1;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_SPI1_Init(void);
static void MX_TIM1_Init(void);
/* USER CODE BEGIN PFP */
static void SPI1_TRANSCEIVER_Delay(uint8_t* tx, uint8_t* rx, uint8_t lengh)
{
	HAL_GPIO_WritePin(SX1280_NSS_GPIO_Port, SX1280_NSS_Pin, GPIO_PIN_RESET);
	HAL_Delay(1);
	HAL_SPI_TransmitReceive(&hspi1, tx, rx, lengh, 10);
	HAL_GPIO_WritePin(SX1280_NSS_GPIO_Port, SX1280_NSS_Pin, GPIO_PIN_SET);
}

static void SPI1_TRANSCEIVER(uint8_t* tx, uint8_t* rx, uint8_t lengh)
{
//	__disable_irq();
	HAL_GPIO_WritePin(SX1280_NSS_GPIO_Port, SX1280_NSS_Pin, GPIO_PIN_RESET);
	HAL_SPI_TransmitReceive(&hspi1, tx, rx, lengh, 10);
	HAL_GPIO_WritePin(SX1280_NSS_GPIO_Port, SX1280_NSS_Pin, GPIO_PIN_SET);
//	__enable_irq();
}
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

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
  MX_USART2_UART_Init();
  MX_SPI1_Init();
  MX_TIM1_Init();
  /* USER CODE BEGIN 2 */
  HAL_TIM_Base_Start(&htim1);

  char uart_buf[100];
  int uart_buf_len;
  int received;

  uart_buf_len = sprintf(uart_buf, "SX1280 RX bit rate test\r\n");
  HAL_UART_Transmit(&huart2, (uint8_t *)uart_buf, uart_buf_len, 100);
  //===========================================
  //===========================================
  //===========================================
  uint8_t tx[10] = {};
  uint8_t rx[10] = {};

  // reset
  HAL_GPIO_WritePin(SX1280_RST_GPIO_Port, SX1280_RST_Pin, GPIO_PIN_SET);
  HAL_Delay(100);
  HAL_GPIO_WritePin(SX1280_RST_GPIO_Port, SX1280_RST_Pin, GPIO_PIN_RESET);
  HAL_Delay(100);
  HAL_GPIO_WritePin(SX1280_RST_GPIO_Port, SX1280_RST_Pin, GPIO_PIN_SET);

  //===========================================
  // common transceiver setting for LoRa
  //===========================================

  // setstandby(stdby_xosc)
  *(uint32_t*)tx = 0x80 | 0x01 << 8;
  SPI1_TRANSCEIVER_Delay(tx, rx, 2);

  // setpackettype(packet_type_lora)
  *(uint32_t*)tx = 0x8A | 0x01 << 8; // LoRa mode
  SPI1_TRANSCEIVER_Delay(tx, rx, 2);
  if (rx[0] != 0x65 && rx[0]!=0x64 && rx[0]!=0x54) {
	  uart_buf_len = sprintf(uart_buf, "Error with state %X\r\n", rx[0]);
	  HAL_UART_Transmit(&huart2, (uint8_t *)uart_buf, uart_buf_len, 100);
	  Error_Handler();
  }

  // setrffrequency(rfFrequency)
  *(uint32_t*)tx = 0x86 | 0xB8 << 8 | 0x9D << 16 | 0x89 << 24;
  SPI1_TRANSCEIVER(tx, rx, 4);

  // setbufferbaseaddress()
  *(uint32_t*)tx = 0x8F | 0x80 << 8 | 0x00 << 16;
  SPI1_TRANSCEIVER(tx, rx, 3);

  // setmodulationparams(modparam1, modparam2, modparam3)
  *(uint32_t*)tx = 0x8B | 0x70 << 8 | 0x18 << 16 | 0x01 << 24;
  SPI1_TRANSCEIVER(tx, rx, 4);

  // setpacketparams(pktparam1, pktparam2, pktparam3, pktparam4, pktparam5)
  *(uint32_t*)tx = 0x8C | 0x0C << 8 | 0x80 << 16 | 0x08 << 24;
  *(uint32_t*)(tx+4) = 0x20 | 0x40 << 8 | 0x00 << 16 | 0x00 << 24;
  SPI1_TRANSCEIVER(tx, rx, 8);


    //===========================================
    // Rx Setting and Operations
    //===========================================

    // SetDioIrqParams(irqMask, dio1Mask, dio2Mask, dio3Mask)
    *(uint32_t*)tx = 0x8D | 0x40 << 8 | 0x23 << 16 | 0x00 << 24;
    *(uint32_t*)(tx+4) = 0x01 | 0x00 << 8 | 0x02 << 16 | 0x40 << 24;
    *(uint32_t*)(tx+8) = 0x20;
    SPI1_TRANSCEIVER(tx, rx, 9);

    // SetRx(periodBase, periodBaseCount[15:8], periodBaseCount[7:0])
    *(uint32_t*)tx = 0x82 | 0x00 << 8 | 0x00 << 16 | 0x00 << 24;
    SPI1_TRANSCEIVER(tx, rx, 4);

    // WaitIrq
    while(1)
    {
    	int temp = HAL_GetTick();
  	   *(uint32_t*)tx = 0x15 | 0x00 << 8 | 0x00 << 16 | 0x00 << 24;
  	   SPI1_TRANSCEIVER(tx, rx, 4);
  	   if(rx[3] & 0x02) break;
  	   if(HAL_GetTick() - temp > 3) break;
    }

    // GetPacketStatus()
    *(uint32_t*)tx = 0x1D | 0x00 << 8 | 0x00 << 16 | 0x00 << 24;
    *(uint32_t*)(tx+4) = 0x00 | 0x00 << 8 | 0x00 << 16;
    SPI1_TRANSCEIVER(tx, rx, 7);

    // ClrIrqStatus(irqMask)
    *(uint32_t*)tx = 0x97 | 0xFF << 8 | 0xFF << 16;
    SPI1_TRANSCEIVER(tx, rx, 3);

    // GetRxBufferStatus()
    *(uint32_t*)tx = 0x17 | 0x00 << 8 | 0x00 << 16 | 0x00 << 24;
    SPI1_TRANSCEIVER(tx, rx, 4);

    // ReadBuffer(offset, payloadLengthRx)
	 *(uint32_t*)tx = 0x1B | 0x00 << 8 | 0x00 << 16 | 0x00 << 24;
	 *(uint32_t*)(tx+4) = 0x00 | 0x00 << 8 | 0x00 << 16 | 0x00 << 24;
	 *(uint32_t*)(tx+8) = 0x00 | 0x00 << 8 | 0x00 << 16 | 0x00 << 24;
	 SPI1_TRANSCEIVER(tx, rx, 11);

	 received = *(int*) (rx+3);


  // FrequencyError[Hz]

  //===========================================
  //===========================================
  //===========================================


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  int time_temp = HAL_GetTick();
	     uint8_t received_arr[100] = {0};
	     int pre_get = 0;
	     uint8_t sum;
	     float bps;
	     uart_buf_len = sprintf(uart_buf, "SX1280 RX bit rate test\r\n");
	     HAL_UART_Transmit(&huart2, (uint8_t *)uart_buf, uart_buf_len, 100);
	     while (1)
	     {
	  	      // WriteBuffer(offset, *data)
	  	      *(uint32_t*)tx = 0x1A | 0x00 << 8 | 0x00 << 16 | 0x00 << 24;
	  	      *(uint32_t*)(tx+4) = 0x00000000;
	  	      SPI1_TRANSCEIVER(tx, rx, 8);
	  	      // SetRx(periodBase, periodBaseCount[15:8], periodBaseCount[7:0])
	    	     *(uint32_t*)tx = 0x82 | 0x00 << 8 | 0x00 << 16 | 0x00 << 24;
	    	     SPI1_TRANSCEIVER(tx, rx, 4);

	    	     // WaitIrq
	    	     while(1)
	    	     {
	    	  	   *(uint32_t*)tx = 0x15 | 0x00 << 8 | 0x00 << 16 | 0x00 << 24;
	    	  	   SPI1_TRANSCEIVER(tx, rx, 4);
	    	  	   if(rx[3] & 0x02) break;
	    	     }



	    	     // GetPacketStatus()
	    	     *(uint32_t*)tx = 0x1D | 0x00 << 8 | 0x00 << 16 | 0x00 << 24;
	    	     *(uint32_t*)(tx+4) = 0x00 | 0x00 << 8 | 0x00 << 16;
	    	     SPI1_TRANSCEIVER(tx, rx, 7);

	    	     // ClrIrqStatus(irqMask)
	    	     *(uint32_t*)tx = 0x97 | 0xFF << 8 | 0xFF << 16;
	    	     SPI1_TRANSCEIVER(tx, rx, 3);

	    	    *(uint32_t*)tx = 0x15 | 0x00 << 8 | 0x00 << 16 | 0x00 << 24;
	    	     SPI1_TRANSCEIVER(tx, rx, 4);

	    	     // GetRxBufferStatus()
	    	     *(uint32_t*)tx = 0x17 | 0x00 << 8 | 0x00 << 16 | 0x00 << 24;
	    	     SPI1_TRANSCEIVER(tx, rx, 4);

	    	     // ReadBuffer(offset, payloadLengthRx)
	    	     *(uint32_t*)tx = 0x1B | 0x00 << 8 | 0x00 << 16 | 0x00 << 24;
	    	     *(uint32_t*)(tx+4) = 0x00 | 0x00 << 8 | 0x00 << 16 | 0x00 << 24;
	    	     *(uint32_t*)(tx+8) = 0x00 | 0x00 << 8 | 0x00 << 16 | 0x00 << 24;
	    	     SPI1_TRANSCEIVER(tx, rx, 11);

	    	     uart_buf_len = sprintf(uart_buf, "RX: %ld %ld\r\n", *(uint32_t*)(rx+3), *(uint32_t*)(rx+7));
	    	     HAL_UART_Transmit(&huart2, (uint8_t *)uart_buf, uart_buf_len, 100);

	    		 received = *(int*) (rx+3);


	    	     // FrequencyError[Hz]
	    		 if(received < 100 && received >= 0) // valid
	    		 {
	    			if(pre_get > received)
	    			{
	    				time_temp = HAL_GetTick() - time_temp;
	    				sum = 0;
	    				for(int i = 0; i < 100; i++)
	    				{
	    					if(received_arr[i] == 1)
	    					{
	    						sum ++;
	    					}
	    				}
	    				bps = sum*8*8*1000./time_temp;
	    				uart_buf_len = sprintf(uart_buf, "Bit Rate: %15.5f bps, sum: %03d\r\n", bps, sum);
	    				HAL_UART_Transmit(&huart2, (uint8_t *)uart_buf, uart_buf_len, 100);

	    				// end
						for(int i = 0; i < 100; i++)
						{
							received_arr[i] = 0;
						}
						time_temp = HAL_GetTick();
	    			}
	    			pre_get = received;
	    			received_arr[received] = 1;
	    		 }
	     }
    /* USER CODE END WHILE */

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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 0;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 65535;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(SX1280_RST_GPIO_Port, SX1280_RST_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(SX1280_NSS_GPIO_Port, SX1280_NSS_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : SX1280_RST_Pin */
  GPIO_InitStruct.Pin = SX1280_RST_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(SX1280_RST_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PA8 */
  GPIO_InitStruct.Pin = GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : SX1280_BUSY_Pin */
  GPIO_InitStruct.Pin = SX1280_BUSY_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(SX1280_BUSY_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : SX1280_NSS_Pin */
  GPIO_InitStruct.Pin = SX1280_NSS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(SX1280_NSS_GPIO_Port, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
//	HAL_UART_Receive_IT(&huart1, rx_buff, 10); //You need to toggle a breakpoint on this line!
	;
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
  __disable_irq();
  while (1)
  {
  }
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
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
