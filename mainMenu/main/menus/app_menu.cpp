#include <menus/app_menu.h>

#include <esp_ota_ops.h>

#include <util/log.h>
#include <net/http.h>

#include <http/http_handler.h>

static const char *TAG = "appMenu";

AppMenu::AppMenu(VGAExtended *vga, const char *title) : Submenu(vga, title)
{
	appQueueTx = xQueueCreate(16, sizeof(queue_message*));
	attachQueues(httpQueueRx, appQueueTx);

	connectionStatus = heap_caps_malloc_cast<Button>(MALLOC_CAP_PREFERRED);
	new (connectionStatus) Button(vga, "Not connected", vga->xres/2, vga->yres/48);
	widgets.push_back(connectionStatus);

	appList = heap_caps_malloc_cast<List<char*>>(MALLOC_CAP_PREFERRED);
	new (appList) List<char*>(vga, vga->xres/4, vga->yres/8, 8);
	widgets.push_back(appList);

	progressBar = heap_caps_malloc_cast<ProgressBar>(MALLOC_CAP_PREFERRED);
	new (progressBar) ProgressBar(vga);
	progressBar->setVisible(false);
	widgets.push_back(progressBar);

	setFocusedWidget(APP_LIST);

	esp_partition_iterator_t appPartitionIt;
	esp_app_desc_t appInfo = {};
	char appName[32] = {0};

	appPartitionIt = esp_partition_find(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_APP_OTA_0, NULL);
	if (appPartitionIt)
	{
		const esp_partition_t *appPartition = esp_partition_get(appPartitionIt);
		if (appPartition)
		{
			esp_ota_get_partition_description(appPartition, &appInfo);
			esp_partition_read(appPartition, 0x120, appName, 32);
		}
	}

	// Check if there is an app loaded on OTA partition
	if (appInfo.magic_word == ESP_APP_DESC_MAGIC_WORD)
	{
		hasDownloadedApp = true;
		snprintf(downloadedAppName, 44, "Installed: %s", appName);
		appList->addElement(strdup(downloadedAppName));
	}

	esp_partition_iterator_release(appPartitionIt);
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
					connectionStatus->setFillColor(GREEN);

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

				case APP_QUEUE_RX_DOWNLOAD_PERCENT:
				{
					progressBar->setVisible(true);
					progressBar->setPercentage(atof(rxMessage->msg_text));
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
					state = APP_MENU_STATE_DISPLAY_LIST;
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
		case APP_MENU_STATE_LIST_UPDATING:
		case APP_MENU_STATE_DISPLAY_LIST:
		{
			if (isActive)
			{
				setFocusedWidget(APP_LIST);

				// Get the index of the radio station that was selected
				int8_t listElementIndex = appList->getStatus();

				// Refresh the list of apps from the server
				if (isKeyPressed(F5_key))
				{
					appPageNum = 0;
					appPageNumMax = 0xFFFF;
					state = APP_MENU_STATE_REQUEST_LIST;
				}

				// Prevent loading an app or requesting a new list when the list is updating
				if (state != APP_MENU_STATE_LIST_UPDATING)
				{
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

							esp_err_t error = sendQueueData(queueTx, 0, GET_REQUEST_URL());
							if (error == ESP_OK)
							{
								connectionStatus->setText("Downloading app...");
								connectionStatus->setFillColor(CORNFLOWER);

								state = APP_MENU_STATE_DOWNLOAD_BIN;
							}

							FREE_REQUEST_URL();
						}
					}
				}
			}

			break;
		}

		case APP_MENU_STATE_DOWNLOAD_BIN:
		{
			progressBar->setFocused(true);
			break;
		}

		case APP_MENU_STATE_REQUEST_LIST:
		{
			if (!wifiConnected)
			{
				state = APP_MENU_STATE_DISPLAY_LIST;
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
			sendQueueData(queueTx, 0, GET_REQUEST_URL());
			FREE_REQUEST_URL();
			state = APP_MENU_STATE_LIST_UPDATING;
			break;
		}
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
