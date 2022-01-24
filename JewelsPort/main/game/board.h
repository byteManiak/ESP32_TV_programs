#pragma once

#include "engine/engine.h"

#include "game/common.h"
#include "game/gem.h"
#include "game/score.h"
#include "game/bar.h"

#include <vector>

class PauseMenu;
class ProgressGem;

class Board
{
public:
	Board();
	~Board();

	bool update();

	void genBoard();

	void loadGame();
private:
	Score *score;
	Bar *bar;

	PauseMenu *pauseMenu;
	bool isPaused = false;

	// Board generation
	bool isSlotAvailable(int x, int y);
	void genPartialMatch(int x, int y, int type);
	void avoidMatches();

	// Match finder
	void findMatch(bool initBoardStage);
	bool hasMatch = false;
	void sweepMatches();

	// Combos
	int combo = -1;
	// Wait between combos
	bool shortWait = false;
	int waitTick;

	// Game state
	void newGame();
	void saveGame();
	bool tryLoadGame();

	// Gem swapping
	void swap(int x1, int y1, int x2, int y2, bool moveCursor);

	enum SwapState
	{
		NO_SWAP,
		SWAP_FIRST,
		SWAP_BACK
	};

	SwapState swapState = NO_SWAP;
	int x1swap, y1swap, x2swap, y2swap;

	// Player controls
	bool isSelecting = false;
	int xCursor = 4, yCursor = 4;

	Gem *gems[8][8];

	// Board animations
	bool isAnimating = false;

	// Game over
	void checkGameover();
	bool gameover = false;
	bool gameoverFirst = false;

	SpriteSDL *arrows;

	std::vector<ProgressGem> progressGems;
};