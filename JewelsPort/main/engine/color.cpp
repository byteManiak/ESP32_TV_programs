#include "engine/engine.h"

#include <unordered_map>
#include <util/numeric.h>

/** TODO: set colors from 3rd row below **/

static std::unordered_map<std::string, ColorPalette> colorPalettes;
static std::unordered_map<std::string, ColorPalette>::iterator currentPaletteIndex;
static ColorPalette currentPalette = {};

void addColorPalette(std::string palName, VGAColor c1, VGAColor c2, VGAColor c3, VGAColor c4)
{
	colorPalettes[palName] = {c1, c2, c3, c4};
}

int startTick;

void setColorPalette(std::string palName)
{
	auto requestedPalette = colorPalettes.find(palName);
	if (requestedPalette == colorPalettes.end())
	{
		setColorPalette("Default");
		return;
	}

	currentPaletteIndex = requestedPalette;

	ColorPalette palette = requestedPalette->second;
	currentPalette.c1 = palette.c1;
	currentPalette.c2 = palette.c2;
	currentPalette.c3 = palette.c3;
	currentPalette.c4 = palette.c4;

	vga->setIndexedColors((VGAColor*)(void*)(&currentPalette), 4);
	vga->setIndexedKey((VGAColor)4);

	vga->frontColor = currentPalette.c4;
}

void setNextColorPalette()
{
	if (std::next(currentPaletteIndex) == colorPalettes.end())
	{
		currentPaletteIndex = colorPalettes.begin();
	}
	else currentPaletteIndex = std::next(currentPaletteIndex);

	setColorPalette(currentPaletteIndex->first);
}
