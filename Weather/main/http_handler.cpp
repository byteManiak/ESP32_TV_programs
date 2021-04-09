/*
	Author: Mihai Daniel Ivanescu, Coventry University
 */

#include <http_handler.h>

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

QueueHandle_t weatherQueueTx = NULL;

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
				contentSize = atoi(event->header_value);
				buf = heap_caps_malloc_cast<char>(MALLOC_CAP_PREFERRED, contentSize+1);
			}

			break;
		}

		case HTTP_EVENT_ON_DATA:
		{
			// Keep reading data as long as it is available
			lastResponse = esp_http_client_get_status_code(httpClient);
			if (lastResponse == 200 && contentSize > 0 && !esp_http_client_is_chunked_response(event->client))
			{
				memcpy(&buf[writtenBytes], event->data, event->data_len);
				writtenBytes += event->data_len;
			}
			break;
		}

		case HTTP_EVENT_ON_FINISH:
		{
			buf[contentSize] = '\0';

			char *forecast = strtok(buf, ";");
			while(forecast)
			{
				sendQueueData(weatherQueueTx, WEATHER_QUEUE_RX_FORECAST, forecast, portMAX_DELAY);

				forecast = strtok(NULL, ";");
			}
			sendQueueData(weatherQueueTx, WEATHER_QUEUE_RX_FINISHED_OP, NULL, portMAX_DELAY);

			heap_caps_free(buf);
			writtenBytes = 0;
			break;
		}

		default: break;
	}

	return ESP_OK;
}