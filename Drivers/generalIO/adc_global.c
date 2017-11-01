/*
 * adc_global.c
 *
 *  Created on: Jul 27, 2017
 *      Author: jose
 */

#include "adc_global.h"
#define ADC_SAMPLE_TIME ADC_SAMPLETIME_239CYCLES_5
volatile adc_measures_t adc_measures[10];
volatile uint16_t iron_temp_adc_avg = 0;
