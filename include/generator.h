#pragma once

#include "sudoku_grid.h"

enum Difficulty { EASY, MEDIUM, HARD };

// Playable grid: valid, unique solution, clues marked fixed and given.
Sudoku generateGrid(Difficulty level);

// Number of solutions, capped at limit (2 is enough to test uniqueness).
int countSolutions(Sudoku s, int limit);
