/*
 * iron.c
 *
 *  Created on: Sep 14, 2017
 *      Author: jose
 */

#include "gun.h"

#include <stdlib.h>
#include "tempsensors.h"
#include "settings.h"

static gun_mode_t currentMode = mode_standby;
static uint32_t currentModeTimer = 0;
static uint16_t currentSetTemperature = 200;
static uint16_t user_currentSetTemperature = 200;
static uint16_t tempSetPoint;
static uint8_t currentGunPower = 0;
static uint8_t temperatureReachedFlag = 0;
static uint32_t gunTimeOnSinceLastUpdate = 0;
static uint32_t lastGunTimeOn = 0;
static uint8_t isGunOn = 0;
static uint32_t powerCalcTimer;
static uint8_t currentFanSpeed = 80;
static uint8_t currentUserSetFanSpeed = 80;
#define POWER_CALCULATION_PERIOD 1000

static GPIO_TypeDef *m_heater_port = NULL;
static TIM_HandleTypeDef *fanTimer = NULL;
static uint16_t m_heater_pin;
static uint8_t temperatureNeedsSaving = 0;
static uint32_t temperatureNeedsSavingTime = 0;
static uint8_t fanSpeedNeedsSaving = 0;
static uint32_t fanSpeedNeedsSavingTime = 0;

typedef struct setTemperatureReachedCallbackStruct_t setTemperatureReachedCallbackStruct_t;

struct setTemperatureReachedCallbackStruct_t {
	setTemperatureReachedCallback callback;
	setTemperatureReachedCallbackStruct_t *next;
};

typedef struct currentModeChangedCallbackStruct_t currentModeChangedCallbackStruct_t;
struct currentModeChangedCallbackStruct_t {
	currentModeChanged callback;
	currentModeChangedCallbackStruct_t *next;
};
static currentModeChangedCallbackStruct_t *currentModeChangedCallbacks = NULL;
static setTemperatureReachedCallbackStruct_t *temperatureReachedCallbacks = NULL;

uint8_t getCurrentFanSpeed() {
	return currentFanSpeed;
}
uint8_t getCurrentUserSetFanSpeed() {
	return currentUserSetFanSpeed;
}
void setCurrentFanSpeed(uint8_t speed) {
	if(speed == 0) {
		HAL_TIM_PWM_Stop(fanTimer, TIM_CHANNEL_1);
	}
	HAL_TIM_PWM_Start(fanTimer, TIM_CHANNEL_1);
	uint16_t set = speed * 1500.0 / 100.0;
	__HAL_TIM_SET_COMPARE(fanTimer, TIM_CHANNEL_1, set);
	currentFanSpeed = speed;
}
void setCurrentUserSetFanSpeed(uint8_t speed) {
	currentUserSetFanSpeed = speed;
	setCurrentFanSpeed(speed);
	if(speed != systemSettings.lastFanSpeed) {
		fanSpeedNeedsSaving = 1;
		fanSpeedNeedsSavingTime = HAL_GetTick();
	}
}
static void temperatureReached(uint16_t temp) {
	setTemperatureReachedCallbackStruct_t *s = temperatureReachedCallbacks;
	while(s) {
		if(s->callback) {
			s->callback(temp);
		}
		s = s->next;
	}
}

static void modeChanged(gun_mode_t newMode) {
	currentModeChangedCallbackStruct_t *s = currentModeChangedCallbacks;
	while(s) {
		s->callback(newMode);
		s = s->next;
	}
}

void setSetTemperature(uint16_t temperature) {
	user_currentSetTemperature = temperature;
	temperatureReachedFlag = 0;
	setCurrentTemperature(temperature);
	if(temperature != systemSettings.lastGunTemperature) {
		temperatureNeedsSaving = 1;
		temperatureNeedsSavingTime = HAL_GetTick();
	}
}
void setCurrentTemperature(uint16_t temperature) {
	currentSetTemperature = temperature;
	tempSetPoint = temperature;

}
void addSetTemperatureReachedCallback(setTemperatureReachedCallback callback) {
	setTemperatureReachedCallbackStruct_t *s = malloc(sizeof(setTemperatureReachedCallbackStruct_t));
	if(!s)
		while(1){}
	s->callback = callback;
	s->next = NULL;
	setTemperatureReachedCallbackStruct_t *last = temperatureReachedCallbacks;
	if(!last) {
		temperatureReachedCallbacks = s;
		return;
	}
	while(last && last->next != NULL) {
		last = last->next;
	}
	last->next = s;
}
void addModeChangedCallback(currentModeChanged callback) {
	currentModeChangedCallbackStruct_t *s = malloc(sizeof(currentModeChangedCallbackStruct_t));
	s->callback = callback;
	s->next = NULL;
	currentModeChangedCallbackStruct_t *last = currentModeChangedCallbacks;
	while(last && last->next != NULL) {
		last = last->next;
	}
	if(last)
		last->next = s;
	else
		last = s;
}

