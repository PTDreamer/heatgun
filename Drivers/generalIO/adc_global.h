/*
 * adc_global.h
 *
 *  Created on: Jul 27, 2017
 *      Author: jose
 */

#ifndef GENERALIO_ADC_GLOBAL_H_
#define GENERALIO_ADC_GLOBAL_H_

#include "stm32f1xx_hal.h"

typedef struct {
	uint16_t termocouple;
	uint16_t ntc;
} adc_measures_t;

extern volatile adc_measures_t adc_measures[10];
extern volatile uint16_t iron_temp_adc_avg;
#endif /* GENERALIO_ADC_GLOBAL_H_ */
