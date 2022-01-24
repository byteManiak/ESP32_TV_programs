#include "engine/engine.h"

#include "game/bar.h"

#define BARX 5
#define BARY 4
Bar::Bar()
{
	createTexture("/spiffs/barholder.pcx", "barholder");
	createTexture("/spiffs/bar.pcx", "bar");
	createTexture("/spiffs/bardesc.pcx", "bardesc");
}

Bar::~Bar()
{

}

void Bar::addProgress()
{
	gemcount++;
	if (gemcount >= maxgems)
	{
		maxgems *= 1.5;
		gemcount = 0;
		startNewLevel = true;
	}
}

void Bar::draw()
{
	int t = 79-(getMillis()/300)%80;
	float p = 1-gemcount/(float)maxgems;

	drawTexture("bar", BARX+1, BARY+1+80-t, 20, t,    0, 0);
	drawTexture("bar", BARX+1, BARY+1,      20, 80-t, 0, t);
	vga->drawRectIndexed(BARX+1, BARY+1, 20, 80*p, 2, true);
	drawTexture("barholder", BARX, BARY, 22, 82, 0, 0);

	drawTexture("bardesc", BARX-2, BARY+83, 26, 36, 0, 0);
}

void Bar::reset()
{
	maxgems = 60;
	gemcount = 0;
}
