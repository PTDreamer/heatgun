/*
 * main_screen.c
 *
 *  Created on: Aug 2, 2017
 *      Author: jose
 */

#include "main_screen.h"
#include "tempsensors.h"
#include "../../../Src/gun.h"
#include "../../../Src/settings.h"
#include "../../../generalIO/rotary_encoder.h"

static uint8_t hasIron = 1;
static uint16_t m_mode = 0;
static uint16_t m_temp = 0;
static uint16_t m_fan = 0;
static char *modestr[] = {"STBY", "COOL", "SET "};
static widget_t *ironTempWidget;
static widget_t *ironTempLabelWidget;
static widget_t *noIronWidget;

static widget_t *tempSetWidget = NULL;
static widget_t *fanSpeedSetWidget = NULL;

static int main_screenProcessInput(screen_t * scr, RE_Rotation_t input, RE_State_t *state);
static int temperatureAndFanSpeed_widgetProcessInput(widget_t *widget, RE_Rotation_t input, RE_State_t *state);

static uint8_t tempIsBeeingEdited = 0;
static uint8_t fanSpeedIsBeeingEdited = 0;
static uint32_t tempEditTimeout = 0;
static uint32_t fanSpeedEditTimeout = 0;
static RE_State_t *RE2;

void setRotaryEncoder2(RE_State_t *rotary) {
	RE2 = rotary;
}

static void setTemp(uint16_t *value) {
	m_temp = *value;
	setSetTemperature(m_temp);
}

static void * getTemp() {
	if(tempIsBeeingEdited) {
		m_temp = getUserSetTemperature();
		return &m_temp;
	}
	else {
		m_temp = getCurrentTemperature();
	}
	return &m_temp;
}

static void setFanSpeed(uint16_t *value) {
	m_fan = *value;
	setCurrentUserSetFanSpeed(m_fan);
}

static void * getFanSpeed() {
	m_fan = getCurrentFanSpeed();
	return &m_fan;
}
static void setMode(uint16_t *value) {
	m_mode = *value;
	setCurrentMode((gun_mode_t)m_mode);
}

static void * getMode() {
	m_mode = getCurrentMode();
	return &m_mode;
}


static uint16_t temp;
const unsigned char therm [] = {
		0x00, 0x00, 0x00, 0xC0, 0x20, 0xC0, 0x00, 0x00,
		0x00, 0x20, 0x70, 0xFF, 0xFE, 0xFF, 0x70, 0x20

};

static void * main_screen_getAmbTemp() {
	temp = readColdJunctionSensorTemp_mC() / 100;
	return &temp;
}

