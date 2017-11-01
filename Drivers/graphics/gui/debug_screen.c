/*
 * debug_screen.c
 *
 *  Created on: Aug 2, 2017
 *      Author: jose
 */

#include "debug_screen.h"
#include "adc_global.h"
#include "tempsensors.h"

uint16_t temp;
uint16_t test;
static TIM_HandleTypeDef *htim4;
static uint16_t pwm = 1;

void setPWM_tim(TIM_HandleTypeDef *tim){
	htim4 = tim;
}
static void *dGetPWM() {
	return &pwm;
}
static void dSetPWM(uint16_t *value) {
	pwm = *value;
	 // __HAL_TIM_SET_COMPARE(htim4, TIM_CHANNEL_3, pwm);
}
void * testwGet() {
	return &test;
}

void testwSet(uint16_t *value) {
	test = *value;
}

static void * debug_screen_getAmbTemp() {
	temp = readColdJunctionSensorTemp_mC() / 100;
	return &temp;
}

static void * debug_screen_getADC1_1() {
	temp = iron_temp_adc_avg;
	return &temp;
	uint32_t ac = 0;
	for(uint8_t x = 0; x < sizeof(adc_measures)/ sizeof(adc_measures[0]); ++x)
	{
		ac += adc_measures[x].termocouple;
	}
	ac = ac / (sizeof(adc_measures)/ sizeof(adc_measures[0]));
	temp = ac;
	return &temp;
}
static void * debug_screen_getADC1_3() {
	uint32_t ac = 0;
	for(uint8_t x = 0; x < sizeof(adc_measures)/ sizeof(adc_measures[0]); ++x)
	{
		ac += adc_measures[x].ntc;
	}
	ac = ac / (sizeof(adc_measures)/ sizeof(adc_measures[0]));
	temp = ac;
	return &temp;
}
static void * debug_screen_getADC2_2() {
	uint32_t ac = 0;
	for(uint8_t x = 0; x < sizeof(adc_measures)/ sizeof(adc_measures[0]); ++x)
	{
	}
	ac = ac / (sizeof(adc_measures)/ sizeof(adc_measures[0]));
	temp = ac;
	return &temp;
}

static void debug_screen_init(screen_t *scr) {
	UG_FontSetHSpace(0);
	UG_FontSetVSpace(0);
	default_init(scr);
}

void debug_screen_setup(screen_t *scr) {
	scr->draw = &default_screenDraw;
	scr->processInput = &default_screenProcessInput;
	scr->init = &debug_screen_init;
	scr->update = &default_screenUpdate;

	widget_t *widget = screen_addWidget(scr);
	widgetDefaultsInit(widget, widget_label);
	char *s = "ADC1_1";
	strcpy(widget->displayString, s);
	widget->posX = 1;
	widget->posY = 0;
	widget->font_size = &FONT_8X14;
	widget->reservedChars = 6;
	widget->draw = &default_widgetDraw;
	widget = screen_addWidget(scr);
	widgetDefaultsInit(widget, widget_label);
	s = "ADC1_3";
	strcpy(widget->displayString, s);
	widget->posX = 1;
	widget->posY = 16;
	widget->font_size = &FONT_6X8;
	widget->reservedChars = 6;
	widget->draw = &default_widgetDraw;
	widget = screen_addWidget(scr);
	widgetDefaultsInit(widget, widget_label);
	s = "ADC2_4";
	strcpy(widget->displayString, s);
	widget->posX = 1;
	widget->posY = 27;
	widget->font_size = &FONT_6X8;
	widget->reservedChars = 6;
	widget->draw = &default_widgetDraw;
	widget = screen_addWidget(scr);
	widgetDefaultsInit(widget, widget_label);
	s = "ADC2_2";
	strcpy(widget->displayString, s);
	widget->posX = 1;
	widget->posY = 38;
	widget->font_size = &FONT_6X8;
	widget->reservedChars = 6;
	widget->draw = &default_widgetDraw;

	//ADC_1_1 display
	widget = screen_addWidget(scr);
	widgetDefaultsInit(widget, widget_display);
	widget->posX = 40;
	widget->posY = 0;
	widget->font_size = &FONT_8X14;
	widget->displayWidget.getData = &debug_screen_getADC1_1;
	widget->displayWidget.number_of_dec = 0;
	widget->displayWidget.type = field_uinteger16;
	widget->reservedChars = 5;

	//ADC_1_3 display
	widget = screen_addWidget(scr);
	widgetDefaultsInit(widget, widget_display);
	widget->posX = 40;
	widget->posY = 16;
	widget->font_size = &FONT_6X8;
	widget->displayWidget.getData = &debug_screen_getADC1_3;
	widget->displayWidget.number_of_dec = 0;
	widget->displayWidget.type = field_uinteger16;
	widget->reservedChars = 5;

	//ADC_2_2 display
	widget = screen_addWidget(scr);
	widgetDefaultsInit(widget, widget_display);
	widget->posX = 40;
	widget->posY = 27;
	widget->font_size = &FONT_6X8;
	widget->displayWidget.getData = &debug_screen_getADC2_2;
	widget->displayWidget.number_of_dec = 0;
	widget->displayWidget.type = field_uinteger16;
	widget->reservedChars = 5;


	widget = screen_addWidget(scr);
	widgetDefaultsInit(widget, widget_display);
	widget->posX = 100;
	widget->posY = 16;
	widget->font_size = &FONT_6X8;
	widget->displayWidget.getData = &debug_screen_getAmbTemp;
	widget->displayWidget.number_of_dec = 1;
	widget->displayWidget.type = field_uinteger16;
	widget->reservedChars = 4;
	// tip temperature setpoint

	widget = screen_addWidget(scr);
	widgetDefaultsInit(widget, widget_editable);
	widget->posX = 100;
	widget->posY = 27;
	widget->font_size = &FONT_6X8;
	widget->editable.inputData.getData = &testwGet;
	widget->editable.inputData.number_of_dec = 0;
	widget->editable.inputData.type = field_uinteger16;
	widget->editable.big_step = 10;
	widget->editable.step = 1;
	widget->editable.selectable.tab = 0;
	widget->editable.setData = (void (*)(void *))&testwSet;
	widget->reservedChars = 4;
	widget->editable.selectable.state = widget_edit;

	widget = screen_addWidget(scr);
	widgetDefaultsInit(widget, widget_editable);
	widget->posX = 100;
	widget->posY = 38;
	widget->font_size = &FONT_6X8;
	widget->editable.inputData.getData = &dGetPWM;
	widget->editable.inputData.number_of_dec = 0;
	widget->editable.inputData.type = field_uinteger16;
	widget->editable.big_step = 10;
	widget->editable.step = 1;
	widget->editable.selectable.tab = 1;
	widget->editable.setData = (void (*)(void *))&dSetPWM;
	widget->reservedChars = 3;


}