void setCurrentMode(gun_mode_t mode) {
	currentModeTimer = HAL_GetTick();
	switch (mode) {
		case mode_set:
			setCurrentTemperature(user_currentSetTemperature);
			setCurrentFanSpeed(currentUserSetFanSpeed);
			break;
		case mode_cooling:
			setCurrentTemperature(currentCoolDownSettings.coolDownTemperature);
			setCurrentFanSpeed(currentCoolDownSettings.fanSpeed);
			break;
		case mode_sleep:
			setCurrentTemperature(currentSleepSettings.temperature);
			setCurrentFanSpeed(currentSleepSettings.fanSpeed);
			break;
		case mode_standby:
			setCurrentTemperature(0);
			setCurrentFanSpeed(0);
			break;
		default:
			break;
	}
	currentMode = mode;
	modeChanged(mode);
}
gun_mode_t getCurrentMode() {
	return currentMode;
}
uint16_t getSetTemperature() {
	return currentSetTemperature;
}
uint16_t getUserSetTemperature() {
	return user_currentSetTemperature;
}
void handleGun(uint8_t activity) {
	if(temperatureNeedsSaving && ((HAL_GetTick() - temperatureNeedsSavingTime) > 3000)) {
		systemSettings.lastGunTemperature = user_currentSetTemperature;
		temperatureNeedsSaving = 0;
		saveSettings();
	}
	if(fanSpeedNeedsSaving && ((HAL_GetTick() - fanSpeedNeedsSavingTime) > 3000)) {
		systemSettings.lastFanSpeed = currentUserSetFanSpeed;
		fanSpeedNeedsSaving = 0;
		saveSettings();
	}
	uint32_t currentTime = HAL_GetTick();
	if(currentTime - powerCalcTimer > POWER_CALCULATION_PERIOD) {
		currentGunPower = ((double)(gunTimeOnSinceLastUpdate * 100))/ (double)POWER_CALCULATION_PERIOD;
		gunTimeOnSinceLastUpdate = 0;
		lastGunTimeOn = HAL_GetTick();
		if(currentGunPower > 100)
			currentGunPower = 100;
		powerCalcTimer = HAL_GetTick();
	}
	switch (currentMode) {
		case mode_set:
			if(activity)
				currentModeTimer = currentTime;
			else
				setCurrentMode(mode_sleep);
			break;
		case mode_sleep:
			if(activity)
				setCurrentMode(mode_set);
			else {
				if(currentTime - currentModeTimer > (currentSleepSettings.maxTime * 1000 * 60)) {
					setCurrentMode(mode_standby);
				}
			}
			break;
		case mode_cooling:
			if(currentTime - currentModeTimer > (currentCoolDownSettings.maxTime * 1000 * 60)) {
				setCurrentMode(mode_standby);
			}
			else if(human2adc(currentCoolDownSettings.coolDownTemperature) > iron_temp_adc_avg) {
				setCurrentMode(mode_standby);
			}
			break;
		case mode_standby:
			break;
		default:
			break;
	}
	if(human2adc(tempSetPoint) > iron_temp_adc_avg) {
		if(!isGunOn) {
			isGunOn = 1;
			lastGunTimeOn = HAL_GetTick();
		}
		else {
			gunTimeOnSinceLastUpdate += (HAL_GetTick() - lastGunTimeOn);
			lastGunTimeOn = HAL_GetTick();
		}
		HAL_GPIO_WritePin(m_heater_port, m_heater_pin, GPIO_PIN_SET);
	}
	else {
		if(isGunOn) {
			isGunOn = 0;
			gunTimeOnSinceLastUpdate += (HAL_GetTick()- lastGunTimeOn);
		}
		HAL_GPIO_WritePin(m_heater_port, m_heater_pin, GPIO_PIN_RESET);
	}
	if((getSetTemperature() == readTipTemperatureCompensated(0)) && !temperatureReachedFlag) {
		  temperatureReached(getSetTemperature());
		  temperatureReachedFlag = 1;
	  }
}
uint16_t getCurrentTemperature() {
	return readTipTemperatureCompensated(0);
}
void gunInit(TIM_HandleTypeDef *fanTim, GPIO_TypeDef *heater_port, uint16_t heater_pin, uint16_t temperature, uint8_t fan) {
	m_heater_port = heater_port;
	m_heater_pin = heater_pin;
	powerCalcTimer = HAL_GetTick();
	fanTimer = fanTim;
	currentSetTemperature = temperature;
	user_currentSetTemperature = temperature;
	currentFanSpeed = fan;
	currentUserSetFanSpeed = fan;
}

uint8_t getCurrentPower() {
	return currentGunPower;
}
