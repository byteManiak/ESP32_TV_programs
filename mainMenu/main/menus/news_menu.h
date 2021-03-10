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
		NEWS_MENU_STATE_WAITING_WIFI,
		NEWS_MENU_STATE_REQUEST_LIST,
		NEWS_MENU_STATE_DISPLAY_LIST,
		NEWS_MENU_STATE_LIST_UPDATING
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