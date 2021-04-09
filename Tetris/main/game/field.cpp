/*
	Author: Mihai Daniel Ivanescu, Coventry University
 */

#include <game/field.h>

#include <memory/alloc.h>
#include <util/log.h>
#include <util/numeric.h>

static const char *TAG = "field";

Field::Field(VGAExtended *vga) : vga{vga}
{
	field = heap_caps_malloc_cast<PieceType*>(MALLOC_CAP_PREFERRED, ROWS);
	for(int i = 0; i < ROWS; i++) field[i] = (PieceType*)heap_caps_calloc(COLUMNS, sizeof(PieceType), MALLOC_CAP_PREFERRED);

	movingPiece = createPiece((PieceType)(rand()%7+1));

	nextPiece = (PieceType)(rand()%7+1);
}

void Field::drawBox(int row, int column, int variation)
{
	vga->rect(xBase + column*8, yBase + row*8, 7, 7, fieldColors[variation]);
}

void Field::drawCheckered(int row, int column, int variation)
{
	drawBox(row, column, variation);

	uint8_t x = xBase + column*8;
	uint8_t y = yBase + row*8;
	VGAColor c = fieldColors[(variation+1)%3];

	for(int i = 0; i < 5; i+=2)
		for(int j = 0; j < 5; j+=2)
			vga->dotFast(x+1 + i, y+1 + j, c);
}

void Field::drawWaves(int row, int column, int variation)
{
	drawBox(row, column, variation);

	uint8_t x = xBase + column*8;
	uint8_t y = yBase + row*8;

	vga->line(x+1, y+1, x+5, y+5, fieldColors[(variation+1) % 3]);
	vga->line(x+3, y+1, x+5, y+3, fieldColors[(variation+2) % 3]);
	vga->line(x+1, y+3, x+3, y+5, fieldColors[(variation+2) % 3]);
	vga->dotFast(x+1, y+5, fieldColors[(variation+1)%3]);
	vga->dotFast(x+5, y+1, fieldColors[(variation+1)%3]);
}

void Field::drawIce(int row, int column, int variation)
{
	drawBox(row, column, variation);

	uint8_t x = xBase + column*8;
	uint8_t y = yBase + row*8;

	vga->line(x+1, y+3, x+3, y+1, fieldColors[(variation+2) % 3]);
	vga->line(x+3, y+5, x+5, y+3, fieldColors[(variation+2) % 3]);
}

void Field::drawField()
{
	vga->clear(BLACK);

	// Draw some "padding" on the grid
	for(int i = 0; i < ROWS+1; i++)
		for(int j = -1; j < COLUMNS; j++)
			vga->dotFast(xBase+j*8+7, yBase+i*8-1, CREAM);

	for(int i = 0; i < ROWS; i++)
		for(int j = 0; j < COLUMNS; j++)
		{
			switch (field[i][j])
			{
				case NO_PIECE: break;

				case T_PIECE:
				{
					drawCheckered(i, j, 2);
					break;
				}

				case SQUARE_PIECE:
				{
					drawCheckered(i, j, 1);
					break;
				}

				case L_PIECE:
				{
					drawCheckered(i, j, 0);
					break;
				}

				case L_PIECE_B:
				{
					drawWaves(i, j, 1);
					break;
				}

				case S_PIECE:
				{
					drawWaves(i, j, 0);
					break;
				}

				case S_PIECE_B:
				{
					drawIce(i, j, 1);
					break;
				}
				
				case LINE_PIECE:
				{
					drawIce(i, j, 2);
					break;
				}
			}
		}
	
	if (movingPiece) movingPiece->draw();

	if (isClearingLines && getMillis() - timeClearStart < 500)
	{
		int t = getMillis() - timeClearStart;

		for(int i = 0; i < numLinesToClear; i++)
		{
			int y = yBase + linesToClear[i]*8;
			vga->fillRect(xBase + (500-t)/1000.f*COLUMNS*8, y, t/500.f*COLUMNS*8, 8, fieldColors[rand()%3]);
		}
	}
}

void Field::scanForLines()
{
	for(int i = 0; i < ROWS; i++)
	{
		// Assume the current line needs clearing
		bool isLine = true;

		// Line doesn't need clearing if an empty space was found
		for(int j = 0; j < COLUMNS; j++)
		{
			if(field[i][j] == NO_PIECE)
			{
				isLine = false;
				break;
			}
		}
		
		if (isLine)
		{
			linesToClear[numLinesToClear] = i;
			numLinesToClear++;
		}
	}
}

