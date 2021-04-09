/*
	Author: Mihai Daniel Ivanescu, Coventry University
 */

#pragma once

#include <string.h>
#include <esp_log.h>

#include <io/ps2.h>

enum PieceType
{
	NO_PIECE = 0,
	T_PIECE,
	SQUARE_PIECE,
	L_PIECE,
	L_PIECE_B,
	S_PIECE,
	S_PIECE_B,
	LINE_PIECE
};

class Piece
{
public:
	Piece(VGAExtended *vga, PieceType **field) : vga{vga}, field{field} {}

	enum PieceState
	{
		FALLING,
		PLACED,
		OVERFLOWED
	};

	PieceState state = FALLING;

	void moveLeft()
	{
		if (leftmostChunk+xPos > 0)
		{
			xPos--;
			if (isColliding()) xPos++;
		}
	}
	void moveRight()
	{
		if (rightmostChunk+1+xPos < COLUMNS)
		{
			xPos++;
			if (isColliding()) xPos--;
		}
	}
	void moveDown()
	{
		yPos++;
		if (isColliding() && state != PLACED) { placeDown(); }
	}

	virtual void draw() = 0;

	/**
	 * @brief Rotate the currently falling piece.
	 */
	virtual void rotate() = 0;

	int8_t xPos = COLUMNS/2, yPos = 0;

protected:
	/**
	 * @brief Check if the currently falling piece is colliding with pieces already on the field
	 */
	virtual bool isColliding() = 0;

	/**
	 * @brief Places a collided piece down on the field.
	 * @return Returns false if the piece has overflowed the field, resulting in a game over.
	 */
	virtual bool placeDown() = 0;

	int8_t leftmostChunk = 0;
	int8_t rightmostChunk = 0;

	VGAExtended *vga;
	PieceType **field;
};

class Piece3x3Collider : public Piece
{
public:
	Piece3x3Collider(VGAExtended *vga, PieceType **field) : Piece(vga, field)
	{
		if (isColliding()) state = OVERFLOWED;
	}

	void rotate()
	{
		PieceType originalCollider[3][3];
		memcpy(originalCollider, collider, 9 * sizeof(PieceType));

		int8_t leftmostOriginal = leftmostChunk;
		int8_t rightmostOriginal = rightmostChunk;

		for(int i = 0; i < 3; i++)
			for(int j = 0; j < i; j++)
				std::swap(collider[i][j], collider[j][i]);

		std::swap(collider[0][0], collider[0][2]);
		std::swap(collider[1][0], collider[1][2]);
		std::swap(collider[2][0], collider[2][2]);

		if (collider[0][0] != NO_PIECE || collider[1][0] != NO_PIECE || collider[2][0] != NO_PIECE)
			leftmostChunk = 0;
		else leftmostChunk = 1;

		if (collider[0][2] != NO_PIECE || collider[1][2] != NO_PIECE || collider[2][2] != NO_PIECE)
			rightmostChunk = 2;
		else rightmostChunk = 1;

		if (leftmostChunk + xPos < 0) xPos += leftmostChunk+1;
		if (rightmostChunk + xPos >= COLUMNS) xPos -= rightmostChunk-1;

		if (isColliding())
		{
			memcpy(collider, originalCollider, 9 * sizeof(PieceType));
			leftmostChunk = leftmostOriginal;
			rightmostChunk = rightmostOriginal;
		}
	}

	void draw()
	{
		for(int i = 0; i < 3; i++)
			for(int j = 0; j < 3; j++)
				if(collider[i][j] != NO_PIECE)
					vga->rect(xBase+(xPos+j)*8, yBase+(yPos+i)*8, 7, 7, WHITE);
	}

protected:
	bool isColliding()
	{
		for(int i = 0; i < 3; i++)
			for(int j = 0; j < 3; j++)
			{
				if (collider[i][j] != NO_PIECE)
				{
					if (yPos+i >= ROWS) return true;
					if (field[yPos+i][xPos+j]) return true;
				}
			}

		return false;
	}

	bool placeDown()
	{
		if (yPos > 0) state = PLACED;
		else state = OVERFLOWED;

		for(int i = 0; i < 3; i++)
			for(int j = 0; j < 3; j++)
				if(collider[i][j] != NO_PIECE)
				{
					if (i+yPos-1 >= ROWS) continue;
					// Y position going outside of field means game over condition
					if (i+yPos-1 < 0) return false;
					// Place the piece one block higher than where it collided
					else field[yPos+i-1][xPos+j] = collider[i][j];
				}

		return true;
	}

	PieceType collider[3][3] = { {}, {}, {} };
};
