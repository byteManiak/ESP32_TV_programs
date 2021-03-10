#pragma once

#include <ux/submenu.h>

#include <widget/button.h>
#include <widget/list.h>

class AppMenu : public Submenu
{
public:
    AppMenu(VGAExtended *vga, const char *title);
    void updateSubmenu();

private:
	Button *connectionStatus;
	List<char*> *appList;

	enum appMenuState
	{
		APP_MENU_STATE_DEFAULT,
		APP_MENU_STATE_LOAD_INTERNAL,
		APP_MENU_STATE_REQUEST_LIST,
		APP_MENU_STATE_LIST_UPDATING,
		APP_MENU_STATE_DOWNLOAD_BIN
	};

	enum widgetPosition
	{
		CONNECTION_STATUS,
		APP_LIST
	};

	appMenuState state = APP_MENU_STATE_DEFAULT;

	void receiveQueueData();

	void loadInternalApp();

	// Establish whether there is a user-downloaded app installed to OTA partition
	bool hasDownloadedApp = false;
	char downloadedAppName[32];

	bool wifiConnected = false;

	// Track the current page number from the remote app list
	uint16_t appPageNum = 0;
	uint16_t appPageNumMax = 0xFFFF;
};
