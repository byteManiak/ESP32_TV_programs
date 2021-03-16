#include <menus/wifi_menu.h>

#include <string.h>

#include <io/ps2.h>
#include <memory/alloc.h>
#include <net/wifi.h>
#include <util/numeric.h>
#include <util/queues.h>

WifiMenu::WifiMenu(VGAExtended *vga, const char *title) : Submenu(vga, title)
{
    attachQueues(wifiQueueRx, wifiQueueTx);

    actionButton = heap_caps_malloc_cast<Button>(MALLOC_CAP_PREFERRED);
    new (actionButton) Button(vga, "Begin WiFi scan", vga->xres/2 + vga->xres/16, vga->yres/6);
    actionButton->setFillColor(56);
    widgets.push_back(actionButton);

    ssidList = heap_caps_malloc_cast<List<char*>>(MALLOC_CAP_PREFERRED);
    new (ssidList) List<char*>(vga, vga->xres/2 + vga->xres/16, vga->yres/4, 8, true);
    widgets.push_back(ssidList);

    ipDetails = heap_caps_malloc_cast<Button>(MALLOC_CAP_PREFERRED);
    new (ipDetails) Button(vga, "", vga->xres/2 + vga->xres/16, vga->yres/2 + vga->yres/12);
    ipDetails->setVisible(false);
    widgets.push_back(ipDetails);

    gatewayDetails = heap_caps_malloc_cast<Button>(MALLOC_CAP_PREFERRED);
    new (gatewayDetails) Button(vga, "", vga->xres/2 + vga->xres/16, vga->yres/2 + vga->yres/6);
    gatewayDetails->setVisible(false);
    widgets.push_back(gatewayDetails);

    setFocusedWidget(ACTION_BUTTON);
}

void WifiMenu::receiveQueueData()
{
    if (queueRx)
    {
        queue_message *rxMessage;
        if (xQueueReceive(queueRx, &rxMessage, 0) == pdTRUE)
        {
            switch (rxMessage->msg_flags)
            {
                // A SSID was sent from the WiFi task, therefore it can be added to the menu list
                case WIFI_QUEUE_RX_SCAN_RESULT:
                {
                    ESP_LOGI("menu", "Received ssid %s", rxMessage->msg_text);
                    // Keep reading until the list reaches its max capacity
                    if (!ssidList->isFull())
                    {
                        char *ssid = heap_caps_malloc_cast<char>(MALLOC_CAP_PREFERRED, 33);
                        strlcpy(ssid, rxMessage->msg_text, 33);
                        ssidList->addElement(ssid);
                    }

                    setFocusedWidget(SSID_LIST);
                    // Now that the list is not empty, a SSID can be chosen from it
                    state = WIFI_MENU_STATE_CHOOSE_SSID;
                    break;
                }

                // Indicate that a WiFi connection has been made, and display IP info
                case WIFI_QUEUE_RX_CONNECTED:
                {
                    ipDetails->setVisible(true);
                    gatewayDetails->setVisible(true);

                    actionButton->setText("Connected! Rescan?");
                    actionButton->setFillColor(GREEN);

                    state = WIFI_MENU_STATE_DEFAULT;

                    sendQueueData(radioQueueTx, RADIO_QUEUE_RX_WIFI_CONNECTED, NULL);
                    sendQueueData(appQueueTx, APP_QUEUE_RX_WIFI_CONNECTED, NULL);
                    sendQueueData(newsQueueTx, NEWS_QUEUE_RX_WIFI_CONNECTED, NULL);
                    break;
                }

                // Indicate that WiFi dropped, and prompt to rescan
                case WIFI_QUEUE_RX_DISCONNECTED:
                {
                    actionButton->setText("Connection lost. Rescan?");
                    actionButton->setFillColor(WINE);

                    state = WIFI_MENU_STATE_DISCONNECTED;
                    break;
                }

                // Receive the IP address when WiFi connects
                case WIFI_QUEUE_RX_IP_ADDRESS:
                {
                    char ipText[65];
                    strlcpy(ipAddress, rxMessage->msg_text, 16);
                    sprintf(ipText, "IP Address: %s", ipAddress);
                    ipDetails->setText(ipText);
                    break;
                }

                // Receive the gateway address when WiFi connects
                case WIFI_QUEUE_RX_GATEWAY_ADDRESS:
                {
                    char gatewayText[65];
                    strlcpy(gatewayAddress, rxMessage->msg_text, 16);
                    sprintf(gatewayText, "Gateway Address: %s", gatewayAddress);
                    gatewayDetails->setText(gatewayText);
                    break;
                }
            }
            heap_caps_free(rxMessage);
        }
    }
}