static void * main_screen_getGunPower() {
	temp = getCurrentPower();
	return &temp;
}
static void main_screen_init(screen_t *scr) {
	UG_FontSetHSpace(0);
	UG_FontSetVSpace(0);
	default_init(scr);
}
void main_screenUpdate(screen_t *scr) {
	uint16_t t = readTipTemperatureCompensated(0);
	if((t > 500) && hasIron) {
		UG_FillScreen(C_BLACK);
		ironTempLabelWidget->enabled = 0;
		ironTempWidget->enabled = 0;
		noIronWidget->enabled = 1;
		hasIron = 0;
	}
	else if((t <= 500) && !hasIron){
		UG_FillScreen(C_BLACK);
		ironTempLabelWidget->enabled = 1;
		ironTempWidget->enabled = 1;
		noIronWidget->enabled = 0;
		hasIron = 1;
	}
	default_screenUpdate(scr);

}
void main_screen_setup(screen_t *scr) {
	scr->draw = &default_screenDraw;
	scr->processInput = &main_screenProcessInput;
	scr->init = &main_screen_init;
	scr->update = &main_screenUpdate;

	//gun tip temperature display
	widget_t *widget = screen_addWidget(scr);
	widgetDefaultsInit(widget, widget_editable);
	widget->editable.selectable.processInput = &temperatureAndFanSpeed_widgetProcessInput;
	widget->posX = 45;
	widget->posY = 20;
	widget->font_size = &FONT_12X20;
	widget->editable.inputData.getData = &getTemp;
	widget->editable.inputData.number_of_dec = 0;
	widget->editable.inputData.type = field_uinteger16;
	widget->editable.big_step = 10;
	widget->editable.step = 1;
	widget->editable.max_value = 500;
	widget->editable.min_value = 20;
	widget->editable.selectable.tab = 0;
	widget->editable.setData = (void (*)(void *))&setTemp;
	widget->reservedChars = 3;
	widget->editable.selectable.state = widget_edit;
	scr->current_widget = widget;
	tempSetWidget = widget;

	//ºC label next to iron tip temperature
	widget = screen_addWidget(scr);
	widgetDefaultsInit(widget, widget_label);
	char *s = "\247C";
	strcpy(widget->displayString, s);
	widget->posX = 50 + 3 * 12 -5;
	widget->posY = 20;
	widget->font_size = &FONT_12X20;
	widget->reservedChars = 2;
	widget->draw = &default_widgetDraw;
	ironTempLabelWidget = widget;

	//power display
	widget = screen_addWidget(scr);
	widgetDefaultsInit(widget, widget_display);
	widget->posX = 93;
	widget->posY = 1;
	widget->font_size = &FONT_8X14;
	widget->displayWidget.getData = &main_screen_getGunPower;
	widget->displayWidget.number_of_dec = 0;
	widget->displayWidget.type = field_uinteger16;
	widget->reservedChars = 3;

	//power percentage symbol
	widget = screen_addWidget(scr);
	widgetDefaultsInit(widget, widget_label);
	s = "%";
	strcpy(widget->displayString, s);
	widget->posX = 119;
	widget->posY = 1;
	widget->font_size = &FONT_8X14;
	widget->reservedChars = 1;
	widget->draw = &default_widgetDraw;

	//fan speed display
	widget = screen_addWidget(scr);
	widgetDefaultsInit(widget, widget_editable);
	widget->editable.selectable.processInput = &temperatureAndFanSpeed_widgetProcessInput;
	widget->posX = 45;
	widget->posY = 40;
	widget->font_size = &FONT_12X20;
	widget->editable.inputData.getData = &getFanSpeed;
	widget->editable.inputData.number_of_dec = 0;
	widget->editable.inputData.type = field_uinteger16;
	widget->editable.big_step = 10;
	widget->editable.step = 1;
	widget->editable.max_value = 100;
	widget->editable.min_value = 0;
	widget->editable.selectable.tab = 0;
	widget->editable.setData = (void (*)(void *))&setFanSpeed;
	widget->reservedChars = 3;
	widget->editable.selectable.state = widget_edit;
	fanSpeedSetWidget = widget;

	widget = screen_addWidget(scr);
	widgetDefaultsInit(widget, widget_label);
	strcpy(widget->displayString, "NO IRON");
	widget->posX = 20;
	widget->posY = 20 + 5;
	widget->font_size = &FONT_12X20;
	widget->reservedChars = 7;
	widget->draw = &default_widgetDraw;
	noIronWidget = widget;
	widget->enabled = 0;

	//Thermometer bmp next to Ambient temperature
	widget = screen_addWidget(scr);
	widgetDefaultsInit(widget, widget_bmp);
	widget->posX = 1;
	widget->posY = 1;
	widget->displayBmp.bmp.p = (unsigned char*)therm;
	widget->displayBmp.bmp.width = 8;
	widget->displayBmp.bmp.height = 16;
	widget->displayBmp.bmp.colors = 2;
	widget->displayBmp.bmp.bpp = 8;

	//Ambient temperature display
	widget = screen_addWidget(scr);
	widgetDefaultsInit(widget, widget_display);
	widget->posX = 11;
	widget->posY = 1;
	widget->font_size = &FONT_8X14;
	widget->displayWidget.getData = &main_screen_getAmbTemp;
	widget->displayWidget.number_of_dec = 1;
	widget->displayWidget.type = field_uinteger16;
	widget->reservedChars = 4;

	// mode
	widget = screen_addWidget(scr);
	widgetDefaultsInit(widget, widget_multi_option);
	widget->posX = 50;
	widget->posY = 1;
	widget->font_size = &FONT_8X14;
	widget->multiOptionWidget.editable.inputData.getData = &getMode;
	widget->multiOptionWidget.editable.inputData.number_of_dec = 0;
	widget->multiOptionWidget.editable.inputData.type = field_uinteger16;
	widget->multiOptionWidget.editable.big_step = 0;
	widget->multiOptionWidget.editable.step = 0;
	widget->multiOptionWidget.editable.selectable.tab = 2;
	widget->multiOptionWidget.editable.setData = (void (*)(void *))&setMode;
	widget->reservedChars = 5;
	widget->multiOptionWidget.options = modestr;
	widget->multiOptionWidget.numberOfOptions = 4;
	widget->multiOptionWidget.currentOption = 0;
	widget->multiOptionWidget.defaultOption = 0;
	fanSpeedSetWidget->editable.selectable.state = widget_idle;
	tempSetWidget->editable.selectable.state = widget_idle;
}

