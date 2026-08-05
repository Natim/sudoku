// Write a generated grid to grille.sdk for screenshots.
#include "generator.h"
#include "sudoku_grid.h"

#include <cstdio>

int main(){
  Sudoku s = generateGrid(MEDIUM);
  if(!saveGrid(s, "grille.sdk")){
    std::fprintf(stderr, "cannot write grille.sdk\n");
    return 1;
  }
  return 0;
}