void WifiMenu::updateSubmenu()
{
    updateState();

    // Logic for the wifi menu
    switch (state)
    {
        // Wait for user to press the button in order to begin scanning for networks
        case WIFI_MENU_STATE_DEFAULT:
        {
            bool isButtonPushed = actionButton->getStatus();
            if (isButtonPushed)
            {
                esp_err_t error = sendQueueData(queueTx, WIFI_QUEUE_TX_USER_BEGIN_SCAN, NULL);
                if (error == ESP_OK)
                {
                    // Once a SSID scan request has been made, the list 
                    // is cleared to display the new received SSIDs
                    ssidList->clear();
                    ESP_LOGI("menu", "Sent scan event to queue %p", queueTx);
                    // No longer need to scan an SSID
                    state = WIFI_MENU_STATE_WAITING;
                    actionButton->setFillColor(CORNFLOWER);
                    actionButton->setText("Scanning WiFi...");
                }
            }

            break;
        }

        // Wait for user to choose an SSID from the list of available ones
        case WIFI_MENU_STATE_CHOOSE_SSID:
        {
            actionButton->setFillColor(PURPLE);
            actionButton->setText("Scan results:");
            // Logic for choosing a SSID for which to enter a password and then connect to
            if (!ssidList->isEmpty())
            {
                // Get the selected SSID from the list
                int8_t listElementIndex = ssidList->getStatus();
                if (listElementIndex != -1)
                {
                    esp_err_t e = sendQueueData(queueTx, WIFI_QUEUE_TX_USER_SSID, ssidList->getElement());
                    if (e == ESP_OK)
                    {
                        // Once an SSID is selected, the user can now input a password
                        state = WIFI_MENU_STATE_QUERY_PASSWORD;

                        passwordTextbox = (Textbox*)heap_caps_malloc(sizeof(Textbox), MALLOC_CAP_PREFERRED);
                        new (passwordTextbox) Textbox(vga, true);
                        widgets.push_back(passwordTextbox);

                        setFocusedWidget(PASSWORD_TEXTBOX);
                    }
                }
            }

            break;
        }

        // Wait for user to input a password
        case WIFI_MENU_STATE_QUERY_PASSWORD:
        {
            // When the Enter key is pressed, the password is sent to the WiFi task to connect
            int8_t textboxState = passwordTextbox->getStatus();
            if (textboxState == TEXTBOX_ENTER)
            {
                esp_err_t error = sendQueueData(queueTx, WIFI_QUEUE_TX_USER_PSK, passwordTextbox->getText());
                if (error == ESP_OK)
                {
                    // Wait for result of connection attempt
                    actionButton->setText("Connecting...");
                    ssidList->clear();

                    state = WIFI_MENU_STATE_WAITING;
                    setFocusedWidget(ACTION_BUTTON);

                    widgets.pop_back();
                    heap_caps_free(passwordTextbox);
                }
            }
            else if (textboxState == TEXTBOX_CANCEL)
            {
                state = WIFI_MENU_STATE_CHOOSE_SSID;
                setFocusedWidget(SSID_LIST);

                widgets.pop_back();
                heap_caps_free(passwordTextbox);
            }

            break;
        }

        // When the WiFi disconnects, prompt the user to rescan for SSIDs
        case WIFI_MENU_STATE_DISCONNECTED:
        {
            bool isButtonPushed = actionButton->getStatus();
            if (isButtonPushed)
            {
                esp_err_t error = sendQueueData(queueTx, WIFI_QUEUE_TX_USER_BEGIN_SCAN, NULL);
                if (error == ESP_OK)
                {
                    // Wait for the list of SSIDs to be populated
                    ssidList->clear();
                    ESP_LOGI("menu", "Sent scan event to queue %p", queueTx);
                    state = WIFI_MENU_STATE_WAITING;
                    actionButton->setText("Scanning WiFi...");
                    setFocusedWidget(SSID_LIST);
                }
            }

            break;
        }

        // Nothing for user to do in case of a connection, or waiting for SSIDs
        case WIFI_MENU_STATE_WAITING:
            break;
    }
}
