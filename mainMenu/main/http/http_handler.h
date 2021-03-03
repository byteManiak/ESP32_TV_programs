#pragma once

#include <esp_err.h>
#include <esp_http_client.h>

/**
 * @brief Handler of all requests sent by the user to the HTTP client
 */
esp_err_t httpEventHandler(esp_http_client_event_t *event);