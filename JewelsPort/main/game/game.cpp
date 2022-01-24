#include "engine/engine.h"

#include "game/game.h"
#include "game/common.h"

#include <ctime>

SpriteSDL *test;

Game::Game(VGAExtended *vga)
{
	addColorPalette("Gboy", BLACK, DARK_GREEN, GREEN, ACID);
	addColorPalette("Gold", BLACK, BROWN, ORANGE, YELLOW);
	addColorPalette("Vboy", BLACK, DARK_BROWN, DARK_RED, BUBBLEGUM2);
	addColorPalette("Nymph", BLACK, INDIGO, MIDNIGHT_GREEN, CYAN);
	addColorPalette("Blue", BLACK, DARK_BLUE, BLUE4, CHALK_BLUE2);
	addColorPalette("Lluv", BLACK, INDIGO, WINE, BUBBLEGUM2);
	addColorPalette("Electric", BLACK, DARK_BROWN, GRAY, CYAN4);
	addColorPalette("Monochrome", BLACK, BLACK, GRAY, WHITE);
	addColorPalette("Midnight", BLACK, DARK_BLUE2, BLACK, CYAN4);

	setColorPalette("Gboy");

	createTexture("/spiffs/jewel1.pcx", "gem1");
	createTexture("/spiffs/jewel2.pcx", "gem2");
	createTexture("/spiffs/jewel3.pcx", "gem3");
	createTexture("/spiffs/jewel4.pcx", "gem4");
	createTexture("/spiffs/jewel5.pcx", "gem5");
	createTexture("/spiffs/jewel6.pcx", "gem6");
	createTexture("/spiffs/arrows.pcx", "arrows");

	playMusic("/spiffs/intro.ogg", false);

	menu = new MainMenu();

	board = new Board();
}

bool Game::update()
{
	if (inMenu)
	{
		if (menu->update())
		{
			delete menu;
			inMenu = false;
			board->loadGame();
			playMusic("/spiffs/music.ogg", true);
		}
	}
	else
	{
		if (board->update()) return false;
	}

	static bool isChangingColor = false;
	if (isKeyPressed(Q_key)) isChangingColor = !isChangingColor;

	if (isChangingColor)
	{
		static int cs[4] = {0, 2, 3, 23};
		static int c = 0;

		if (isKeyPressed(Up_key)) cs[c]++;
		if (isKeyPressed(Down_key)) cs[c]--;
		if (isKeyPressed(Left_key)) if (c > 0) c--;
		if (isKeyPressed(Right_key)) if (c < 3) c++;

		char scs[4][3] = {};

		vga->setIndexedColors((VGAColor*)(void*)cs, 4);

		for(int i = 0; i < 4; i++)
		{
			snprintf(scs[i], 3, "%u", cs[i]);
			if (c == i)	vga->printBox(scs[i], (i+1)*32, 100, 63, 63, 0);
			else vga->printBox(scs[i], (i+1)*32, 100, 14, 63, 0);
		}
	}

	if (isKeyPressed(Space_key)) setNextColorPalette();

	return true;
}
