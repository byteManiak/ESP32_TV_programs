#include <esp_ota_ops.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <ESP32-TV.h>
#include <Ressources/CodePage437_8x8.h>

VGAExtended *vga;

const int r[] = {13, 12}, g[] = {14, 27}, b[] = {33, 32};

void loop();

extern "C" void app_main()
{
	initClientApp(&vga, 500, 240, 13, 12, 14, 27, 33, 32, 22, 23, 2);

	vga->setFont(CodePage437_8x8);

	for(;;) loop();
}

void loop()
{
	calculateTimeDelta();
	updateKeyboard();
	vga->clear();

	vga->setCursor(vga->xres/5, vga->yres/2-9);
	vga->drawText("  This is an example test OTA app. ");
	vga->setCursor(vga->xres/5+4, vga->yres/2+1);
	vga->drawText("Press enter to return to main menu.");

	vga->showDrawables();

	if (isKeyPressed(Enter_key)) closeApp();

	vTaskDelay(10 / portTICK_PERIOD_MS);
}
