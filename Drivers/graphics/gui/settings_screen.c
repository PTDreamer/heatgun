/*
 * debug_screen.c
 *
 *  Created on: Aug 2, 2017
 *      Author: jose
 */

#include "settings_screen.h"
#include "../../../../Src/settings.h"
#include "../ssd1306.h"
#include "oled.h"
#include "../../../Src/gun.h"

static widget_t *combo = NULL;

static uint16_t CONTRAST = 0;
static uint16_t coolDownMaxTime = 0;
static uint16_t coolDownTemp = 0;

static void * getContrast_() {
	CONTRAST = getContrast();
	return &CONTRAST;
}
static void setContrast_(uint16_t *val) {
	CONTRAST = *val;
	setContrast(CONTRAST);
}
static int saveContrast(widget_t *w) {
	systemSettings.contrast = CONTRAST;
	saveSettings();
	return screen_main;
}
static int cancelContrast(widget_t *w) {
	setContrast(systemSettings.contrast);
	return screen_main;
}

////
static int saveCoolDown(widget_t *w) {
	systemSettings.coolDown = currentCoolDownSettings;
	saveSettings();
	return screen_main;
}
static int cancelCoolDown(widget_t *w) {
	currentCoolDownSettings = systemSettings.coolDown;
	return screen_main;
}
static void setCoolDownMaxTime(uint16_t *val) {
	coolDownMaxTime = *val;
	currentCoolDownSettings.maxTime = coolDownMaxTime;
}

static void * getCoolDownMaxTime() {
	coolDownMaxTime = currentCoolDownSettings.maxTime;
	return &coolDownMaxTime;
}

static void setCoolDownTemp(uint16_t *val) {
	coolDownTemp = *val;
	currentCoolDownSettings.coolDownTemperature = coolDownTemp;
}
static void * getCoolDownTemp() {
	coolDownTemp = currentCoolDownSettings.coolDownTemperature;
	return &coolDownTemp;
}
static void settings_screen_init(screen_t *scr) {
	UG_FontSetHSpace(0);
	UG_FontSetVSpace(0);
	default_init(scr);
	scr->current_widget = combo;
	scr->current_widget->comboBoxWidget.selectable.state = widget_selected;
}

