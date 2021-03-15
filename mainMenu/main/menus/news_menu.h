#pragma once

#include <ux/submenu.h>

#include <widget/button.h>
#include <widget/list.h>

class NewsMenu : public Submenu
{
public:
    NewsMenu(VGAExtended *vga, const char *title);
    void updateSubmenu();

private:
	Button *connectionStatus;
	List<char*> *newsList;

	enum newsMenuState
	{
		NEWS_MENU_STATE_WAITING_WIFI, // Waiting for Wifi to connect
		NEWS_MENU_STATE_REQUEST_LIST, // Send request to HTTP task to get news headlines
		NEWS_MENU_STATE_DISPLAY_LIST, // Show the list of news headlines and allow user to scroll through them
		NEWS_MENU_STATE_LIST_UPDATING // Waiting for HTTP task to return news headlines
	};

	enum widgetPosition
	{
		CONNECTION_STATUS,
		NEWS_LIST
	};

	newsMenuState state = NEWS_MENU_STATE_WAITING_WIFI;

	void receiveQueueData();

	// Track the current page number from the remote app list
	uint16_t rssFeedIndex = 0;
	uint16_t rssFeedPageNum = 0;
	uint16_t rssFeedPageNumMax = 0xFFFF;
};