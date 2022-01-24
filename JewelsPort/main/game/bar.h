#pragma once

#include "game/common.h"

#include <string>

class Bar
{
public:
	Bar();
	~Bar();

	void addProgress();
	void draw();
	void reset();

	bool startNewLevel = false;

	int maxgems = 60;
	int gemcount = 0;

private:
};