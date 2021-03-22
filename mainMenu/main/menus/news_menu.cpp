#include <menus/news_menu.h>

#include <util/log.h>
#include <net/http.h>

#include <http/http_handler.h>

static const char *TAG = "newsMenu";

NewsMenu::NewsMenu(VGAExtended *vga, const char *title) : Submenu(vga, title)
{
	newsQueueTx = xQueueCreate(16, sizeof(queue_message*));
	attachQueues(httpQueueRx, newsQueueTx);

	connectionStatus = heap_caps_malloc_cast<Button>(MALLOC_CAP_PREFERRED);
	new (connectionStatus) Button(vga, "Not connected", vga->xres/2, vga->yres/48);
	widgets.push_back(connectionStatus);

	newsList = heap_caps_malloc_cast<List<char*>>(MALLOC_CAP_PREFERRED);
	new (newsList) List<char*>(vga, vga->xres/4, vga->yres/8, 16, false, true);
	widgets.push_back(newsList);

	setFocusedWidget(NEWS_LIST);
}

void NewsMenu::receiveQueueData()
{
	if (queueRx)
	{
		queue_message *rxMessage;
		if (xQueueReceive(queueRx, &rxMessage, 0) == pdPASS)
		{
			switch(rxMessage->msg_flags)
			{
				case NEWS_QUEUE_RX_WIFI_CONNECTED:
				{
					connectionStatus->setText("WiFi connected");
					connectionStatus->setFillColor(GREEN);

					state = NEWS_MENU_STATE_REQUEST_LIST;
					break;
				}

				case NEWS_QUEUE_RX_RSS_FEED_NAME:
				{
					connectionStatus->setText(rxMessage->msg_text);

					break;
				}

				case NEWS_QUEUE_RX_HEADLINE_NAME:
				{
					char *newsHeadline = heap_caps_malloc_cast<char>(MALLOC_CAP_DEFAULT, 256);
					strlcpy(newsHeadline, rxMessage->msg_text, 256);
					newsList->addElement(newsHeadline);

					break;
				}

				case NEWS_QUEUE_RX_FINISHED_OP:
				{
					// Establish if there are anymore "pages" to request
					// from the server when scrolling through the list
					if (newsList->getSize() < 16)
					{
						if (newsList->getSize() == 0)
							rssFeedPageNumMax = rssFeedPageNum-1;
						else rssFeedPageNumMax = rssFeedPageNum;
					}

					connectionStatus->setFillColor(GREEN);
					state = NEWS_MENU_STATE_DISPLAY_LIST;
					break;
				}

				case NEWS_QUEUE_RX_HTTP_SERVER_ERROR:
				{
					connectionStatus->setText("Server not responding");
					connectionStatus->setFillColor(RED);
					break;
				}
			}

			heap_caps_free(rxMessage);
		}
	}
}

void NewsMenu::updateSubmenu()
{
	updateState();

	switch(state)
	{
		// Wait until a WiFi connection has been established
		case NEWS_MENU_STATE_WAITING_WIFI:
		{
			connectionStatus->setText("Waiting for WiFi");
			connectionStatus->setFillColor(CORNFLOWER);
			break;
		}

		// Send out a request to the HTTP client to fetch a page from the list of radio stations
		case NEWS_MENU_STATE_REQUEST_LIST:
		{
			MAKE_REQUEST_URL("news=%d,%d", rssFeedIndex, rssFeedPageNum);

			esp_err_t error = sendQueueData(queueTx, 0, GET_REQUEST_URL());
			if (error == ESP_OK) newsList->clear();

			FREE_REQUEST_URL();
			state = NEWS_MENU_STATE_LIST_UPDATING;
			break;
		}

		case NEWS_MENU_STATE_LIST_UPDATING:
		case NEWS_MENU_STATE_DISPLAY_LIST:
		{
			if (isActive)
			{
				setFocusedWidget(NEWS_LIST);

				// Refresh the list of news headlines from the server
				if (isKeyPressed(F5_key))
				{
					rssFeedPageNum = 0;
					rssFeedPageNumMax = 0xFFFF;
					state = NEWS_MENU_STATE_REQUEST_LIST;
				}

				// Prevent requesting a new list when the list is updating
				if (state != NEWS_MENU_STATE_LIST_UPDATING)
				{
					// Get the previous page of stations if PageUp is pressed
					if (isKeyPressed(PgUp_key) && rssFeedPageNum > 0)
					{
						rssFeedPageNum--;
						state = NEWS_MENU_STATE_REQUEST_LIST;
					}
					// Get the next page (if any) if PageDown is pressed
					else if (isKeyPressed(PgDown_key) && rssFeedPageNum < rssFeedPageNumMax)
					{
						rssFeedPageNum++;
						state = NEWS_MENU_STATE_REQUEST_LIST;
					}
					// If a radio station is selected (e.g. Enter is pressed), request to play this station
					else if (isKeyPressed(Left_key) || isKeyPressed(Right_key))
					{
						rssFeedPageNum = 0;
						rssFeedPageNumMax = 0xFFFF;
						if (isKeyPressed(Left_key) && rssFeedIndex > 0) rssFeedIndex--;
						else if(isKeyPressed(Right_key)) rssFeedIndex++;

						state = NEWS_MENU_STATE_REQUEST_LIST;
					}
				}
			}

			break;
		}
	}
}
