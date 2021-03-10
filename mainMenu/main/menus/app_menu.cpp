#include <menus/app_menu.h>

#include <util/log.h>
#include <net/http.h>

#include <esp_ota_ops.h>

static const char *TAG = "appMenu";

AppMenu::AppMenu(VGAExtended *vga, const char *title) : Submenu(vga, title)
{
	attachQueues(httpQueueRx, appQueueTx);

	connectionStatus = heap_caps_malloc_cast<Button>(MALLOC_CAP_PREFERRED);
	new (connectionStatus) Button(vga, "Not connected", vga->xres/2, vga->yres/48);
	widgets.push_back(connectionStatus);

	appList = heap_caps_malloc_cast<List<char*>>(MALLOC_CAP_PREFERRED);
	new (appList) List<char*>(vga, vga->xres/4, vga->yres/8, 8);
	widgets.push_back(appList);

	setFocusedWidget(APP_LIST);

	esp_partition_iterator_t appPartition;
	esp_app_desc_t appInfo = {};

	appPartition = esp_partition_find(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_APP_OTA_0, NULL);
	if (appPartition)
	{
		const esp_partition_t *appPartitionInfo = esp_partition_get(appPartition);
		if (appPartitionInfo)
		{
			esp_ota_get_partition_description(appPartitionInfo, &appInfo);
		}
	}

	// Check if there is an app loaded on OTA partition
	if (appInfo.magic_word == ESP_APP_DESC_MAGIC_WORD)
	{
		hasDownloadedApp = true;
		strlcpy(downloadedAppName, appInfo.project_name, 32);
		appList->addElement(strdup(downloadedAppName));
	}

	esp_partition_iterator_release(appPartition);
}

void AppMenu::receiveQueueData()
{
	if (queueRx)
	{
		queue_message *rxMessage;
		if (xQueueReceive(queueRx, &rxMessage, 0) == pdPASS)
		{
			switch(rxMessage->msg_flags)
			{
				case APP_QUEUE_RX_WIFI_CONNECTED:
				{
					connectionStatus->setText("WiFi connected");
					connectionStatus->setFillColor(8);

					wifiConnected = true;
					state = APP_MENU_STATE_REQUEST_LIST;
					break;
				}

				case APP_QUEUE_RX_APP_NAME:
				{
					char *appName = heap_caps_malloc_cast<char>(MALLOC_CAP_DEFAULT, 256);
					strlcpy(appName, rxMessage->msg_text, 256);
					appList->addElement(appName);
					break;
				}

				case APP_QUEUE_RX_FINISHED_OP:
				{
					// Establish if there are anymore "pages" to request
					// from the server when scrolling through the list
					if (appList->getSize() < 16)
					{
						if (appList->getSize() == 0)
							appPageNumMax = appPageNum-1;
						else appPageNumMax = appPageNum;
					}

					setFocusedWidget(APP_LIST);
					state = APP_MENU_STATE_DEFAULT;
					break;
				}
			}

			heap_caps_free(rxMessage);
		}
	}
}

void AppMenu::updateSubmenu()
{
	updateState();

	switch(state)
	{
		case APP_MENU_STATE_DEFAULT:
		{
			// Get the index of the radio station that was selected
			int8_t listElementIndex = appList->getStatus();

			// Get the previous page of stations if PageUp is pressed
			if (isKeyPressed(PgUp_key) && appPageNum > 0)
			{
				appPageNum--;
				state = APP_MENU_STATE_REQUEST_LIST;
			}
			// Get the next page (if any) if PageDown is pressed
			else if (isKeyPressed(PgDown_key) && appPageNum < appPageNumMax)
			{
				appPageNum++;
				state = APP_MENU_STATE_REQUEST_LIST;
			}
			// If an app is selected (e.g. Enter is pressed), request to download this app
			else if (listElementIndex > -1 && appList->getSize() > 0)
			{
				if (hasDownloadedApp && listElementIndex == 0 && appPageNum == 0)
				{
					loadInternalApp();
				}
				else
				{
					// Send out get request with the app id
					MAKE_REQUEST_URL("app=%d", appPageNum*16+listElementIndex-hasDownloadedApp);

					esp_err_t error = sendQueueData(queueTx, HTTP_QUEUE_TX_REQUEST_APP, GET_REQUEST_URL());
					if (error == ESP_OK)
					{
						connectionStatus->setText("Downloading app...");
						connectionStatus->setFillColor(56);

						state = APP_MENU_STATE_DOWNLOAD_BIN;
					}
				}
			}
			break;
		}

		case APP_MENU_STATE_DOWNLOAD_BIN:
		{
			break;
		}

		case APP_MENU_STATE_LOAD_INTERNAL:
		{
			break;
		}

		case APP_MENU_STATE_REQUEST_LIST:
		{
			if (!wifiConnected)
			{
				state = APP_MENU_STATE_DEFAULT;
				return;
			}

			appList->clear();

			int start = 0, end = 15;
			if (hasDownloadedApp)
			{
				end = 14;
				if (appPageNum == 0)
				{
					appList->addElement(strdup(downloadedAppName));
					start = 0;
				}
				else start = -1;
			}

			MAKE_REQUEST_URL("appList=%d,%d", appPageNum*16+start, appPageNum*16+end);
			esp_err_t error = sendQueueData(queueTx, HTTP_QUEUE_TX_REQUEST_APP_LIST, GET_REQUEST_URL());
			if (error == ESP_OK)
			{
				state = APP_MENU_STATE_LIST_UPDATING;

				setFocusedWidget(CONNECTION_STATUS);
			}
			else state = APP_MENU_STATE_DEFAULT;
			break;
		}

		case APP_MENU_STATE_LIST_UPDATING:
			break;
	}
}

void AppMenu::loadInternalApp()
{
	esp_partition_iterator_t appPartition;
	esp_app_desc_t appInfo = {};

	appPartition = esp_partition_find(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_APP_OTA_0, NULL);
	if (appPartition)
	{
		const esp_partition_t *appPartitionInfo = esp_partition_get(appPartition);
		if (appPartitionInfo)
		{
			LOG_INFO("OTA partition label: %s", appPartitionInfo->label);
			esp_ota_get_partition_description(appPartitionInfo, &appInfo);
			esp_ota_set_boot_partition(appPartitionInfo);
			esp_restart();
		}
	}

	// Check if there is an app loaded on OTA partition
	if (appInfo.magic_word == ESP_APP_DESC_MAGIC_WORD)
	{
		hasDownloadedApp = true;
		appList->addElement(appInfo.project_name);
	}

	esp_partition_iterator_release(appPartition);
}