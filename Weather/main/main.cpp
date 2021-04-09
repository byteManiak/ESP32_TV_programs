/*
	Author: Mihai Daniel Ivanescu, Coventry University
 */

#include <esp_ota_ops.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <ESP32-TV.h>
#include <Ressources/CodePage437_8x8.h>
#include <Graphics/Sprites.h>
#include <stats/stats.h>

#include <http_handler.h>
#include <weather_menu.h>

VGAExtended *vga;
WeatherMenu *menu;

const int r[] = {13, 12}, g[] = {14, 27}, b[] = {33, 32};

void loop();

extern "C" void app_main()
{
	initClientApp(&vga, 320, 240, 13, 12, 14, 27, 33, 32, 22, 23, 2);
	initNetwork(httpEventHandler);

	vga->setFont(CodePage437_8x8);

	menu = heap_caps_malloc_construct<WeatherMenu, VGAExtended*>(MALLOC_CAP_PREFERRED, vga);

	for(;;) loop();
}

void loop()
{
	calculateTimeDelta();
	updateKeyboard();
	vga->clear(VIOLET);

	//printFPS(vga);
	//printMemStats(vga);

	menu->updateSubmenu();
	menu->drawSubmenu();

	vga->showDrawables();

	vTaskDelay(10 / portTICK_PERIOD_MS);
}