void settings_screen_setup(screen_t *scr) {

	///settings combobox
	scr->draw = &default_screenDraw;
	scr->processInput = &default_screenProcessInput;
	scr->init = &settings_screen_init;
	scr->update = &default_screenUpdate;
	widget_t *widget = screen_addWidget(scr);
	widgetDefaultsInit(widget, widget_label);
	char *s = "Settings";
	strcpy(widget->displayString, s);
	widget->posX = 10;
	widget->posY = 0;
	widget->font_size = &FONT_8X14;
	widget->reservedChars = 8;
	widget->draw = &default_widgetDraw;
	widget = screen_addWidget(scr);
	widget->posX = 0;
	widgetDefaultsInit(widget, widget_combo);
	widget->posY = 17;
	widget->font_size = &FONT_6X8;
	comboAddItem(widget, "SCREEN", screen_edit_contrast);
	comboAddItem(widget, "SLEEP", screen_edit_sleep);
	comboAddItem(widget, "CALIBRATION", screen_edit_calibration_wait);
	comboAddItem(widget, "EXIT", screen_main);
	combo = widget;

	screen_t *sc = oled_addScreen(screen_edit_contrast);
	sc->draw = &default_screenDraw;
	sc->processInput = &default_screenProcessInput;
	sc->init = &default_init;
	sc->update = &default_screenUpdate;
	widget_t *w = screen_addWidget(sc);

	widgetDefaultsInit(w, widget_label);
	s = "CONTRAST";
	strcpy(w->displayString, s);
	w->posX = 50;
	w->posY = 0;
	w->font_size = &FONT_8X14;
	w->reservedChars = 8;

	w = screen_addWidget(sc);
	widgetDefaultsInit(w, widget_label);
	s = "Value:";
	strcpy(w->displayString, s);
	w->posX = 30;
	w->posY = 17;
	w->font_size = &FONT_6X8;
	w->reservedChars = 6;

	w = screen_addWidget(sc);
	widgetDefaultsInit(w, widget_editable);
	w->posX = 70;
	w->posY = 17;
	w->font_size = &FONT_6X8;
	w->editable.inputData.getData = &getContrast_;
	w->editable.inputData.number_of_dec = 0;
	w->editable.inputData.type = field_uinteger16;
	w->editable.big_step = 10;
	w->editable.step = 1;
	w->editable.selectable.tab = 0;
	w->editable.setData = (void (*)(void *))&setContrast_;
	w->editable.max_value = 255;
	w->reservedChars = 3;

	w = screen_addWidget(sc);
	widgetDefaultsInit(w, widget_button);
	w->font_size = &FONT_6X8;
	w->posX = 2;
	w->posY = 56;
	s = "SAVE";
	strcpy(w->displayString, s);
	w->reservedChars = 4;
	w->buttonWidget.selectable.tab = 1;
	w->buttonWidget.action = &saveContrast;
	w = screen_addWidget(sc);
	widgetDefaultsInit(w, widget_button);
	w->font_size = &FONT_6X8;
	w->posX = 90;
	w->posY = 56;
	s = "CANCEL";
	strcpy(w->displayString, s);
	w->reservedChars = 6;
	w->buttonWidget.selectable.tab = 2;
	w->buttonWidget.action = &cancelContrast;

	//Screen edit Sleep
	sc = oled_addScreen(screen_edit_sleep);
	sc->draw = &default_screenDraw;
	sc->processInput = &default_screenProcessInput;
	sc->init = &default_init;
	sc->update = &default_screenUpdate;
	w = screen_addWidget(sc);

	widgetDefaultsInit(w, widget_label);
	s = "SLEEP & STANDBY";
	strcpy(w->displayString, s);
	w->posX = 0;
	w->posY = 0;
	w->font_size = &FONT_8X14;
	w->reservedChars = 3;

	w = screen_addWidget(sc);
	widgetDefaultsInit(w, widget_label);
	s = "Sleep Time(s):";
	strcpy(w->displayString, s);
	w->posX = 2;
	w->posY = 17;
	w->font_size = &FONT_6X8;
	w->reservedChars = 3;
	//
	w = screen_addWidget(sc);
	widgetDefaultsInit(w, widget_editable);
	w->posX = 100;
	w->posY = 17;
	w->font_size = &FONT_6X8;
	w->editable.inputData.getData = &getCoolDownMaxTime;
	w->editable.inputData.number_of_dec = 0;
	w->editable.inputData.type = field_uinteger16;
	w->editable.big_step = 10;
	w->editable.step = 1;
	w->editable.selectable.tab = 0;
	w->editable.setData = (void (*)(void *))&setCoolDownMaxTime;
	w->editable.max_value = 999;
	w->reservedChars = 3;

	w = screen_addWidget(sc);
	widgetDefaultsInit(w, widget_label);
	s = "Sleep Temp(C):";
	strcpy(w->displayString, s);
	w->posX = 2;
	w->posY = 29;
	w->font_size = &FONT_6X8;
	w->reservedChars = 3;

	w = screen_addWidget(sc);
	widgetDefaultsInit(w, widget_editable);
	w->posX = 100;
	w->posY = 29;
	w->font_size = &FONT_6X8;
	w->editable.inputData.getData = &getCoolDownTemp;
	w->editable.inputData.number_of_dec = 0;
	w->editable.inputData.type = field_uinteger16;
	w->editable.big_step = 10;
	w->editable.step = 1;
	w->editable.selectable.tab = 1;
	w->editable.setData = (void (*)(void *))&setCoolDownTemp;
	w->reservedChars = 3;
	w->editable.max_value = 450;

	w = screen_addWidget(sc);
	widgetDefaultsInit(w, widget_label);
	s = "StandBy Time(m):";
	strcpy(w->displayString, s);
	w->posX = 2;
	w->posY = 41;
	w->font_size = &FONT_6X8;
	w->reservedChars = 3;

	w = screen_addWidget(sc);
	widgetDefaultsInit(w, widget_button);
	w->font_size = &FONT_6X8;
	w->posX = 2;
	w->posY = 56;
	s = "SAVE";
	strcpy(w->displayString, s);
	w->reservedChars = 4;
	w->buttonWidget.selectable.tab = 3;
	w->buttonWidget.action = &saveCoolDown;
	w = screen_addWidget(sc);
	widgetDefaultsInit(w, widget_button);
	w->font_size = &FONT_6X8;
	w->posX = 90;
	w->posY = 56;
	s = "CANCEL";
	strcpy(w->displayString, s);
	w->reservedChars = 6;
	w->buttonWidget.selectable.tab = 4;
	w->buttonWidget.action = &cancelCoolDown;
	}

