#include "game/board.h"
#include <engine/engine.h>

#include <fstream>

#include <io/ps2.h>

enum PauseReturn
{
	PAUSE_NONE,
	PAUSE_NEWGAME,
	PAUSE_QUIT
};

bool musicMuted = false;

#define PAUSEY 42

class PauseMenu
{
public:
	PauseMenu() = default;
	PauseReturn update()
	{
		if (isKeyPressed(Down_key) || isKeyPressed(S_key)) menuCursor++;
		if (isKeyPressed(Up_key) || isKeyPressed(W_key)) menuCursor--;
		if (menuCursor < 0) menuCursor = 0;
		else if (menuCursor > 3) menuCursor = 3;

		switch(menuCursor)
		{
			case 0:
				if (isKeyPressed(Z_key))
				{
					return PAUSE_NEWGAME;
				}
				break;

			case 1:
			{
				if (isKeyPressed(Z_key))
				{
					soundMuted = !soundMuted;
					muteSound(soundMuted);
				}
				break;
			}
			case 2:
			{
				if (isKeyPressed(Z_key))
				{
					musicMuted = !musicMuted;
					muteMusic(musicMuted);
				}
				break;
			}
			case 3:
				if (isKeyPressed(Z_key))
				{
					return PAUSE_QUIT;
				}
		}

		vga->drawRectIndexed(0, PAUSEY-2, 160, 1, 1, true);
		vga->drawRectIndexed(0, PAUSEY-1, 160, 49, 2, true);
		vga->drawRectIndexed(0, PAUSEY+47, 160, 1, 1, true);
		vga->drawText("pause", XCENTRE-20, PAUSEY);

		vga->drawText("new game", XCENTRE-32, PAUSEY+8);

		vga->drawText("sounds", XCENTRE-24, PAUSEY+16);
		if (!soundMuted) vga->drawText("x", XCENTRE+32, PAUSEY+16);
		vga->drawText("music", XCENTRE-20, PAUSEY+24);

		if (!musicMuted) vga->drawText("x", XCENTRE+24, PAUSEY+24);

		vga->drawText("save and quit", XCENTRE-48, PAUSEY+32);

		vga->drawText("-", 24, PAUSEY+(menuCursor+1)*8);

		return PAUSE_NONE;
	}

private:
	int menuCursor = 0;
	bool soundMuted = false;
};

class ProgressGem
{
public:
	ProgressGem(int type, int x, int y)
	{
		this->x = x; this->y = y;
		texture = "gem" + std::to_string(type);
		startTick = getMillis();
	}

	void draw()
	{
		float dx = abs(x-xdest)+1;
		float dy = abs(y-ydest)+1;

		if ((int)x < xdest)
		{
			x += dx*(getMillis() - startTick)/2000.f;
			if (x > xdest) x = xdest;
		}
		else if ((int)x > xdest)
		{
			x -= dx*(getMillis() - startTick)/2000.f;
			if (x < xdest) x = xdest;
		}

		if ((int)y < ydest)
		{
			y += dy*(getMillis() - startTick)/2000.f;
			if (y > ydest) y = ydest;
		}
		else if ((int)y > ydest)
		{
			y -= dy*(getMillis() - startTick)/2000.f;
			if (y < ydest) y = ydest;
		}

		if ((int)x == xdest && (int)y == ydest) reached = true;

		drawTexture(texture.c_str(), x, y, 15, 15, 0, 0);
	}

	bool reached = false;
private:
	int startTick;
	int xdest = 8, ydest = 4;
	int x, y;
	std::string texture;
};

Board::Board()
{
	score = new Score();
	bar = new Bar();
	pauseMenu = new PauseMenu();

	arrows = new SpriteSDL("arrows", 9, 9, 4, 0);
}

Board::~Board()
{
	delete arrows;
	for(int i = 0; i < 8; i++)
	for(int j = 0; j < 8; j++)
		delete gems[i][j];
}

