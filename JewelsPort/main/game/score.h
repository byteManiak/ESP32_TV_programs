#pragma once

#include "game/common.h"

#include <string>

class Score
{
public:
	Score();
	~Score();

	void addScore(int combo);
	void draw();
	void increaseLevel();
	void setLevel();
	void reset();

	int level = 1;
	int score = 0;
private:
	std::string s, l;
};