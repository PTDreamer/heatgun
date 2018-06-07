/*
 * settings.c
 *
 *  Created on: Sep 13, 2017
 *      Author: jose
 */

#include "settings.h"
#include <string.h>
#define FLASH_ADDR (0x8000000|64512)/*Flash start OR'ed with the maximum amount of flash - 256 bytes*/

void saveSettings() {
	HAL_FLASH_Unlock(); //unlock flash writing
	FLASH_EraseInitTypeDef erase;
	erase.NbPages = 1;
	erase.PageAddress = FLASH_ADDR;
	erase.TypeErase = FLASH_TYPEERASE_PAGES;
	uint32_t error;
	HAL_FLASHEx_Erase(&erase, &error);
	uint16_t *data = (uint16_t*) &systemSettings;
	for (uint8_t i = 0; i < (sizeof(systemSettings) / 2); i++) {
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, FLASH_ADDR + (i * 2), data[i]);
	}
	HAL_FLASH_Lock();
}

void restoreSettings() {
	uint16_t *data = (uint16_t*) &systemSettings;
	for (uint8_t i = 0; i < (sizeof(systemSettings) / 2); i++) {
		data[i] = *(uint16_t *) (FLASH_ADDR + (i * 2));
	}
	if (systemSettings.version != SETTINGSVERSION) {
		resetSettings();
		saveSettings();
	}

}

void resetSettings() {
	systemSettings.version = SETTINGSVERSION;
	systemSettings.contrast = 0x7F;
	systemSettings.coolDown.coolDownTemperature = 100;
	systemSettings.coolDown.fanSpeed = 100;
	systemSettings.coolDown.maxTime = 5;
	systemSettings.calibrationData.calADC_At_200 = 1170;//793;
	systemSettings.calibrationData.calADC_At_300 = 1980;//1323;
	systemSettings.calibrationData.calADC_At_400 = 2460;//1900;
	systemSettings.lastFanSpeed = 80;
	systemSettings.lastGunTemperature = 300;
	systemSettings.sleep.temperature = 200;
	systemSettings.sleep.fanSpeed = 50;
	systemSettings.sleep.maxTime = 3;
}
