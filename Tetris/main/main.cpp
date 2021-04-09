/*
	Author: Mihai Daniel Ivanescu, Coventry University
 */

#include <esp_ota_ops.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <stdlib.h>
#include <time.h>

#include <ESP32-TV.h>
#include <Ressources/CodePage437_8x8.h>
#include <ux/menu.h>

#include <game/game.h>
#include <music.h>

VGAExtended *vga;
Game *game;

void loop();

extern "C" void app_main()
{
	initClientApp(&vga, 160, 150, 13, 12, 14, 27, 33, 32, 22, 23, 2);
	initBuzzers();

	playBuzzerMusic(&music);

	vga->setFont(CodePage437_8x8);

	game = heap_caps_malloc_construct<Game, VGAExtended*>(MALLOC_CAP_PREFERRED, vga);

	for(;;) loop();
}

void loop()
{
	calculateTimeDelta();
	updateKeyboard();
	
	game->update();
	game->draw();

	vga->showDrawables();

	vTaskDelay(20 / portTICK_PERIOD_MS);
}
