#pragma once

const int SIZE = 9;
const bool BACK = false;
const bool NEXT = true;

typedef struct {
  int row, col;
  int cells[SIZE][SIZE];
  bool fixed[SIZE][SIZE];
  bool given[SIZE][SIZE];
} Sudoku;

Sudoku emptyGrid();

bool checkRow(Sudoku s, int row, int col);
bool checkCol(Sudoku s, int row, int col);
bool checkBlock(Sudoku s, int row, int col);
bool checkCell(Sudoku s, int row, int col);
int step(Sudoku * s, bool forward);
bool isComplete(Sudoku s);
bool resolve(Sudoku * s);
bool place(Sudoku * s, int row, int col, int value);

void displaySudoku(Sudoku s);
Sudoku readGrid();
// Grids are read and written as one line of 81 characters, row after row, a
// digit for a clue and 0 for an empty cell. Reading also takes the dot other
// programs use for an empty cell, and skips the comment lines of a file that
// holds several grids.
Sudoku loadGrid(const char * path);
bool saveGrid(Sudoku s, const char * path);
