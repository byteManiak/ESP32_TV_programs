#pragma once

#include <ux/submenu.h>

#include <widget/button.h>
#include <widget/list.h>

class RadioMenu : public Submenu
{
public:
	RadioMenu(VGAExtended *vga, const char *title);
	void updateSubmenu();

private:
	List<char*> *radioStationList;
	Button *connectionStatus;

	enum radioMenuState
	{
		RADIO_MENU_STATE_WAITING_WIFI, // Waiting for Wifi to connect
		RADIO_MENU_STATE_REQUEST_LIST, // Send request to HTTP task to get radio list entries
		RADIO_MENU_STATE_DISPLAY_LIST, // Show the radio list entries and allow user to pick one
		RADIO_MENU_STATE_LIST_UPDATING // Waiting for HTTP task to return radio list entries
	};

	enum widgetPosition
	{
		CONNECTION_STATUS,
		RADIO_LIST
	};

	radioMenuState state = RADIO_MENU_STATE_WAITING_WIFI;

	void receiveQueueData();

	// Track the current page number from the remote radio list
	uint16_t radioPageNum = 0;
	uint16_t radioPageNumMax = 0xFFFF;
};
