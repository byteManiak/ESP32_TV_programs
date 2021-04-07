/*
	Author: Mihai Daniel Ivanescu, Coventry University
	License: BSD License
 */

#pragma once

#include <math.h>

#include <memory/alloc.h>

#include <game/field.h>

class Game
{
public:
	Game(VGAExtended *vga) : vga{vga}
	{
		field = heap_caps_malloc_construct<Field, VGAExtended*>(MALLOC_CAP_PREFERRED, vga);
	}

	void update()
	{
		if (isKeyPressed(ESC_key)) isPaused = !isPaused;
		if (isGameover && isKeyPressed(Enter_key))
		{
			isGameover = false;
			field->clearField();
			score = lines = 0;
			level = 1;
			levelLinesThreshold = 10;
		}

		if (!isGameover && !isPaused)
		{
			int result = field->updateField();
			if (result > -1)
			{
				lines += result;
				if (result > 0) score += 100*pow(1.75+level*.25, result);
				if (lines >= levelLinesThreshold)
				{
					level++;
					levelLinesThreshold += 10;
					field->advanceLevel();
				}
			}
			else isGameover = true;

			PieceType nextPieceTypeNew = field->nextPiece;
			if (nextPieceTypeNew != nextPieceType)
			{
				heap_caps_free(nextPiece);
				nextPieceType = nextPieceTypeNew;
				nextPiece = field->createPiece(nextPieceType);

				nextPiece->xPos = 14;
				nextPiece->yPos = 1;

				if (nextPieceType == LINE_PIECE)
				{
					nextPiece->rotate();
					nextPiece->xPos = 13;
				}
			}
		}
	}

	void draw()
	{
		field->drawField();

		vga->setCursor(vga->xres/2 + vga->xres/7, 2);
		vga->setTextColor(WHITE);

		vga->println("Next:\n\n\n");

		nextPiece->draw();

		vga->println("Score:");
		char scoreTxt[8];
		sprintf(scoreTxt, "%6d\n", score);
		vga->println(scoreTxt);

		vga->println("Lines:");
		char linesTxt[8];
		sprintf(linesTxt, "%6d\n", lines);
		vga->println(linesTxt);

		vga->println("Level");
		char levelTxt[7];
		sprintf(levelTxt, "%6d", level);
		vga->print(levelTxt);

		if (isGameover || isPaused)
		{
			vga->fillRect(0, vga->xres/2-5, vga->xres, 10, field->fieldColors[2]);
			vga->setCursor(vga->xres/4, vga->xres/2-4);
			vga->setTextColor(BLACK);
			if (isGameover)	vga->print("GAME OVER");
			else vga->print("GAME PAUSED");
		}
	}

private:
	VGAExtended *vga;
	Field *field;

	int score = 0;
	uint8_t level = 1;
	int levelLinesThreshold = 10;
	uint8_t lines = 0;

	bool isGameover = false;
	bool isPaused = false;

	PieceType nextPieceType = NO_PIECE;
	Piece *nextPiece = NULL;
};