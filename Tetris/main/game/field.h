/*
	Author: Mihai Daniel Ivanescu, Coventry University
 */

#pragma once

#include <vga/vga.h>

const uint16_t xBase = 4;
const uint16_t yBase = 4;

#define ROWS 18
#define COLUMNS 10

#include <game/piece.h>
#include <game/piece_types/3x3pieces.h>
#include <game/piece_types/linepiece.h>

class Field
{
public:
	Field(VGAExtended *vga);

	/**
	 * @brief Clear all the pieces from the field and reset the speed.
	 */
	void clearField();

	void drawField();

	/**
	 * @brief Update the state of the field e.g. pieces to move, lines to clear.
	 * @return If >= 0, number of lines that were just cleared in a tick.
	 *         Returning -1 represents game over condition.
	 */
	int updateField();

	/**
	 * @brief Increase the speed of the falling piece and change the color palette.
	 */
	void advanceLevel();

	/**
	 * @brief Create a piece from the specified piece type.
	 */
	Piece *createPiece(PieceType pieceType);

	PieceType nextPiece;

	VGAColor fieldColors[3] = {LIGHT_GREEN2, ACID, YELLOW};

private:
	VGAExtended *vga;

	void drawBox(int row, int column, int variation);
	void drawCheckered(int row, int column, int variation);
	void drawWaves(int row, int column, int variation);
	void drawIce(int row, int column, int variation);

	/**
	 * @brief Scan the field for any lines that need clearing.
	 */
	void scanForLines();

	Piece *movingPiece;
	int speedTimer = 75, speedTimerMax = 75;
	int movePieceTimer = 0;

	PieceType **field;
	uint8_t numLinesToClear = 0;
	uint8_t linesToClear[4] = {};
	bool isClearingLines = false;
	double timeClearStart;
};