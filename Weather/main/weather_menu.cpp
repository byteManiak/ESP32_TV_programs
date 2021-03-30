#include <weather_menu.h>

#include <net/http.h>
#include <util/log.h>

#include <http_handler.h>

#include <images/weather.h>

static const char *TAG = "weather";

WeatherMenu::WeatherMenu(VGAExtended *vga) : Submenu(vga, "")
{
	weatherQueueTx = xQueueCreate(16, sizeof(queue_message*));

	setActiveSubmenu(true);

	connectionStatus = heap_caps_malloc_cast<Button>(MALLOC_CAP_PREFERRED);
	new (connectionStatus) Button(vga, "Waiting for WiFi", vga->xres/2, 2);
	connectionStatus->setVisible(true);
	widgets.push_back(connectionStatus);

	setFocusedWidget(0);
}

void WeatherMenu::receiveQueueData()
{
	if (wifiQueueTx)
	{
		queue_message *rxMessage;
		if (xQueueReceive(wifiQueueTx, &rxMessage, 0) == pdPASS)
		{
			switch(rxMessage->msg_flags)
			{
				case WIFI_QUEUE_RX_CONNECTED:
				{
					state = WEATHER_APP_STATE_GET_LOCATION;
					connectionStatus->setText("Waiting for input location");
					connectionStatus->setFillColor(PURPLE);

					break;
				}

				case WIFI_QUEUE_RX_HTTP_SERVER_ERROR:
				{
					state = WEATHER_APP_STATE_GET_LOCATION;
					connectionStatus->setText("Server not responding. Try again.");
					connectionStatus->setFillColor(WINE2);

					break;
				}
			}

			heap_caps_free(rxMessage);
		}
	}

	if (weatherQueueTx)
	{
		queue_message *rxMessage;
		if (xQueueReceive(weatherQueueTx, &rxMessage, 0) == pdPASS)
		{
			switch(rxMessage->msg_flags)
			{
				case WEATHER_QUEUE_RX_FORECAST:
				{
					Forecast *currentForecast = heap_caps_malloc_cast<Forecast>(MALLOC_CAP_PREFERRED);

					char *dateStr = strtok(rxMessage->msg_text, " ");
					char *hourStr = strtok(NULL, " ");
					char *tempStr = strtok(NULL, " ");
					char *condStr = strtok(NULL, ";");

					strlcpy(currentForecast->date, dateStr, 16);
					currentForecast->hour = atoi(hourStr);
					currentForecast->degrees = atoi(tempStr);
					if (strcasestr(condStr, "clear")) currentForecast->condition = CLEAR;
					else if (strcasestr(condStr, "clouds")) currentForecast->condition = CLOUDY;
					else if (strcasestr(condStr, "snow")) currentForecast->condition = SNOWY;
					else if (strcasestr(condStr, "storm")) currentForecast->condition = STORMY;
					else if (strcasestr(condStr, "rain")) currentForecast->condition = RAINY;
					else currentForecast->condition = UNKNOWN;

					forecasts.push_back(currentForecast);
				
					break;
				}

				case WEATHER_QUEUE_RX_FINISHED_OP:
				{
					for(auto &day : days) day = NULL;

					if (forecasts.size() > 0)
					{
						days[0] = forecasts[0]->date;
						int currentDay = 0;

						for(auto i : forecasts)
						{
							if (strcmp(i->date, days[currentDay]))
							{
								currentDay++;
								days[currentDay] = i->date;
							}
						}

						forecastValid = true;

						state = WEATHER_APP_STATE_SHOW_WEATHER;
					}
					else
					{
						connectionStatus->setText("Invalid location entered");
						connectionStatus->setFillColor(WINE2);

						state = WEATHER_APP_STATE_GET_LOCATION;
					}

					break;
				}
			}

			heap_caps_free(rxMessage);
		}
	}
}

void WeatherMenu::updateSubmenu()
{
	updateState();

	smoothLerp(scrollHourPos, hoveredHour);
	smoothLerp(scrollDayPos, hoveredDay);

	switch(state)
	{
		case WEATHER_APP_STATE_WAITING_WIFI:
		{
			break;
		}

		case WEATHER_APP_STATE_GET_LOCATION:
		{
			locationTextbox = heap_caps_malloc_cast<Textbox>(MALLOC_CAP_PREFERRED);
			new (locationTextbox) Textbox(vga, "Insert location:");
			widgets.push_back(locationTextbox);

			setFocusedWidget(1);

			state = WEATHER_APP_STATE_GET_WEATHER;

			break;
		}

		case WEATHER_APP_STATE_GET_WEATHER:
		{
			int8_t status = locationTextbox->getStatus();
			if (status == TEXTBOX_ENTER)
			{
				for(auto i : forecasts) heap_caps_free(i);
				forecasts.clear();

				MAKE_REQUEST_URL("weather=%s", locationTextbox->getText());
				sendQueueData(httpQueueRx, HTTP_QUEUE_TX_REQUEST_WEATHER, GET_REQUEST_URL());
				FREE_REQUEST_URL();

				connectionStatus->setText(locationTextbox->getText());

				widgets.pop_back();
				heap_caps_free(locationTextbox);

				setFocusedWidget(0);

				locationTextbox = NULL;

				forecastValid = false;
				state = WEATHER_APP_STATE_SHOW_WEATHER;
			}
		
			break;
		}

		case WEATHER_APP_STATE_SHOW_WEATHER:
		{
			if (isKeyPressed(Up_key)) hoveredHour = getPrevInt(hoveredHour, 8);
			if (isKeyPressed(Down_key)) hoveredHour = getNextInt(hoveredHour, 8);

			if (isKeyPressed(Left_key)) hoveredDay = getPrevInt(hoveredDay, 6);
			if (isKeyPressed(Right_key)) hoveredDay = getNextInt(hoveredDay, 6);

			bool buttonPressed = connectionStatus->getStatus();
			if (buttonPressed) state = WEATHER_APP_STATE_GET_LOCATION;
			break;
		}

		default: break;
	}
}

void WeatherMenu::drawSubmenu()
{
	if (forecasts.size() > 0)
	{
		char *lastDay = forecasts[0]->date;
		int dayCount = 0;
		for(auto i : forecasts)
		{
			if (strcmp(lastDay, i->date))
			{
				lastDay = i->date;
				dayCount++;
			}

			int hour = i->hour/3;
			float hourDiff = scrollHourPos-hour;
			float yHour = vga->yres/2 - hourDiff*64;
			float xDay = dayCount * vga->xres/6 + vga->xres/2 - scrollDayPos*vga->xres/6;
			float yScale = 1.75 - abs(hourDiff/1.25);
			//if (yScale < 0.5) yScale = 0.5;

			vga->drawSprite(weather, i->condition, xDay, yHour, yScale);
			vga->setCursor(xDay - 10, yHour - vga->font->charHeight*2 - 6);
			vga->print(i->hour);
			vga->println(":00");

			vga->print(i->degrees);
			vga->print("C");
		}

		if (forecastValid)
			for(int i = 0; i < 6; i++)
				if (days[i]) vga->printBox(days[i], vga->xres/2 + vga->xres/6*i+4 - scrollDayPos*vga->xres/6 - 12, vga->yres-20, WHITE, WHITE, MIDNIGHT_GREEN);
	}

	Submenu::drawSubmenu();
}