bool Board::update()
{
	if (gameover)
	{
		pauseMusic();
		if (isKeyPressed(Z_key))
		{
			newGame();
			if (!musicMuted) resumeMusic();
		}
	}
	else if (isPaused)
	{
		if (isKeyPressed(Enter_key)) isPaused = false;
	}
	else
	{
		if (isKeyPressed(Enter_key)) isPaused = true;
		else
		{
			isSelecting = isKeyDown(Z_key);

			if (!isSelecting)
			{
				if (isKeyPressed(Left_key) || isKeyPressed(A_key)) xCursor--;
				if (isKeyPressed(Right_key) || isKeyPressed(D_key)) xCursor++;
				if (isKeyPressed(Up_key) || isKeyPressed(W_key)) yCursor--;
				if (isKeyPressed(Down_key) || isKeyPressed(S_key)) yCursor++;
			}
			else if (!isAnimating && !shortWait && swapState == NO_SWAP)
			{
				if (isKeyPressed(Left_key) || isKeyPressed(A_key))
				{
					if (xCursor > 0)
					{
						swap(xCursor, yCursor, xCursor-1, yCursor, true);
					}
				}
				else if (isKeyPressed(Right_key) || isKeyPressed(D_key))
				{
					if (xCursor < 7)
					{
						swap(xCursor, yCursor, xCursor+1, yCursor, true);
					}
				}
				else if (isKeyPressed(Up_key) || isKeyPressed(W_key))
				{
					if (yCursor > 0)
					{
						swap(xCursor, yCursor, xCursor, yCursor-1, true);
					}
				}
				else if (isKeyPressed(Down_key) || isKeyPressed(S_key))
				{
					if (yCursor < 7)
					{
						swap(xCursor, yCursor, xCursor, yCursor+1, true);
					}
				}
			}

			if (xCursor < 0) xCursor = 0;
			else if (xCursor > 7) xCursor = 7;

			if (yCursor < 0) yCursor = 0;
			else if (yCursor > 7) yCursor = 7;
		}
	}

	isAnimating = false;

	for(int i = 0; i < 8; i++)
	for(int j = 0; j < 8; j++)
	{
		vga->drawRectIndexed(BASEX+i*16, BASEY+j*16, 17, 17, 1);

		if (i == xCursor && j == yCursor)
		{
			vga->drawRectIndexed(BASEX+i*16+1, BASEY+j*16+1, 15, 15, 1, true);
			if(gems[i][j])
			{
				gems[i][j]->draw(true);
				if (gems[i][j]->isMoving) isAnimating = true;
			}
		}
		else
		{
			if (gems[i][j])
			{
				gems[i][j]->draw();
				if (gems[i][j]->isMoving) isAnimating = true;
			}
		}
	}

	findMatch(false);

	if (!isAnimating)
	{
		if (shortWait)
		{
			if (getMillis() - waitTick > 900 || !hasMatch)
			{
				shortWait = false;
			}

			for(int i = 0; i < 8; i++)
			for(int j = 0; j < 8; j++)
			if (gems[i][j]->isMatched)
			{
				vga->drawRectIndexed(BASEX+i*16+1, BASEY+j*16+1, 15, 15, 3, true);
				gems[i][j]->draw(true);
			}
		}
		else
		{
			if (!gameover) checkGameover();

			if (isSelecting && swapState == NO_SWAP)
			{
				if (xCursor > 0) arrows->drawTile(BASEX+xCursor*16-7, BASEY+yCursor*16+4, 0);
				if (xCursor < 7) arrows->drawTile(BASEX+xCursor*16+15, BASEY+yCursor*16+4, 1);
				if (yCursor > 0) arrows->drawTile(BASEX+xCursor*16+4, BASEY+yCursor*16-7, 3);
				if (yCursor < 7) arrows->drawTile(BASEX+xCursor*16+4, BASEY+yCursor*16+15, 2);
			}

			if (swapState == SWAP_FIRST)
			{
				if (hasMatch) swapState = NO_SWAP;
				else swap(x1swap, y1swap, x2swap, y2swap, true);
			}
			else if (swapState == SWAP_BACK) swapState = NO_SWAP;

			if (hasMatch)
			{
				combo++;
				playSound(("/spiffs/combo" + std::to_string(combo) +".ogg").c_str());
				sweepMatches();
			}
			else combo = -1;
		}
	}

	score->draw();
	bar->draw();

	for(auto it = progressGems.begin(); it != progressGems.end();)
	{
		it->draw();
		if (it->reached)
		{
			bar->addProgress();

			if (bar->startNewLevel)
			{
				bar->startNewLevel = false;
				score->increaseLevel();
				playSound("/spiffs/levelup.ogg");
				setNextColorPalette();
			}

			it = progressGems.erase(it);
		}
		else ++it;
	}

	if (gameover)
	{
		if (gameoverFirst) playSound("/spiffs/gameover.ogg");
		gameoverFirst = false;
		vga->drawRectIndexed(0, 63, 160, 1, 1, true);
		vga->drawRectIndexed(0, 64, 160, 25, 2, true);
		vga->drawRectIndexed(0, 88, 160, 1, 1, true);
		vga->drawText("game over", 48, 67);
		vga->drawText("press z to restart", 10, 75);
	}

	if (isPaused)
	{
		PauseReturn ret = pauseMenu->update();
		if (ret == PAUSE_NEWGAME)
		{
			newGame();
			isPaused = false;
		}
		else if (ret == PAUSE_QUIT)
		{
			saveGame();
			closeApp();
			esp_restart();
		}
	}

	return false;
}

