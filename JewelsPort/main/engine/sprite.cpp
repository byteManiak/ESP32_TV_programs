#include "engine/sprite.h"

#include "engine/texture.h"
#include <iostream>

#include <engine/engine.h>

#include <util/numeric.h>

SpriteSDL::SpriteSDL(std::string texture, int tileW, int tileH, int numTiles, int tickSpeed)
{
	this->texture = texture;
	this->tileW = tileW;
	this->tileH = tileH;
	this->numTiles = numTiles;
	this->tickSpeed = tickSpeed;

	startTick = getMillis();
}

void SpriteSDL::drawTile(int x, int y, int tile)
{
	if (tile == -1) drawTexture(texture, x, y, tileW, tileH, currentTile*tileW, 0);
	else drawTexture(texture, x, y, tileW, tileH, tile*tileW, 0);
}

void SpriteSDL::draw(int x, int y)
{
	if (getMillis() - startTick > tickSpeed)
	{
		startTick = getMillis();
		currentTile = (currentTile+1) % numTiles;
	}

	drawTexture(texture.c_str(), x, y, tileW, tileH, tileW*currentTile, 0);
}

void SpriteSDL::setSprite(std::string texture)
{
	this->texture = texture;
}