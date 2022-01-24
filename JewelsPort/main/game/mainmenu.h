#pragma once

#include "engine/engine.h"

#include <string>
#include <vector>

class SpinnyGem;

class MainMenu
{
public:
	MainMenu();
	~MainMenu();
	bool update();
private:
	uint32_t startTick;
	bool logoMoving = true;
	std::vector<SpinnyGem> gems;
	int logoY;
	int soundChannel;
};