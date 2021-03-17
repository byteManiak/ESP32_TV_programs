#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

#include <esp_err.h>
#include <esp_http_client.h>

/**
 * @brief Handler of all requests sent by the user to the HTTP client
 */
esp_err_t httpEventHandler(esp_http_client_event_t *event);

extern QueueHandle_t radioQueueTx;

// Queue flags to be used with the radio queues
enum radio_queue_rx_flag {
	RADIO_QUEUE_RX_WIFI_CONNECTED, // Signal the radio menu that wifi is connected
	RADIO_QUEUE_RX_RADIO_STATION,  // Signal the radio menu that a radio station was sent to the list
	RADIO_QUEUE_RX_FINISHED_OP     // Signal the radio menu that the server finished sending the list of radio stations
};

extern QueueHandle_t appQueueTx;

// Queue flags to be used with the app queues
enum app_queue_rx_flag {
    APP_QUEUE_RX_WIFI_CONNECTED, // Signal the app menu that wifi is connected
    APP_QUEUE_RX_APP_NAME,       // Signal the app menu that an application was sent to the list
    APP_QUEUE_RX_FINISHED_OP // Signal the app menu that the server finished sending the list of application
};

extern QueueHandle_t newsQueueTx;

// Queue flags to be used with the news queues
enum news_queue_rx_flag {
    NEWS_QUEUE_RX_WIFI_CONNECTED,
    NEWS_QUEUE_RX_RSS_FEED_NAME,
    NEWS_QUEUE_RX_HEADLINE_NAME,
    NEWS_QUEUE_RX_FINISHED_OP
};