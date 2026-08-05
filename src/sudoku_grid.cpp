//------------------------------------------------------//
// sudoku_grid.cpp
// Sudoku structure and functions to solve and generate
// sudoku grids
//-----------------------------------------------------//
// Author: Natim
// Last modified: 12-05-2006
//-----------------------------------------------------//

#include "sudoku_grid.h"

#include <iostream>
#include <fstream>
#include <string>

using namespace std;

Sudoku emptyGrid(){
  Sudoku s;
  int i, j;
  s.row = 0;
  s.col = 0;

  for(i = 0; i < SIZE; i++)
    for(j = 0; j < SIZE; j++){
      s.cells[i][j] = 0;
      s.fixed[i][j] = false;
      s.given[i][j] = false;
    }
  return s;
}

bool checkRow(Sudoku s, int row, int col){
  int i;

  for(i = 0; i < SIZE; i++)
    if(i != col)
      if(s.cells[row][i] == s.cells[row][col])
	return false;

  return true;
}

bool checkCol(Sudoku s, int row, int col){
  int i;

  for(i = 0; i < SIZE; i++)
    if(i != row)
      if(s.cells[i][col] == s.cells[row][col])
	return false;

  return true;
}

bool checkBlock(Sudoku s, int row, int col){
  int i, j;

  int minRow = row / 3 * 3;
  int maxRow = (row / 3 + 1) * 3;
  int minCol = col / 3 * 3;
  int maxCol = (col / 3 + 1) * 3;

  for(i = minRow; i < maxRow; i++){
    for(j = minCol; j < maxCol; j++){
      if(i != row && j != col)
	if(s.cells[i][j] == s.cells[row][col])
	  return false;
    }
  }
  return true;
}

bool checkCell(Sudoku s, int row, int col){
  if(checkRow(s, row, col) && checkCol(s, row, col) && checkBlock(s, row, col))
    return true;
  else
    return false;
}

int step(Sudoku * s, bool forward){
  if(forward){
    s->row++;
    if(s->row > 8){
      s->row = 0;
      if(s->col == 8) return 1;
      else s->col++;
    }
  }else{
    s->row--;
    if(s->row < 0){
      s->row = 8;
      if(s->col == 0) return -1;
      else s->col--;
    }
  }
  return 0;
}

bool isComplete(Sudoku s){
  int i, j;
  for(i = 0; i < SIZE; i++)
    for(j = 0; j < SIZE; j++)
      if(s.cells[i][j] == 0)
	return false;
  return true;
}

bool resolve(Sudoku * s){
  int loops = 0;
  int noSolution = 0;

  while(!isComplete(*s) && noSolution == 0){
    if(s->cells[s->row][s->col] == 0)
      s->cells[s->row][s->col] = 1;

    while(!checkCell(*s, s->row, s->col)){
      if(s->cells[s->row][s->col] == 0)
	s->cells[s->row][s->col] = 1;
      else if(!s->fixed[s->row][s->col])
	s->cells[s->row][s->col]++;
      else if(s->fixed[s->row][s->col]){
	while(s->fixed[s->row][s->col])
	    noSolution = step(s, BACK);
	s->cells[s->row][s->col]++;
      }

      while(s->cells[s->row][s->col] > 9){
	s->cells[s->row][s->col] = 0;
	noSolution = step(s, BACK);

	while(s->fixed[s->row][s->col])
	    noSolution = step(s, BACK);
	s->cells[s->row][s->col]++;
      }
    }
    if(noSolution != -1)
	noSolution = step(s, NEXT);
    loops++;
  }
  cerr << loops << endl;
  if(noSolution == -1)
    return false;
  else
    return true;
}

bool place(Sudoku * s, int row, int col, int value){
  int previous = 0;

  if(row >= 0 && row < 9 && col >= 0 && col < 9 && value >= 0 && value < 10){
    if(s->given[row][col])
      return false;

    previous = s->cells[row][col];
    s->cells[row][col] = value;
    s->fixed[row][col] = (value != 0);

    if(value != 0){
      if(!checkCell(*s, row, col)){
	s->cells[row][col] = previous;
	s->fixed[row][col] = false;
	return false;
      }
    }
  }else
    return false;
  return true;
}

void displaySudoku(Sudoku s){
  int i, j;

  cerr << ",-----------," << endl;

  for(i = 0; i < 9; i++){
    for(j = 0; j < 9; j++){
      if(j == 0)
	cerr << "|" << flush;
      if(s.cells[i][j] == 0)
	cerr << " " << flush;
      else
	cerr << s.cells[i][j] << flush;

      if(j == 2)
	cerr << "|" << flush;
      if(j == 5)
	cerr << "|" << flush;
      if(j == 8)
	cerr << "|" << endl;
    }

    if(i == 2 || i == 5)
      cerr << "|---|---|---|" << endl;
  }
  cerr << "'-----------'" << endl;
}

Sudoku readGrid(){
  Sudoku s;
  s = emptyGrid();
  int i, j, value;
  for(i = 0; i < SIZE; i++)
    for(j = 0; j < SIZE; j++){
      cin >> value;
      place(&s, i, j, value);
    }
  displaySudoku(s);
  return s;
}

// The grid on one line, one character per cell and row after row, as sudoku
// programs pass puzzles around. A digit is a clue, 0 or a dot an empty cell.
static const int LINE_LENGTH = SIZE * SIZE;

static bool gridFromLine(const string & line, Sudoku * s){
  Sudoku read = emptyGrid();

  for(int i = 0; i < LINE_LENGTH; i++){
    const char c = line[i];
    if(c == '0' || c == '.')
      continue;
    if(c < '1' || c > '9')
      return false;

    // A digit the rules refuse leaves the cell empty rather than fixed on a
    // value the player could never have reached.
    const int row = i / SIZE, col = i % SIZE;
    if(place(&read, row, col, c - '0')){
      read.fixed[row][col] = true;
      read.given[row][col] = true;
    }
  }

  *s = read;
  return true;
}

Sudoku loadGrid(const char * path){
  ifstream file;
  string line;

  file.open(path, ios::in);
  if(!file.good())
    return emptyGrid();

  // A file holds one grid per line, and comments opening on a hash: read the
  // first line long enough to be a grid.
  Sudoku s = emptyGrid();
  while(getline(file, line))
    if(line.size() >= (size_t) LINE_LENGTH && line[0] != '#')
      if(gridFromLine(line, &s))
	break;

  file.close();
  return s;
}

bool saveGrid(Sudoku s, const char * path){
  ofstream file;
  int i, j;

  file.open(path, ios::out);
  if(file.good()){
    for(i = 0; i < SIZE; i++)
      for(j = 0; j < SIZE; j++)
	file << (char) ('0' + s.cells[i][j]);
    file << endl;
  }
  file.close();
  return file.good();
}
