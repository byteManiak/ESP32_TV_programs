// C and ESP-IDF includes
#include <stdlib.h>
#include <time.h>
#include <esp_event.h>

// bitluni includes
#include <ESP32Lib.h>
#include <Ressources/CodePage437_8x8.h>

// ESP32-TV-lib includes
#include <io/ps2.h>
#include <io/sound.h>
#include <memory/alloc.h>
#include <menus/submenus.h>
#include <stats/stats.h>
#include <util/numeric.h>
#include <util/queues.h>
#include <vga/vga.h>
#include <ux/menu.h>
#include <net/wifi.h>
#include <net/http.h>

// User app includes
#include <http/http_handler.h>

VGAExtended *vga;

// Pins used for driving the VGA signal
const int r[] = {13, 12}, g[] = {14, 27}, b[] = {33, 32};

Menu *menu;
WifiMenu *wifiMenu;
AppMenu *appMenu;
RadioMenu *radioMenu;
NewsMenu *newsMenu;

void loop();

extern "C" void app_main()
{
	srand(time(NULL));

	initSound();
	initKeyboard();

	esp_event_loop_create_default();
	initWifi();
	initHTTP(httpEventHandler);

	vga = heap_caps_malloc_construct<VGAExtended>(MALLOC_CAP_PREFERRED);

	// Initialize VGA display
	vga->setFrameBufferCount(1);
	// Set display mode to 425x240
	vga->init(vga->MODE640x480.custom(425,240), r, g, b, 22, 23);
	// Use IBM BIOS font
	vga->setFont(CodePage437_8x8);

	menu = heap_caps_malloc_construct<Menu, VGAExtended*>(MALLOC_CAP_PREFERRED, vga);

	wifiMenu = heap_caps_malloc_construct<WifiMenu, VGAExtended*, const char*>(MALLOC_CAP_PREFERRED, vga, "Wi-Fi");
	menu->addSubMenu(wifiMenu);

	appMenu = heap_caps_malloc_construct<AppMenu, VGAExtended*, const char*>(MALLOC_CAP_PREFERRED, vga, "Apps");
	menu->addSubMenu(appMenu);

	radioMenu = heap_caps_malloc_construct<RadioMenu, VGAExtended*, const char*>(MALLOC_CAP_PREFERRED, vga, "Radio");
	menu->addSubMenu(radioMenu);

	newsMenu = heap_caps_malloc_construct<NewsMenu, VGAExtended*, const char*>(MALLOC_CAP_PREFERRED, vga, "Headlines");
	menu->addSubMenu(newsMenu);

	// Establish the background color of the screen
	vga->clear(21);
	vga->backColor = 21;

	for(;;) loop();
}

void loop()
{
	// Get time spent to render last frame
	calculateTimeDelta();

	// Get PS/2 keyboard state
    updateKeyboard();

	menu->drawMenu();

#if defined(CONFIG_DEBUG_VGA_PROJ)
	// Display memory statistics
	printMemStats(vga);
#endif

#if defined(CONFIG_DEBUG_SHOW_FPS)
	// Display FPS counter
	printFPS(vga);
#endif

	// Display elements
	vga->showDrawables();

	// Limit framerate to 50fps as there is no point going above that
	vTaskDelay(20 / portTICK_PERIOD_MS);
}
