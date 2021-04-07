/*
	Author: Mihai Daniel Ivanescu, Coventry University
	License: BSD License
 */

#pragma once

#include <game/piece.h>

class TPiece : public Piece3x3Collider
{
public:
	TPiece(VGAExtended *vga, PieceType **field) : Piece3x3Collider(vga, field)
	{
		collider[0][0] = collider[0][1] = collider[0][2] = collider[1][1] = T_PIECE;
		leftmostChunk = 0;
		rightmostChunk = 2;

		if (isColliding()) state = OVERFLOWED;
	}
};

class SPiece : public Piece3x3Collider
{
public:
	SPiece(VGAExtended *vga, PieceType **field) : Piece3x3Collider(vga, field)
	{
		collider[0][0] = collider[0][1] = collider[1][1] = collider[1][2] = S_PIECE;
		leftmostChunk = 0;
		rightmostChunk = 2;

		if (isColliding()) state = OVERFLOWED;
	}
};

class SPieceB : public Piece3x3Collider
{
public:
	SPieceB(VGAExtended *vga, PieceType **field) : Piece3x3Collider(vga, field)
	{
		collider[0][2] = collider[0][1] = collider[1][1] = collider[1][0] = S_PIECE_B;
		leftmostChunk = 0;
		rightmostChunk = 2;

		if (isColliding()) state = OVERFLOWED;
	}
};

class LPiece : public Piece3x3Collider
{
public:
	LPiece(VGAExtended *vga, PieceType **field) : Piece3x3Collider(vga, field)
	{
		collider[0][0] = collider[0][1] = collider[0][2] = collider[1][2] = L_PIECE;
		leftmostChunk = 0;
		rightmostChunk = 2;

		if (isColliding()) state = OVERFLOWED;
	}
};

class LPieceB : public Piece3x3Collider
{
public:
	LPieceB(VGAExtended *vga, PieceType **field) : Piece3x3Collider(vga, field)
	{
		collider[0][0] = collider[0][1] = collider[0][2] = collider[1][0] = L_PIECE;
		leftmostChunk = 0;
		rightmostChunk = 2;

		if (isColliding()) state = OVERFLOWED;
	}
};

class SquarePiece : public Piece3x3Collider
{
public:
	SquarePiece(VGAExtended *vga, PieceType **field) : Piece3x3Collider(vga, field)
	{
		collider[0][0] = collider[1][0] = collider[1][1] = collider[0][1] = SQUARE_PIECE;
		leftmostChunk = 0;
		rightmostChunk = 1;

		if (isColliding()) state = OVERFLOWED;
	}
};