void Board::newGame()
{
	genBoard();
	score->reset();
	bar->reset();
	gameover = false;
}

void Board::saveGame()
{
	std::ofstream file;
	file.open("/spiffs/.savegame", std::ios::binary);
	file.write((char*)&score->level, sizeof(int));
	file.write((char*)&score->score, sizeof(int));
	file.write((char*)&bar->gemcount, sizeof(int));
	file.write((char*)&bar->maxgems, sizeof(int));
	for(int i = 0; i < 8; i++)
	for(int j = 0; j < 8; j++)
		file.write((char*)&gems[i][j]->type, sizeof(int));
	file.close();
}

void Board::loadGame()
{
	if (!tryLoadGame()) newGame();
}

bool Board::tryLoadGame()
{
	std::ifstream file;
	file.open("/spiffs/.savegame", std::ios::binary);
	if (!file.good()) return false;

	file.read((char*)&score->level, sizeof(int));
	file.read((char*)&score->score, sizeof(int));
	file.read((char*)&bar->gemcount, sizeof(int));
	file.read((char*)&bar->maxgems, sizeof(int));

	for(int i = 0; i < 8; i++)
	for(int j = 0; j < 8; j++)
	{
		int type;
		file.read((char*)&type, sizeof(int));
		gems[i][j] = new Gem(type, i, j);
	}

	file.close();

	score->addScore(0);
	score->setLevel();

	for(int i = 0; i < (score->level-1)%6; i++) setNextColorPalette();

	findMatch(false);
	sweepMatches();

	return true;
}

enum Dir
{
	LEFT, RIGHT,
	UP, DOWN
};

bool Board::isSlotAvailable(int x, int y)
{
	if (x < 0 || x > 7 || y < 0 || y > 7) return false;

	return gems[x][y] == nullptr;
}

