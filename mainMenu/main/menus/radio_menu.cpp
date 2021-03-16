#include <menus/radio_menu.h>

#include <net/http.h>

#include <http/http_handler.h>

RadioMenu::RadioMenu(VGAExtended *vga, const char *title) : Submenu(vga, title)
{
	// The radio menu sends out requests to the HTTP client
	// and receives replies through the radio TX queue
	attachQueues(httpQueueRx, radioQueueTx);

	connectionStatus = heap_caps_malloc_cast<Button>(MALLOC_CAP_PREFERRED);
	new (connectionStatus) Button(vga, "Not connected", vga->xres/2, vga->yres/48);
	widgets.push_back(connectionStatus);

	radioStationList = heap_caps_malloc_cast<List<char*>>(MALLOC_CAP_PREFERRED);
	new (radioStationList) List<char*>(vga, vga->xres/4, vga->yres/8, 18);
	widgets.push_back(radioStationList);

	setFocusedWidget(CONNECTION_STATUS);
}

void RadioMenu::receiveQueueData()
{
	if (queueRx)
	{
		queue_message *rxMessage;
		if (xQueueReceive(queueRx, &rxMessage, 0) == pdPASS)
		{
			switch(rxMessage->msg_flags)
			{
				// Wait for a WiFi connection before starting the radio list event
				case RADIO_QUEUE_RX_WIFI_CONNECTED:
				{
					connectionStatus->setText("WiFi connected");
					connectionStatus->setFillColor(GREEN);

					state = RADIO_MENU_STATE_REQUEST_LIST;
					break;
				}

				// Receive a requested radio station name from the server
				case RADIO_QUEUE_RX_RADIO_STATION:
				{
					char *radioStation = heap_caps_malloc_cast<char>(MALLOC_CAP_DEFAULT, 256);
					strlcpy(radioStation, rxMessage->msg_text, 256);
					radioStationList->addElement(radioStation);
					break;
				}

				// Event that signals that there are no more station names to read from the server
				case RADIO_QUEUE_RX_FINISHED_OP:
				{
					// Establish if there are anymore "pages" to request
					// from the server when scrolling through the list
					if (radioStationList->getSize() < 16)
					{
						if (radioStationList->getSize() == 0)
							radioPageNumMax = radioPageNum-1;
						else radioPageNumMax = radioPageNum;
					}
					state = RADIO_MENU_STATE_DISPLAY_LIST;
					break;
				}
			}

			heap_caps_free(rxMessage);
		}
	}
}

void RadioMenu::updateSubmenu()
{
	updateState();

	switch(state)
	{
		// Wait until a WiFi connection has been established
		case RADIO_MENU_STATE_WAITING_WIFI:
		{
			connectionStatus->setText("Waiting for WiFi");
			connectionStatus->setFillColor(CORNFLOWER);
			break;
		}

		case RADIO_MENU_STATE_DISPLAY_LIST:
		{
			setFocusedWidget(RADIO_LIST);

			// Get the index of the radio station that was selected
			int8_t listElementIndex = radioStationList->getStatus();

			// Refresh the list of radio station from the server
			if (isKeyPressed(F5_key))
			{
				radioPageNum = 0;
				radioPageNumMax = 0xFFFF;
				state = RADIO_MENU_STATE_REQUEST_LIST;
			}

			// Get the previous page of stations if PageUp is pressed
			if (isKeyPressed(PgUp_key) && radioPageNum > 0)
			{
				radioPageNum--;
				state = RADIO_MENU_STATE_REQUEST_LIST;
			}
			// Get the next page (if any) if PageDown is pressed
			else if (isKeyPressed(PgDown_key) && radioPageNum < radioPageNumMax)
			{
				radioPageNum++;
				state = RADIO_MENU_STATE_REQUEST_LIST;
			}
			// If a radio station is selected (e.g. Enter is pressed), request to play this station
			else if (listElementIndex > -1 && radioStationList->getSize() > 0)
			{
				// Send out get request with the station id
				MAKE_REQUEST_URL("station=%d", radioPageNum*16+listElementIndex);

				esp_err_t error = sendQueueData(queueTx, HTTP_QUEUE_TX_REQUEST_RADIO_STATION, GET_REQUEST_URL());
				if (error == ESP_OK)
				{
					char currentStation[64] = "On air: ";
					strlcpy(currentStation+8, radioStationList->getElement(), 48);
					connectionStatus->setText(currentStation);
				}
			}
			break;
		}

		// Send out a request to the HTTP client to fetch a page from the list of radio stations
		case RADIO_MENU_STATE_REQUEST_LIST:
		{
			MAKE_REQUEST_URL("radio=%d,%d", radioPageNum*16, radioPageNum*16+15);

			esp_err_t error = sendQueueData(queueTx, HTTP_QUEUE_TX_REQUEST_RADIO_LIST, GET_REQUEST_URL());
			if (error == ESP_OK)
			{
				radioStationList->clear();
				setFocusedWidget(RADIO_LIST);
			}
			state = RADIO_MENU_STATE_DISPLAY_LIST;
			break;
		}

		// Wait for a reply from the server before changing the list
		case RADIO_MENU_STATE_LIST_UPDATING:
		{
			break;
		}
	}
}
