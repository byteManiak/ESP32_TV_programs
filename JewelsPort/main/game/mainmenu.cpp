#include "game/mainmenu.h"

#include "game/common.h"

#include <iostream>
#include <cmath>

#include <util/numeric.h>
#include <io/ps2.h>

class SpinnyGem
{
public:
	SpinnyGem( int type)
	{
		sprite = new SpriteSDL("gem" + std::to_string(type), 15, 15, 6, 100);
		startTick = getMillis();
	}

	void update()
	{
		int x, y;
		if (!isLooping)
		{
			x = -15 + (getMillis() - startTick) / 20.f;
			y = YCENTRE/3-9;
			if (x >= XCENTRE-7)
			{
				x = XCENTRE-7;
				startTick = getMillis();
				isLooping = true;
			}
		}
		else
		{
			x = XCENTRE-7+sin((getMillis() - startTick) / 600.f)*(3*XCENTRE/4.f);
			y = YCENTRE-21-cos((getMillis() - startTick) / 600.f)*(YCENTRE/2.f);
		}

		sprite->draw(x, y);
	}

private:
	int startTick;
	bool isLooping = false;
	SpriteSDL *sprite;
};

MainMenu::MainMenu()
{
	startTick = getMillis();

	createTexture("/spiffs/logo.pcx", "logo");
	//createSound("/spiffs/intro.ogg", "intro");
	//soundChannel = playSound("intro");
}

MainMenu::~MainMenu()
{
	deleteTexture("logo");
}

#define LOGO_SIZE_X 47
#define LOGO_SIZE_Y 26

bool MainMenu::update()
{
	bool retVal = false;

	if (logoMoving)
	{
		logoY = -LOGO_SIZE_Y*2 + (getMillis() - startTick) / 40.f;
		if (logoY >= YCENTRE - LOGO_SIZE_Y - 18) logoMoving = false;
		if (isKeyPressed(Enter_key)) logoMoving = false;
	}
	else
	{
		logoY = YCENTRE - LOGO_SIZE_Y - 18;
		if ((getMillis() - startTick) > 620)
		{
			startTick = getMillis();
			if (gems.size() < 6)
			{
				gems.push_back(SpinnyGem(gems.size()+1));
			}
		}

		if (isKeyPressed(Enter_key))
		{
			//stopSound(soundChannel);
			retVal = true;
		}

		vga->drawText("a game by bytemaniak", 1, 1);

		vga->drawText("press enter to play", 5, 100);
		vga->drawText("controls:", 45, 116);
		vga->drawText("hold z to swap gems", 5, 124);
		vga->drawText("arrows to move", 25, 132);
	}

	drawTexture("logo", 31, logoY, LOGO_SIZE_X, LOGO_SIZE_Y, 0, 0, 2);
	for(auto &i : gems) i.update();

	return retVal;
}
