/**
  ******************************************************************************
  * File Name          : main.h
  * Description        : This file contains the common defines of the application
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
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H
  /* Includes ------------------------------------------------------------------*/

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private define ------------------------------------------------------------*/

#define GUN_TEMP_Pin GPIO_PIN_1
#define GUN_TEMP_GPIO_Port GPIOA
#define NTC_Pin GPIO_PIN_2
#define NTC_GPIO_Port GPIOA
#define VIN_Pin GPIO_PIN_3
#define VIN_GPIO_Port GPIOA
#define HEATER_CONTROL_Pin GPIO_PIN_5
#define HEATER_CONTROL_GPIO_Port GPIOA
#define PWM_FAN_CONTROL_Pin GPIO_PIN_6
#define PWM_FAN_CONTROL_GPIO_Port GPIOA
#define WAKE_Pin GPIO_PIN_3
#define WAKE_GPIO_Port GPIOB
#define WAKE_EXTI_IRQn EXTI3_IRQn
#define ROT1_LEFT_Pin GPIO_PIN_6
#define ROT1_LEFT_GPIO_Port GPIOB
#define ROT1_LEFT_EXTI_IRQn EXTI4_IRQn
#define ROT1_RIGHT_Pin GPIO_PIN_5
#define ROT1_RIGHT_GPIO_Port GPIOB
#define ROT1_RIGHT_EXTI_IRQn EXTI9_5_IRQn
#define ROT1_CLICK_Pin GPIO_PIN_4
#define ROT1_CLICK_GPIO_Port GPIOB
#define ROT1_CLICK_EXTI_IRQn EXTI9_5_IRQn
#define ROT2_LEFT_Pin GPIO_PIN_9
#define ROT2_LEFT_GPIO_Port GPIOB
#define ROT2_LEFT_EXTI_IRQn EXTI9_5_IRQn
#define ROT2_RIGHT_Pin GPIO_PIN_8
#define ROT2_RIGHT_GPIO_Port GPIOB
#define ROT2_RIGHT_EXTI_IRQn EXTI9_5_IRQn
#define ROT2_CLICK_Pin GPIO_PIN_7
#define ROT2_CLICK_GPIO_Port GPIOB
#define ROT2_CLICK_EXTI_IRQn EXTI9_5_IRQn

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

void _Error_Handler(char *, int);

#define Error_Handler() _Error_Handler(__FILE__, __LINE__)

/**
  * @}
  */ 

/**
  * @}
*/ 

#endif /* __MAIN_H */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/


////PWM ON OFF
//GPIO_InitStruct.Pin = GPIO_PIN_8;
//GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
//GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
//HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
//
//GPIO_InitStruct.Pin = GPIO_PIN_8;
//GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
//GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
//HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
//
//PWM SET DUTY
//__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_3, duty / 100.0 * 1500);
//
////	  char *tmpSign = (adc_read < 0) ? "-" : "";
////	  float tmpVal = (adc_read < 0) ? -adc_read : adc_read;
////
////	  int tmpInt1 = tmpVal;                  // Get the integer (678).
////	  float tmpFrac = tmpVal - tmpInt1;      // Get fraction (0.0123).
////	  int tmpInt2 = trunc(tmpFrac * 10000);  // Turn into integer (123).
////
////	  // Print as parts, note that you need 0-padding for fractional bit.
////
////	  sprintf (str, "%s%d.%04d\n", tmpSign, tmpInt1, tmpInt2);
//	  int adc_read = readColdJunctionSensorTemp_mC();
//	  sprintf(str,"%d", adc_read);
//	  //UG_PutString( 0 , 40 , str);
//	 // adc_read = readColdJunctionSensorTemp_X102();
//	//  adc_read = readTipTemperatureCompensated();
//	  adc_read = getReferenceVoltage_mv();//cold_junction = supply
//	  sprintf(str,"%d", adc_read);
//	 // UG_PutString( 60 , 40 , str);
//	  UG_Update();
//	  update_display();

