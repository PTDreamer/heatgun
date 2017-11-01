/**
  ******************************************************************************
  * File Name          : main.c
  * Description        : Main program body
  ******************************************************************************
  ** This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * COPYRIGHT(c) 2017 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f1xx_hal.h"
#include "ssd1306.h"
#include "rotary_encoder.h"
#include "adc_global.h"
#include "tempsensors.h"
#include "voltagesensors.h"
#include "screen.h"
#include "main_screen.h"
#include "gui.h"
#include "debug_screen.h"

#include "gun.h"
#include "settings.h"

#define DISPLAY_UPDATE_RATE 200
#define TEMPERATURE_CONTROLLER_UPDATE_RATE 1

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

IWDG_HandleTypeDef hiwdg;
I2C_HandleTypeDef hi2c2;
TIM_HandleTypeDef pwm_timer_htim3;
UART_HandleTypeDef huart1;

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_ADC1_Init(void);
static void MX_I2C2_Init(void);
static void MX_IWDG_Init(void);
static void PWM_Timer_TIM3_Init(void);
static void MX_USART1_UART_Init(void);
void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

RE_State_t RE1_Data;
RE_State_t RE2_Data;

static uint32_t lastActivity, lastTimeDisplay, lastTemperatureControlerTime;

static uint8_t lastActivityNeedsUpdate;

static uint16_t ironTempADCRollingAveraget[10];
static uint8_t rollingAvgTail = 0;
static uint8_t activity = 1;

int main(void)
{
  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();
  /* Configure the system clock */
  SystemClock_Config();
  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_ADC1_Init();
  HAL_ADCEx_Calibration_Start(&hadc1);
  MX_I2C2_Init();
  MX_IWDG_Init();
  PWM_Timer_TIM3_Init();
  MX_USART1_UART_Init();
  HAL_ADC_Start_DMA(&hadc1, (uint32_t*) adc_measures, sizeof(adc_measures)/ sizeof(uint16_t));

  UG_GUI gui;
  ssd1306_init(&hi2c2, 0x78);
  UG_Init(&gui, pset, 128, 64);
  guiInit();
  oled_init();
  oled_draw();

  UG_Update();
  update_display();

  RE_Init(&RE1_Data, ROT1_LEFT_GPIO_Port, ROT1_LEFT_Pin, ROT1_RIGHT_GPIO_Port, ROT1_RIGHT_Pin, ROT1_CLICK_GPIO_Port, ROT1_CLICK_Pin);
  RE_Init(&RE2_Data, ROT2_LEFT_GPIO_Port, ROT2_LEFT_Pin, ROT2_RIGHT_GPIO_Port, ROT2_RIGHT_Pin, ROT2_CLICK_GPIO_Port, ROT2_CLICK_Pin);

  setRotaryEncoder2(&RE2_Data);
  lastTimeDisplay = HAL_GetTick();
  lastTemperatureControlerTime = HAL_GetTick();

  lastActivityNeedsUpdate = 0;
  __HAL_TIM_SET_COMPARE(&pwm_timer_htim3, TIM_CHANNEL_1, 5);
  setPWM_tim(&pwm_timer_htim3);

  restoreSettings();
  setContrast(systemSettings.contrast);
  currentCoolDownSettings = systemSettings.coolDown;
  applyCoolDownSettings();
  setCalData();
  gunInit(&pwm_timer_htim3, HEATER_CONTROL_GPIO_Port, HEATER_CONTROL_Pin);

  if(HAL_GPIO_ReadPin(WAKE_GPIO_Port, WAKE_Pin) == GPIO_PIN_RESET)
	  activity = 0;
 // HAL_IWDG_Start(&hiwdg);
  while (1)
  {

	  //HAL_IWDG_Refresh(&hiwdg);
	  if(HAL_GetTick() - lastTemperatureControlerTime > TEMPERATURE_CONTROLLER_UPDATE_RATE) {
		  readTipTemperatureCompensated(1);
		  handleGun(activity);
		  lastTemperatureControlerTime = HAL_GetTick();
	  }
	  if((lastActivityNeedsUpdate == 1) && (HAL_GetTick() - lastActivity > 100)) {
		  if(HAL_GPIO_ReadPin(WAKE_GPIO_Port, WAKE_Pin) == GPIO_PIN_RESET)
			  activity = 0;
		  else
			  activity = 1;
		  lastActivityNeedsUpdate = 0;
	  }
	  if(HAL_GetTick() - lastTimeDisplay > DISPLAY_UPDATE_RATE) {
		  char sdata[140];
		  for(uint8_t x =0; x<10;++x) {
		  sprintf (sdata, "%d:%d %d %d\n\r", x, adc_measures[x].termocouple, adc_measures[x].ntc, readTipTemperatureCompensated(0));
		 HAL_UART_Transmit(&huart1, (uint8_t *)sdata, strlen(sdata), 1000);
		  }
		  RE_Rotation_t r = RE_Get(&RE1_Data);
		  oled_update();
		  oled_processInput(r, &RE1_Data);
		  oled_draw();
		  lastTimeDisplay = HAL_GetTick();
	  }
  }
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc){
	uint32_t acc = 0;
	uint16_t max = 0;
	uint16_t min = 0xFFFF;
	uint16_t temp;
	if(hadc != &hadc1)
		return;
	for(uint8_t x = 0; x < sizeof(adc_measures)/sizeof(adc_measures[0]); ++x) {
		temp = adc_measures[x].termocouple;
		acc += temp;
		if(temp > max)
			max = temp;
		if(temp < min)
			min = temp;
	}
	acc = acc - min - max;
	uint16_t last = acc / ((sizeof(adc_measures)/sizeof(adc_measures[0])) -2);
	ironTempADCRollingAveraget[rollingAvgTail] = last;
	++rollingAvgTail;
	if(rollingAvgTail > (sizeof(ironTempADCRollingAveraget)/sizeof(ironTempADCRollingAveraget[0]))-1) {
		rollingAvgTail = 0;
	}
	acc = 0;
	for(uint8_t x = 0; x < sizeof(ironTempADCRollingAveraget)/sizeof(ironTempADCRollingAveraget[0]); ++x) {
		acc += ironTempADCRollingAveraget[x];
	}
	iron_temp_adc_avg = acc / (sizeof(ironTempADCRollingAveraget)/sizeof(ironTempADCRollingAveraget[0]));
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	if((GPIO_Pin == ROT1_CLICK_Pin) || (GPIO_Pin == ROT1_RIGHT_Pin) || (GPIO_Pin == ROT1_LEFT_Pin)) {
		RE_Process(&RE1_Data);
	}
	else if((GPIO_Pin == ROT2_CLICK_Pin) || (GPIO_Pin == ROT2_RIGHT_Pin) || (GPIO_Pin == ROT2_LEFT_Pin)) {
		RE_Process(&RE2_Data);
	}
	else if(GPIO_Pin == WAKE_Pin) {
		lastActivityNeedsUpdate = 1;
		lastActivity = HAL_GetTick();
	}
}

