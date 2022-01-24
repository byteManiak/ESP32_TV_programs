#pragma once

#include "engine/engine.h"

class Gem
{
public:
	Gem(int type, int x, int y, int yo = 0);
	~Gem();
	void draw(bool isSelected = false);
	void setNextType();
	void setVisible(bool isVisible);
	void setCoords(int x, int y);

	int type = 0;
	bool isMoving = false;

	bool isMatched = false;

private:
	int xdest, ydest;
	float x, y;
	SpriteSDL *sprite = NULL;

	int startTick;
};