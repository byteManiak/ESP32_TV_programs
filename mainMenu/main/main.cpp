// C and ESP-IDF includes
#include <stdlib.h>
#include <time.h>

// ESP32-TV-lib includes
#include <ESP32-TV.h>
#include <Ressources/CodePage437_8x8.h>
#include <ux/menu.h>

#include <menus/submenus.h>

// User app includes
#include <http/http_handler.h>

VGAExtended *vga;

Menu *menu;
WifiMenu *wifiMenu;
AppMenu *appMenu;
RadioMenu *radioMenu;
NewsMenu *newsMenu;

void loop();

extern "C" void app_main()
{
	initCommon(&vga, 425, 240, 13, 12, 14, 27, 33, 32, 22, 23, 1);
	initNetwork(httpEventHandler);
	initSound();

	// Use IBM BIOS font
	vga->setFont(CodePage437_8x8);

	menu = heap_caps_malloc_construct<Menu, VGAExtended*, VGAColor, VGAColor>(MALLOC_CAP_PREFERRED, vga, ORANGE, ACID);

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