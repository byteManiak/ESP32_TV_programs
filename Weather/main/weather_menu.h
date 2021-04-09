/*
	Author: Mihai Daniel Ivanescu, Coventry University
 */

#pragma once

#include <vga/vga.h>

#include <ux/submenu.h>
#include <widget/button.h>
#include <widget/textbox.h>

enum Condition
{
	CLOUDY, RAINY, SNOWY, STORMY, RAINY_SUN, CLEAR, UNKNOWN
};

struct Forecast
{
	char date[16];
	int hour;
	int degrees;
	Condition condition;
};

class WeatherMenu : public Submenu
{
public:
	WeatherMenu(VGAExtended *vga);

	void updateSubmenu();

	void drawSubmenu() override;

private:

	enum WeatherAppState {
		WEATHER_APP_STATE_WAITING_WIFI,
		WEATHER_APP_STATE_SHOW_WEATHER,
		WEATHER_APP_STATE_GET_WEATHER,
		WEATHER_APP_STATE_GET_LOCATION
	};

	WeatherAppState state = WEATHER_APP_STATE_WAITING_WIFI;

	void receiveQueueData();

	heap_caps_vector<Forecast*> forecasts;
	char *days[6] = {};
	int hoveredHour = 4;
	int hoveredDay = 2;
	double scrollHourPos = 0;
	double scrollDayPos = 0;

	Button *connectionStatus;
	Textbox *locationTextbox;

	bool forecastValid = false;
};