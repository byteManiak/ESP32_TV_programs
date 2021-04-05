#include <http/http_handler.h>

#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_ota_ops.h>

#include <io/sound.h>
#include <memory/alloc.h>
#include <net/http.h>
#include <util/log.h>
#include <util/queues.h>

static const char *TAG = "http_handler";

static char *buf = NULL;
static int contentSize = 0;
static int writtenBytes = 0;
static int lastResponse = 0;

static char url[256] = {};
static char requestType[16] = {};
static int requestValues[2];

QueueHandle_t appQueueTx = NULL;

QueueHandle_t radioQueueTx = NULL;

QueueHandle_t newsQueueTx = NULL;

static void parseURL()
{
	// Get URL that contains the client request
	esp_http_client_get_url(httpClient, url, 256);

	// Get rid of the http://host:port part of the url as we only want the request body
	char *tmp = url + strlen(CONFIG_HTTP_SERVER_URL) + 1;
	// Get the key/type of the request
	char *req = strtok(tmp, "=");
	strcpy(requestType, req);
	// Get the values of the request. Optionally there can be a second value.
	char *val1 = strtok(NULL, ",");
	requestValues[0] = atoi(val1);
	char *val2 = strtok(NULL, ",");
	if (val2) requestValues[1] = atoi(val2);
}

static esp_ota_handle_t ota = 0;
static esp_err_t otaResult = ESP_OK;

esp_err_t httpEventHandler(esp_http_client_event_t *event)
{
	switch(event->event_id)
	{
		case HTTP_EVENT_ON_HEADER:
		{
			LOG_INFO("HTTP header received: %s = %s", event->header_key, event->header_value);

			// Must receive Content-Length header in order to know how much data to read
			if (!strcmp(event->header_key, "Content-Length"))
			{
				// Parse the URL to tell what request to honor
				parseURL();

				contentSize = atoi(event->header_value);

				// In case the input data is parsed as a string, 
				// allocate an extra byte to be able to then NUL-terminate it
				// Also avoid allocating the buffer when downloading a user app as it may not fit in DRAM
				if (strcmp(requestType, "app")) buf = heap_caps_malloc_cast<char>(MALLOC_CAP_PREFERRED, contentSize+1);

				// If we are requesting to download an app, prepare the OTA partition
				else
				{
					esp_partition_iterator_t otaPartitionIt = esp_partition_find(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_APP_OTA_0, NULL);
					const esp_partition_t *otaPartition = esp_partition_get(otaPartitionIt);
					if (!otaPartition) otaResult = ESP_ERR_NOT_FOUND;
					else otaResult = esp_ota_begin(otaPartition, contentSize, &ota);
					esp_partition_iterator_release(otaPartitionIt);
				}
			}
			break;
		}

		case HTTP_EVENT_ON_DATA:
		{
			// Keep reading data as long as it is available
			lastResponse = esp_http_client_get_status_code(httpClient);
			if (lastResponse == 200 && contentSize > 0 && !esp_http_client_is_chunked_response(event->client))
			{
				// If downloading an app, write the OTA data in chunks, not to the HTTP buffer
				if (!strcmp(requestType, "app") && otaResult == ESP_OK)
				{
					otaResult = esp_ota_write(ota, event->data, event->data_len);
					writtenBytes += event->data_len;
					if (contentSize > 0 )
					{
						char percentage[5] = {0};
						snprintf(percentage, 5, "%.2f", writtenBytes/double(contentSize));
						sendQueueData(appQueueTx, APP_QUEUE_RX_DOWNLOAD_PERCENT, percentage, portMAX_DELAY);
					}
				}
				else
				{
					memcpy(&buf[writtenBytes], event->data, event->data_len);
					writtenBytes += event->data_len;
				}
			}
			break;
		}

		case HTTP_EVENT_ON_FINISH:
		{
			// NUL-terminate the buffer when not downloading an app
			if (strcmp(requestType, "app")) buf[contentSize] = '\0';

			// Get list of radio stations
			if (!strcmp(requestType, "radio"))
			{
				// Get the first station name from the reply
				char *stationName = strtok(buf, ";");
				// Keep reading station names until the buffer is empty
				while(stationName)
				{
					// Send the radio station name to the radio menu
					sendQueueData(radioQueueTx, RADIO_QUEUE_RX_RADIO_STATION, stationName, portMAX_DELAY);
					// Get the next radio station name
					stationName = strtok(NULL, ";");
				}
				// Signal the radio menu that no more radio stations are being sent
				sendQueueData(radioQueueTx, RADIO_QUEUE_RX_FINISHED_OP, NULL, portMAX_DELAY);
			}

			// Get URL of the selected radio station from the server
			else if (!strcmp(requestType, "station"))
			{
				char *audioURL = heap_caps_malloc_cast<char>(MALLOC_CAP_PREFERRED, 256);
				strlcpy(audioURL, buf, 256);
				// Send the URL to the audio stream task to play
				xTaskCreatePinnedToCore(audioDispatchTask, "audioTask", 3072, audioURL, tskIDLE_PRIORITY, NULL, 1);
			}

			// Get list of user apps
			else if (!strcmp(requestType, "appList"))
			{
				// Get the first app name from the reply
				char *appName = strtok(buf, ";");
				// Keep reading app names until the buffer is empty
				while(appName)
				{
					// Send the app name to the app menu
					sendQueueData(appQueueTx, APP_QUEUE_RX_APP_NAME, appName, portMAX_DELAY);
					// Get the next app name
					appName = strtok(NULL, ";");
				}
				// Signal the app menu that no more apps are being sent
				sendQueueData(appQueueTx, APP_QUEUE_RX_FINISHED_OP, NULL, portMAX_DELAY);
			}

			// Get list of news headlines
			else if (!strcmp(requestType, "news"))
			{
				char *headline;
				// Get the title of the RSS feed from the reply if requesting the first page from it
				if (requestValues[1] == 0)
				{
					char *title = strtok(buf, ";");
					sendQueueData(newsQueueTx, NEWS_QUEUE_RX_RSS_FEED_NAME, title, portMAX_DELAY);
					headline = strtok(NULL, ";");
				}
				else headline = strtok(buf, ";");

				while(headline)
				{
					// Send the headline to the news menu
					sendQueueData(newsQueueTx, NEWS_QUEUE_RX_HEADLINE_NAME, headline, portMAX_DELAY);
					// Get the next headline
					headline = strtok(NULL, ";");
				}

				// Signal the news menu that no more headlines are being sent
				sendQueueData(newsQueueTx, NEWS_QUEUE_RX_FINISHED_OP, NULL, portMAX_DELAY);
			}

			// Load the downloaded app
			else if (!strcmp(requestType, "app") && otaResult == ESP_OK)
			{
				esp_partition_iterator_t otaPartitionIt = esp_partition_find(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_APP_OTA_0, NULL);
				const esp_partition_t *otaPartition = esp_partition_get(otaPartitionIt);

				if (otaPartition)
				{
					esp_ota_end(ota);
					esp_ota_set_boot_partition(otaPartition);
					esp_restart();
				}
			}

			writtenBytes = 0;
			heap_caps_free(buf);

			break;
		}

		default: break;
	}
	return ESP_OK;
}
