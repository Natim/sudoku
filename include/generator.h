#pragma once

#include "sudoku_grid.h"

enum Difficulte { FACILE, MOYEN, DIFFICILE };

// Grille jouable : valide, a solution unique, indices marques fixe et donnee.
Sudoku genererGrille(Difficulte niveau);

// Nombre de solutions, plafonne a limite (2 suffit pour tester l'unicite).
int compterSolutions(Sudoku s, int limite);
