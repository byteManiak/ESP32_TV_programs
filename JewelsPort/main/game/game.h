#include "game/mainmenu.h"
#include "game/board.h"

#include "engine/engine.h"

class Game
{
public:
	Game(VGAExtended *vga);
	~Game() = default;
	bool update();

private:
	MainMenu *menu;
	Board *board;
	bool inMenu = true;
};