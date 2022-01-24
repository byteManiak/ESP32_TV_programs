#include <string.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <ESP32-TV.h>
#include <Ressources/Font8x8.h>

#include <esp_spiffs.h>

#include <game/game.h>

VGAExtended *vga;

void loop();

Game *game;

extern "C" void app_main()
{
	initClientApp(&vga, 160, 144, 13, 12, 14, 27, 33, 32, 22, 23, 2);

    esp_vfs_spiffs_conf_t spiffsConfig = {};
    spiffsConfig.base_path = "/spiffs";
    spiffsConfig.max_files = 12;
    spiffsConfig.format_if_mount_failed = false;
    esp_vfs_spiffs_register(&spiffsConfig);

    initSound(STREAM_TYPE_FS, DECODER_TYPE_OGG);

	vga->setFont(Font8x8);

    game = new Game(vga);

	for(;;) loop();
}

void loop()
{
	calculateTimeDelta();
	updateKeyboard();

    vga->clearIndexed(2);

    game->update();

	vga->showDrawables();

	vTaskDelay(20 / portTICK_PERIOD_MS);
}