void Field::clearField()
{
	for(int i = 0; i < ROWS; i++)
		for(int j = 0; j < COLUMNS; j++)
			field[i][j] = NO_PIECE;
	
	heap_caps_free(movingPiece);
	movingPiece = createPiece(nextPiece);
	nextPiece = (PieceType)(rand()%7+1);

	speedTimerMax = speedTimer = 75;
}

int Field::updateField()
{
	if (movingPiece->state == Piece::PLACED)
	{
		heap_caps_free(movingPiece);
		movingPiece = createPiece(nextPiece);
		nextPiece = (PieceType)(rand()%7+1);
	}
	else if (movingPiece->state == Piece::OVERFLOWED) return -1;

	// Start the clearing lines state if there are any lines that need clearing
	if (!isClearingLines)
	{
		scanForLines();
		if (numLinesToClear > 0)
		{
			timeClearStart = getMillis();
			isClearingLines = true;

			return numLinesToClear;
		}
	}
	else
	{
		// Stop the clearing lines state after a certain amount of time passes
		if (getMillis() - timeClearStart < 500) return 0;
		else
		{
			isClearingLines = false;

			// Bring all rows above the cleared line down
			for(int i = 0; i < numLinesToClear; i++)
			{
				int row = linesToClear[i];
				for(int j = row; j > 0; j--)
					for (int k = 0; k < COLUMNS; k++)
						field[j][k] = field[j-1][k];
			}

			// Clear the topmost row
			for(int j = 0; j < COLUMNS; j++)
				field[0][j] = NO_PIECE;

			numLinesToClear = 0;
		}
	}

	if (isKeyPressed(Down_key)) movingPiece->moveDown();

	if (isKeyPressed(Left_key))
	{
		movingPiece->moveLeft();
		movePieceTimer = 0;
	}
	if (isKeyPressed(Right_key))
	{
		movingPiece->moveRight();
		movePieceTimer = 0;
	}

	movePieceTimer++;
	if (movePieceTimer % 4 == 0)
	{
		if (isKeyHeld(Down_key)) movingPiece->moveDown();

		if (movePieceTimer > 8)
		{
			if (isKeyHeld(Left_key)) movingPiece->moveLeft();
			if (isKeyHeld(Right_key)) movingPiece->moveRight();
		}
	}

	if (isKeyPressed(Space_key) || isKeyPressed(Up_key)) movingPiece->rotate();

	if (--speedTimer == 0)
	{
		movingPiece->moveDown();
		speedTimer = speedTimerMax;
	}

	return 0;
}

void Field::advanceLevel()
{
	speedTimerMax = speedTimerMax * 0.85;
	speedTimer = speedTimerMax;

	fieldColors[0] = (VGAColor)(fieldColors[0] + 8);
	fieldColors[1] = (VGAColor)(fieldColors[1] + 8);
	fieldColors[2] = (VGAColor)(fieldColors[2] + 8);
}

Piece* Field::createPiece(PieceType pieceType)
{
	Piece *piece = NULL;
	switch (pieceType)
	{
		case T_PIECE:
		{
			piece = heap_caps_malloc_construct<TPiece, VGAExtended*, PieceType**>(MALLOC_CAP_PREFERRED, vga, field);
			break;
		}

		case S_PIECE:
		{
			piece = heap_caps_malloc_construct<SPiece, VGAExtended*, PieceType**>(MALLOC_CAP_PREFERRED, vga, field);
			break;
		}

		case S_PIECE_B:
		{
			piece = heap_caps_malloc_construct<SPieceB, VGAExtended*, PieceType**>(MALLOC_CAP_PREFERRED, vga, field);
			break;
		}

		case L_PIECE:
		{
			piece = heap_caps_malloc_construct<LPiece, VGAExtended*, PieceType**>(MALLOC_CAP_PREFERRED, vga, field);
			break;
		}

		case L_PIECE_B:
		{
			piece = heap_caps_malloc_construct<LPieceB, VGAExtended*, PieceType**>(MALLOC_CAP_PREFERRED, vga, field);
			break;
		}

		case SQUARE_PIECE:
		{
			piece = heap_caps_malloc_construct<SquarePiece, VGAExtended*, PieceType**>(MALLOC_CAP_PREFERRED, vga, field);
			break;
		}

		case LINE_PIECE:
		{
			piece = heap_caps_malloc_construct<LinePiece, VGAExtended*, PieceType**>(MALLOC_CAP_PREFERRED, vga, field);
			break;
		}

		default: break;
	}
	return piece;
}