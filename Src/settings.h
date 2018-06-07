/*
 * settings.h
 *
 *  Created on: Sep 13, 2017
 *      Author: jose
 */

#ifndef SETTINGS_H_
#define SETTINGS_H_

#include "stm32f1xx_hal.h"
#include "tempsensors.h"
#include <stdint.h>
#include "stm32f1xx_hal_flash.h"
#include "gun.h"

#define SETTINGSVERSION 1 /*Change this if you change the struct below to prevent people getting out of sync*/

struct systemSettings {
	uint8_t version;				//Used to track if a reset is needed on firmware upgrade
	uint8_t contrast;
	calData calibrationData;
	gunCoolDown_t coolDown;
	uint8_t lastFanSpeed;
	uint16_t lastGunTemperature;
	gunSleep_t sleep;
} systemSettings;

void saveSettings();
void restoreSettings();
void resetSettings();

#endif /* SETTINGS_H_ */
