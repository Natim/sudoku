// Temporary check of the winning sequence: run it with a time in seconds, and
// optionally a complexity, to see the wave and the panel that follows.
extern "C"{
#include "graphlib.h"
}
#include "bitmap.h"
#include "celebration.h"
#include "generator.h"
#include "score.h"
#include "sudoku_grid.h"
#include "window.h"

#include <cstdlib>
#include <string>

#ifndef SUDOKU_ASSETS_DIR
#define SUDOKU_ASSETS_DIR "assets"
#endif

static const int GRID_X = 50;
static const int GRID_Y = 74;
static const int CELL_SIZE = 32;
static const int WINDOW_SIZE = 600;

static Bitmap * up[10];
static Bitmap * down[10];
static Sudoku grid;

static void cellPosition(int row, int col, int * x, int * y){
  *x = GRID_X + (col / 3) * 182 + (col % 3) * 52;
  *y = GRID_Y + (row / 3) * 182 + (row % 3) * 52;
}

static void paintCell(int row, int col){
  int x, y;
  cellPosition(row, col, &x, &y);
  int value = grid.cells[row][col];
  if(grid.given[row][col])
    down[value]->draw(x, y);
  else
    up[value]->draw(x, y);
}

int main(int argc, char ** argv){
  const int seconds = (argc > 1) ? atoi(argv[1]) : 300;

  for(int i = 0; i < 10; i++){
    std::string base = std::string(SUDOKU_ASSETS_DIR) + "/images/" + std::to_string(i);
    up[i] = new Bitmap((base + ".bmp").c_str());
    down[i] = new Bitmap((base + "d.bmp").c_str());
  }

  grid = generateGrid(EASY);
  int complexity = (argc > 2) ? atoi(argv[2]) : gridComplexity(grid);
  resolve(&grid);

  openScalableWindow(WINDOW_SIZE, WINDOW_SIZE, "Sudoku - verification");
  for(int row = 0; row < SIZE; row++)
    for(int col = 0; col < SIZE; col++)
      paintCell(row, col);

  Score score = rateGame(complexity, seconds);
  celebrate((WINDOW_SIZE - PANEL_WIDTH) / 2, (WINDOW_SIZE - PANEL_HEIGHT) / 2,
	    cellPosition, CELL_SIZE, score);

  fermerFenetre();
  return 0;
}