/** System Clock Configuration
*/
void SystemClock_Config(void)
{

	 RCC_OscInitTypeDef RCC_OscInitStruct;
	  RCC_ClkInitTypeDef RCC_ClkInitStruct;
	  RCC_PeriphCLKInitTypeDef PeriphClkInit;

	    /**Initializes the CPU, AHB and APB busses clocks
	    */
	  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSI;
	  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	  RCC_OscInitStruct.HSICalibrationValue = 16;
	  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
	  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
	  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL12;
	  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
	  {
	    _Error_Handler(__FILE__, __LINE__);
	  }

	    /**Initializes the CPU, AHB and APB busses clocks
	    */
	  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
	                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
	  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
	  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

	  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
	  {
	    _Error_Handler(__FILE__, __LINE__);
	  }

	  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
	  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
	  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
	  {
	    _Error_Handler(__FILE__, __LINE__);
	  }

	    /**Configure the Systick interrupt time
	    */
	  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

	    /**Configure the Systick
	    */
	  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

	  /* SysTick_IRQn interrupt configuration */
	  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}


/* ADC1 init function */
static void MX_ADC1_Init(void)
{

  ADC_ChannelConfTypeDef sConfig;

    /**Common config
    */
  hadc1.Instance = ADC1;
  hadc1.Init.ScanConvMode = ADC_SCAN_ENABLE;
  hadc1.Init.ContinuousConvMode = ENABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 2;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure Regular Channel
    */
  sConfig.Channel = ADC_CHANNEL_1;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure Regular Channel
    */
  sConfig.Channel = ADC_CHANNEL_2;
  sConfig.Rank = 2;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/* IWDG init function */
static void MX_IWDG_Init(void)
{

  hiwdg.Instance = IWDG;
  hiwdg.Init.Prescaler = IWDG_PRESCALER_32;
  hiwdg.Init.Reload = 4095;
  if (HAL_IWDG_Init(&hiwdg) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/* I2C2 init function */
static void MX_I2C2_Init(void)
{

  hi2c2.Instance = I2C2;
  hi2c2.Init.ClockSpeed = 400000;
  hi2c2.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c2.Init.OwnAddress1 = 0;
  hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c2.Init.OwnAddress2 = 0;
  hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c2) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/* TIM4 init function */
static void PWM_Timer_TIM3_Init(void)
{

	  TIM_MasterConfigTypeDef sMasterConfig;
	  TIM_OC_InitTypeDef sConfigOC;

	  pwm_timer_htim3.Instance = TIM3;
	  pwm_timer_htim3.Init.Prescaler = 1;
	  pwm_timer_htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
	  pwm_timer_htim3.Init.Period = 1500;
	  pwm_timer_htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	  if (HAL_TIM_PWM_Init(&pwm_timer_htim3) != HAL_OK)
	  {
	    _Error_Handler(__FILE__, __LINE__);
	  }

	  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	  if (HAL_TIMEx_MasterConfigSynchronization(&pwm_timer_htim3, &sMasterConfig) != HAL_OK)
	  {
	    _Error_Handler(__FILE__, __LINE__);
	  }

	  sConfigOC.OCMode = TIM_OCMODE_PWM1;
	  sConfigOC.Pulse = 750;
	  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	  if (HAL_TIM_PWM_ConfigChannel(&pwm_timer_htim3, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
	  {
	    _Error_Handler(__FILE__, __LINE__);
	  }

	  HAL_TIM_MspPostInit(&pwm_timer_htim3);

}

/* USART1 init function */
static void MX_USART1_UART_Init(void)
{

  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/** 
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void) 
{
  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);

}

/** Configure pins as 
        * Analog 
        * Input 
        * Output
        * EVENT_OUT
        * EXTI
*/
static void MX_GPIO_Init(void)
{

	  GPIO_InitTypeDef GPIO_InitStruct;

	  /* GPIO Ports Clock Enable */
	  __HAL_RCC_GPIOD_CLK_ENABLE();
	  __HAL_RCC_GPIOA_CLK_ENABLE();
	  __HAL_RCC_GPIOB_CLK_ENABLE();

	  /*Configure GPIO pin Output Level */
	  HAL_GPIO_WritePin(HEATER_CONTROL_GPIO_Port, HEATER_CONTROL_Pin, GPIO_PIN_RESET);

	  /*Configure GPIO pins : NTC_Pin VIN_Pin */
	  GPIO_InitStruct.Pin = NTC_Pin|VIN_Pin;
	  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
	  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	  /*Configure GPIO pin : HEATER_CONTROL_Pin */
	  GPIO_InitStruct.Pin = HEATER_CONTROL_Pin;
	  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	  HAL_GPIO_Init(HEATER_CONTROL_GPIO_Port, &GPIO_InitStruct);

	  /*Configure GPIO pins : WAKE_Pin ROT1_LEFT_Pin ROT1_RIGHT_Pin ROT1_CLICK_Pin
	                           ROT2_LEFT_Pin ROT2_RIGHT_Pin ROT2_CLICK_Pin */
	  GPIO_InitStruct.Pin = WAKE_Pin|ROT1_LEFT_Pin|ROT1_RIGHT_Pin|ROT1_CLICK_Pin
	                          |ROT2_LEFT_Pin|ROT2_RIGHT_Pin|ROT2_CLICK_Pin;
	  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
	  GPIO_InitStruct.Pull = GPIO_PULLUP;
	  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	  /* EXTI interrupt init*/
	  HAL_NVIC_SetPriority(EXTI3_IRQn, 0, 0);
	  HAL_NVIC_EnableIRQ(EXTI3_IRQn);

	  HAL_NVIC_SetPriority(EXTI4_IRQn, 0, 0);
	  HAL_NVIC_EnableIRQ(EXTI4_IRQn);

	  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
	  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
void _Error_Handler(char * file, int line)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  while(1) 
  {
  }
  /* USER CODE END Error_Handler_Debug */ 
}

#ifdef USE_FULL_ASSERT

/**
   * @brief Reports the name of the source file and the source line number
   * where the assert_param error has occurred.
   * @param file: pointer to the source file name
   * @param line: assert_param error line source number
   * @retval None
   */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
    ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */

}

#endif

/**
  * @}
  */ 

/**
  * @}
*/ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
