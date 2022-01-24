#include "game/gem.h"

#include "game/common.h"

#include <iostream>

Gem::Gem(int type, int xd, int yd, int yo)
{
	sprite = new SpriteSDL("gem" + std::to_string(type), 15, 15, 6, 100); 
	this->type = type;
	x = xdest = BASEX+1+xd*16;
	ydest = BASEY+1+yd*16;
	y = BASEY-16-yo*16;

	startTick = getMillis();
}

Gem::~Gem()
{
	delete(sprite);
}

void Gem::draw(bool isSelected)
{
	float dx = abs(x-xdest)+1;
	float dy = abs(y-ydest)+1;
	if (dy > 16) dy = 16;

	if ((int)x < xdest)
	{
		x += dx/20*(getMillis() - startTick)/6.f;
		if (x > xdest) x = xdest;
	}
	else if ((int)x > xdest)
	{
		x -= dx/20*(getMillis() - startTick)/6.f;
		if (x < xdest) x = xdest;
	}

	if ((int)y < ydest)
	{
		y += dy/20*(getMillis() - startTick)/6.f;
		if (y > ydest) y = ydest;
	}
	else if ((int)y > ydest)
	{
		y -= dy/20*(getMillis() - startTick)/6.f;
		if (y < ydest) y = ydest;
	}

	if ((int)x == xdest && (int)y == ydest) isMoving = false;

	startTick = getMillis();

	if (isSelected) sprite->draw(x, y);
	else sprite->drawTile(x, y, 0);
}

void Gem::setNextType()
{
	type++;
	if (type > 6) type = 1;
	sprite->setSprite("gem" + std::to_string(type));
}

void Gem::setCoords(int xd, int yd)
{
	isMoving = true;
	xdest = BASEX+1+xd*16;
	ydest = BASEY+1+yd*16;
}