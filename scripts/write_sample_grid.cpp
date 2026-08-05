// Ecrit une grille generee dans grille.sdk pour la capture d'ecran.
#include "generator.h"
#include "sudoku_grid.h"

#include <cstdio>

int main(){
  Sudoku s = genererGrille(MOYEN);
  if(!sauvegarderGrille(s, "grille.sdk")){
    std::fprintf(stderr, "impossible d'ecrire grille.sdk\n");
    return 1;
  }
  return 0;
}
