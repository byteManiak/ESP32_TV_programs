#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

#include <esp_err.h>
#include <esp_http_client.h>

/**
 * @brief Handler of all requests sent by the user to the HTTP client
 */
esp_err_t httpEventHandler(esp_http_client_event_t *event);

// Queue flags to be used by any user-defined HTTP clients
enum http_queue_tx_flag {
	HTTP_QUEUE_TX_REQUEST_WEATHER
};

extern QueueHandle_t weatherQueueTx;

// Queue flags to be used with the weather queues
enum weather_queue_rx_flag {
	WEATHER_QUEUE_RX_FORECAST,
	WEATHER_QUEUE_RX_FINISHED_OP
};