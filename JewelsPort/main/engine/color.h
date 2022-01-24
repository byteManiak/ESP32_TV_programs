#pragma once

#include <string>

#include <ESP32-TV.h>

typedef struct _ColorPalette
{
	VGAColor c1, c2, c3, c4;
} ColorPalette;

/**
 * @brief Add a new color palette to the existing list of palettes
 *
 * @param palName Name of the color palette
 * @param c1 First color in the palette
 * @param c2 Second color in the palette
 * @param c3 Third color in the palette
 * @param c4 Fourth color in the palette
 */
void addColorPalette(std::string palName, VGAColor c1, VGAColor c2, VGAColor c3, VGAColor c4);

/**
 * @brief Set the current color palette
 *
 * @param palName Name of the palette to use
 */
void setColorPalette(std::string palName);

void setNextColorPalette();