static int main_screenProcessInput(screen_t * scr, RE_Rotation_t input, RE_State_t *state) {
	int ret = -1;
	if(input == Rotate_Increment_while_click) {
		uint8_t i = scr->index;
		++i;
		if(i == screen_last_scrollable)
			i = 1;
		return i;
	}
	else if(input == Rotate_Decrement_while_click) {
		uint8_t i = scr->index;
		--i;
		if(i == 0)
			i = screen_last_scrollable - 1;
		return i;
	}
	if(input == Click) {
		if(getCurrentMode() == mode_standby || getCurrentMode() == mode_cooling) {
			setCurrentMode(mode_set);
		}
		else if(getCurrentMode() == mode_set) {
			setCurrentMode(mode_cooling);
		}
		return ret;
	}

	if(input == Rotate_Increment || input == Rotate_Decrement) {
		tempIsBeeingEdited = 1;
		tempSetWidget->editable.selectable.processInput(tempSetWidget, input, state);
		tempSetWidget->editable.selectable.state = widget_edit;
		tempEditTimeout = HAL_GetTick();
	}
	RE_Rotation_t input2 = RE_Get(RE2);
	if(input2 == Rotate_Increment || input2 == Rotate_Decrement) {
		fanSpeedIsBeeingEdited = 1;
		fanSpeedSetWidget->editable.selectable.processInput(fanSpeedSetWidget, input2, RE2);
		fanSpeedSetWidget->editable.selectable.state = widget_edit;
		fanSpeedEditTimeout = HAL_GetTick();
	}
	if(tempIsBeeingEdited && (HAL_GetTick() - tempEditTimeout > 2000)) {
		tempSetWidget->editable.selectable.state = widget_idle;
		tempIsBeeingEdited = 0;
	}
	if(fanSpeedIsBeeingEdited && (HAL_GetTick() - fanSpeedEditTimeout > 2000)) {
		fanSpeedSetWidget->editable.selectable.state = widget_idle;
		fanSpeedIsBeeingEdited = 0;
	}
	return ret;
}

//returns -1 if processed, -2 if not processed, or next screen
static int temperatureAndFanSpeed_widgetProcessInput(widget_t *widget, RE_Rotation_t input, RE_State_t *state) {
	if(input == Rotate_Nothing)
		return -1;
	uint16_t ui16;
	int8_t inc;
	if(fabs(state->Diff) > 2) {
		inc = widget->editable.big_step;
		if(state->Diff < 0)
			inc = -1 * inc;
	}
	else
		inc = widget->editable.step * state->Diff;
	ui16 = *(uint16_t*)widget->editable.inputData.getData();
	ui16 = ui16 + inc;
	if(ui16 > widget->editable.max_value)
		ui16 = widget->editable.max_value;
	else if(ui16 < widget->editable.min_value)
		ui16 = widget->editable.min_value;
	widget->editable.setData(&ui16);
	return -2;
}