void Board::genPartialMatch(int x, int y, int type)
{
	// Don't generate a partial match where there is already a gem
	if (gems[x][y]) return genPartialMatch(rand()%8, rand()%8, type);

	// Establish which way to generate the match
	Dir dir = (Dir)(rand()%4);
	if (dir == DOWN && y >= 6) dir = UP;
	else if (dir == UP && y <= 1) dir = DOWN;
	else if (dir == RIGHT && x >= 6) dir = LEFT;
	else if (dir == LEFT && x <= 1) dir = RIGHT;

	// Generate match in the given direction

	int x1, y1, x2, y2;
	switch(dir)
	{
		case LEFT:
		case RIGHT:
		{
			x1 = x+1; x2 = x+2;
			if (dir == LEFT) {x1-=2; x2-=4;}

			int r = rand()%6;
			switch(r)
			{
				case 0:
					y1 = y+1; y2 = y;
					break;
				case 1:
					y1 = y-1; y2 = y;
					break;
				case 2:
					y1 = y; y2 = y-1;
					break;
				case 3:
					y1 = y; y2 = y+1;
					break;
				case 4:
					y1 = y-1; y2 = y-1;
					break;
				case 5:
					y1 = y+1; y2 = y+1;
					break;
				default:
					x1 = x; y1 = y; x2 = x; y2 = y;
					break;
			}
			break;
		}
		case UP:
		case DOWN:
		{
			y1 = y+1; y2 = y+2;
			if (dir == UP) {y1-=2; y2-=4;}

			int r = rand()%6;
			switch(r)
			{
				case 0:
					x1 = x+1; x2 = x;
					break;
				case 1:
					x1 = x-1; x2 = x;
					break;
				case 2:
					x1 = x; x2 = x-1;
					break;
				case 3:
					x1 = x; x2 = x+1;
					break;
				case 4:
					x1 = x-1; x2 = x-1;
					break;
				case 5:
					x1 = x+1; x2 = x+1;
					break;
				default:
					x1 = x; y1 = y; x2 = x; y2 = y;
					break;
			}
			break;
		}
		default: x1 = x; y1 = y; x2 = x; y2 = y; break;
	}

	if (isSlotAvailable(x1,y1) && isSlotAvailable(x2,y2))
	{
		gems[x][y] = new Gem(type, x, y);
		gems[x1][y1] = new Gem(type, x1, y1);
		gems[x2][y2] = new Gem(type, x2, y2);
	}
	else return genPartialMatch(rand()%8, rand()%8, type);
}

void Board::avoidMatches()
{
	findMatch(true);

	for(int i = 1; i <= 6; i++)
	for(int j = 0; j < 8; j++)
		if (gems[i-1][j]->type == gems[i][j]->type && gems[i][j]->type == gems[i+1][j]->type)
			gems[i][j]->setNextType();


	for(int j = 1; j <= 6; j++)
	for(int i = 0; i < 8; i++)
		if (gems[i][j-1]->type == gems[i][j]->type && gems[i][j]->type == gems[i][j+1]->type)
			gems[i][j]->setNextType();
}

void Board::genBoard()
{
	// A partial match is a pattern where a match is possible when making a single move
	// e.g. below a match is possible by swapping Y and the X under it
	// X X Y
	// T Z X

	// Gen partial matches

	// Clear the board
	for (int i = 0; i < 8; i++)
	for (int j = 0; j < 8; j++)
		gems[i][j] = nullptr;

	// Gen one match for each color
	for(int i = 0; i < 6; i++)
	{
		genPartialMatch(rand()%8, rand()%8, i+1);
	}

	// Gen random gems for the rest of the board
	for(int i = 0; i < 8; i++)
	for(int j = 0; j < 8; j++)
		if (!gems[i][j]) gems[i][j] = new Gem(rand()%6+1, i, j);

	avoidMatches();
	while(hasMatch)	avoidMatches();
}

