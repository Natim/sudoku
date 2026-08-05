#pragma once

#include "sudoku_grid.h"

enum Difficulty { EASY, MEDIUM, HARD };

// Playable grid: valid, unique solution, clues marked fixed and given.
Sudoku generateGrid(Difficulty level);

// Number of solutions, capped at limit (2 is enough to test uniqueness).
int countSolutions(Sudoku s, int limit);

// How much work the grid demands: the empty cells to fill plus the digits a
// constraint solver has to try where elimination alone does not decide. A grid
// without a unique solution is not a puzzle and scores 0.
int gridComplexity(Sudoku s);
