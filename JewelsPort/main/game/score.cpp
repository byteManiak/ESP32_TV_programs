#include "engine/engine.h"

#include "game/score.h"

#define SCOREX 100
#define SCOREY 132
#define LEVELX 6
#define LEVELY 132

Score::Score()
{
	s = "0000000";
	l = "001";
}

void Score::addScore(int combo)
{
	score += combo * (level*5);
	std::string t = std::to_string(score);
	s = std::string(7-t.length(),'0') + t;
}

void Score::draw()
{
	vga->drawText("level", LEVELX, LEVELY);
	vga->drawText(l.c_str(), LEVELX+48, LEVELY);
	vga->drawText(s.c_str(), SCOREX, SCOREY);
}

void Score::setLevel()
{
	std::string t = std::to_string(level);
	l = std::string(3-t.length(),'0') + t;
}

void Score::increaseLevel()
{
	level++;
	setLevel();
}

void Score::reset()
{
	level = 1;
	score = 0;
	s = "0000000";
	l = "001";
}