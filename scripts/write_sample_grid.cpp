// Write a generated grid to grille.sdm for screenshots.
#include "generator.h"
#include "sudoku_grid.h"

#include <cstdio>

int main(){
  Sudoku s = generateGrid(MEDIUM);
  if(!saveGrid(s, "grille.sdm")){
    std::fprintf(stderr, "cannot write grille.sdm\n");
    return 1;
  }
  return 0;
}
