/*
 * ssd1306.h
 *
 *  Created on: Jul 17, 2017
 *      Author: jose
 */

#ifndef GRAPHICS_SSD1306_H_
#define GRAPHICS_SSD1306_H_
#include "ugui.h"
#include "stm32f1xx_hal.h"

void write_cmd(uint8_t x);
void pset(UG_S16 x, UG_S16 y, UG_COLOR c);
void update_display( void );
#ifdef SSD_USE_SPI
void ssd1306_init(SPI_HandleTypeDef *hspi);
#else
void ssd1306_init(I2C_HandleTypeDef *hi2c, uint8_t oled_addr);
#endif
void setContrast(uint8_t value);
uint8_t getContrast();
#endif /* GRAPHICS_SSD1306_H_ */
