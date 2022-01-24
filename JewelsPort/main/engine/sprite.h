#pragma once

#include <string>

class SpriteSDL
{
public:
	SpriteSDL(std::string texture, int tileW, int tileH, int numTiles, int tickSpeed);
	void drawTile(int x, int y, int tile = -1);
	void draw(int x, int y);
	void setSprite(std::string texture);

private:
	int tickSpeed;
	int startTick;

	int numTiles;
	int currentTile = 0;

	int tileW, tileH;
	std::string texture;
};