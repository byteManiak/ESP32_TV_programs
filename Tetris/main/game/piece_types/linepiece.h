/*
	Author: Mihai Daniel Ivanescu, Coventry University
 */

#pragma once

#include <game/piece.h>

class LinePiece : public Piece
{
public:
	LinePiece(VGAExtended *vga, PieceType **field) : Piece(vga, field)
	{
		xPos = 4;

		collider[0][0] = collider[1][0] = collider[2][0] = collider[3][0] = LINE_PIECE;
		leftmostChunk = 0;
		rightmostChunk = 0;

		if (isColliding()) state = OVERFLOWED;
	}

	void rotate()
	{
		PieceType originalCollider[4][4];
		memcpy(originalCollider, collider, 16 * sizeof(PieceType));

		int8_t leftmostOriginal = leftmostChunk;
		int8_t rightmostOriginal = rightmostChunk;

		for(int i = 0; i < 4; i++)
			for(int j = 0; j < i; j++)
				std::swap(collider[i][j], collider[j][i]);

		for(int i = 0; i < 4; i++)
			for(int j = 0; j < 2; j++)
				std::swap(collider[i][j], collider[i][3-j]);
		
		if ((collider[0][0] != NO_PIECE && collider[0][3] != NO_PIECE) ||
				 (collider[3][0] != NO_PIECE && collider[3][3] != NO_PIECE))
		{
			leftmostChunk = 0;
			rightmostChunk = 3;
		}
		else if (collider[0][0] == NO_PIECE)
		{
			leftmostChunk = 3;
			rightmostChunk = 3;
		}
		else
		{
			leftmostChunk = 0;
			rightmostChunk = 0;
		}

		if (leftmostChunk + xPos < 0) xPos += leftmostChunk+1;
		if (rightmostChunk + xPos >= COLUMNS) xPos -= rightmostChunk-1;

		if (isColliding())
		{
			memcpy(collider, originalCollider, 16 * sizeof(PieceType));
			leftmostChunk = leftmostOriginal;
			rightmostChunk = rightmostOriginal;
		}
	}

	void draw()
	{
		for(int i = 0; i < 4; i++)
				for(int j = 0; j < 4; j++)
					if(collider[i][j] != NO_PIECE)
						vga->rect(xBase+(xPos+j)*8, yBase+(yPos+i)*8, 7, 7, WHITE);
	}

private:
	bool isColliding()
	{
		for(int i = 0; i < 4; i++)
			for(int j = 0; j < 4; j++)
			{
				if (yPos+i < 0) continue;
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

		for(int i = 0; i < 4; i++)
			for(int j = 0; j < 4; j++)
				if(collider[i][j] != NO_PIECE)
				{
					if (i+yPos-1 >= ROWS) continue;
					// Y position going outside of field means game over condition
					if (i+yPos-1 < 0) return false;
					// Place the piece one block higher than where it collided
					field[yPos+i-1][xPos+j] = collider[i][j];
				}

		return true;
	}

	PieceType collider[4][4] = { {}, {}, {}, {} };
};