void Board::findMatch(bool initBoardStage)
{
	hasMatch = false;

	for(int i = 0; i < 6; i++)
	for(int j = 0; j < 8; j++)
	{
		if (gems[i][j]->type == gems[i+1][j]->type && gems[i][j]->type == gems[i+2][j]->type)
		{
			if (!initBoardStage)
			{
				gems[i][j]->isMatched = true;
				gems[i+1][j]->isMatched = true;
				gems[i+2][j]->isMatched = true;
			}
			hasMatch = true;
		}
	}

	for(int i = 0; i < 8; i++)
	for(int j = 0; j < 6; j++)
	{
		if (gems[i][j]->type == gems[i][j+1]->type && gems[i][j]->type == gems[i][j+2]->type)
		{
			if (!initBoardStage)
			{
				gems[i][j]->isMatched = true;
				gems[i][j+1]->isMatched = true;
				gems[i][j+2]->isMatched = true;
			}
			hasMatch = true;
		}
	}
}

void Board::sweepMatches()
{
	for(int i = 0; i < 8; i++)
	{
		int gemsMatched = 0;
		for(int j = 0; j < 8; j++)
		{
			if (gems[i][j]->isMatched)
			{
				progressGems.push_back(ProgressGem(gems[i][j]->type,BASEX+i*16,BASEY+j*16));

				delete(gems[i][j]);
				gems[i][j] = nullptr;

				for (int k = j; k > 0; k--)
					swap(i, k, i, k-1, false);

				gems[i][0] = new Gem(rand()%6+1, i, 0, gemsMatched);
				gemsMatched++;

				shortWait = true;
				waitTick = getMillis();

				score->addScore(combo+1);
			}
		}
	}
}

void Board::swap(int x1, int y1, int x2, int y2, bool moveCursor)
{
	std::swap(gems[x1][y1], gems[x2][y2]);

	if (moveCursor)
	{
		xCursor = x2; yCursor = y2;
	}

	if (gems[x1][y1]) gems[x1][y1]->setCoords(x1, y1);
	if (gems[x2][y2]) gems[x2][y2]->setCoords(x2, y2);

	x1swap = x2; y1swap = y2;
	x2swap = x1; y2swap = y1;

	if (swapState == NO_SWAP) swapState = SWAP_FIRST;
	else if (swapState == SWAP_FIRST) swapState = SWAP_BACK;
}

void Board::checkGameover()
{
	gameover = true;

	// Check matches of type
	// X X Y
	// Y Z X
	// or
	// X Y X
	// Z X Z
	for(int i = 0; i < 7; i++)
	{
		int occurences[6] = {};
		for(int j = 0; j < 8; j++)
		{
			for(int k = 1; k <= 6; k++)
			{
				if (gems[i][j]->type == k || gems[i+1][j]->type == k)
				{
					occurences[k-1]++;
					if(occurences[k-1] >= 3) gameover = false;
				}
				else occurences[k-1] = 0;
			}
		}
	}

	for(int j = 0; j < 7; j++)
	{
		int occurences[6] = {};
		for(int i = 0; i < 8; i++)
		{
			for(int k = 1; k <= 6; k++)
			{
				if (gems[i][j]->type == k || gems[i][j+1]->type == k)
				{
					occurences[k-1]++;
					if(occurences[k-1] >= 3) gameover = false;
				}
				else occurences[k-1] = 0;
			}
		}
	}

	// Check matches of type
	// X X Y X
	// or
	// X Y X X
	for(int i = 0; i < 5; i++)
	for(int j = 0; j < 8; j++)
	{
		if (gems[i][j]->type == gems[i+3][j]->type &&
			(gems[i][j]->type == gems[i+1][j]->type ||
			 gems[i][j]->type == gems[i+2][j]->type))
			 {
				 gameover = false;
			 }
	}

	for(int j = 0; j < 5; j++)
	for(int i = 0; i < 8; i++)
	{
		if (gems[i][j]->type == gems[i][j+3]->type &&
			(gems[i][j]->type == gems[i][j+1]->type ||
			 gems[i][j]->type == gems[i][j+2]->type))
			 {
				 gameover = false;
			 }
	}

	gameoverFirst = gameover;
}
