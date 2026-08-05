#pragma once

#include "score.h"

// Top left corner of a cell of the grid, which the main program lays out.
typedef void (*CellLocator)(int row, int col, int * x, int * y);

const int PANEL_WIDTH  = 400;
const int PANEL_HEIGHT = 240;

// Sweep the finished grid, then show what the player earned in a panel drawn at
// (x, y). Returns true when the player asks for another grid.
bool celebrate(int x, int y, CellLocator locate, int cellSize, const Score & score);
