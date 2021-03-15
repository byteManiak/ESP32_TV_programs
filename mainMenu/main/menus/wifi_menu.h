#pragma once

#include <vga/vga.h>
#include <ux/submenu.h>

#include <widget/button.h>
#include <widget/list.h>
#include <widget/textbox.h>

class WifiMenu : public Submenu
{
public:
    WifiMenu(VGAExtended *vga, const char *title);
    void updateSubmenu();

private:
    List<char*> *ssidList;
    Button *actionButton;
    Textbox *passwordTextbox;
    Button *ipDetails, *gatewayDetails;

    enum wifiMenuState
    {
        WIFI_MENU_STATE_DEFAULT,
        WIFI_MENU_STATE_WAITING,        // Waiting for a reply from the low-level Wifi task
        WIFI_MENU_STATE_CHOOSE_SSID,    // Waiting for the user to choose an SSID from the list
        WIFI_MENU_STATE_QUERY_PASSWORD, // Waiting for the user to input a password for the chosen SSID
        WIFI_MENU_STATE_CONNECTED,      // Received reply from low-level Wifi task that we are connected
        WIFI_MENU_STATE_DISCONNECTED    // Received reply from low-level Wifi task that we are disconnected
    };

    enum widgetPosition
    {
        ACTION_BUTTON,
        SSID_LIST,
        IP_DETAILS,
        GATEWAY_DETAILS,
        PASSWORD_TEXTBOX
    };

    wifiMenuState state = WIFI_MENU_STATE_DEFAULT;

    void receiveQueueData();

    // List of SSIDs to choose from after scan event
	int selectedSsid = 0;
	int ssidCount = 0;
	char ssids[8][33];

    // Password to send to wifi task
	char password[65];
	int passwordLength = 0;

    // IP and gateway addresses
    char ipAddress[16], gatewayAddress[16];
};
