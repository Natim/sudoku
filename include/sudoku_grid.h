#pragma once

const int TAILLE = 9;
const bool BACK = false;
const bool NEXT = true;

typedef struct {
  int ligne, colonne;
  int grille[TAILLE][TAILLE];
  bool fixe[TAILLE][TAILLE];
  bool donnee[TAILLE][TAILLE];
} Sudoku;

Sudoku initGrille();

bool testerL(Sudoku s, int lig, int col);
bool testerC(Sudoku s, int lig, int col);
bool testerR(Sudoku s, int lig, int col);
bool tester(Sudoku s, int lig, int col);
int go(Sudoku * s, bool sens);
bool fin(Sudoku s);
bool resolve(Sudoku * s);
bool placer(Sudoku * s, int lig, int col, int nb);

void afficherSudoku(Sudoku s);
Sudoku lireGrille();
Sudoku chargerGrille(const char * chemin);
bool sauvegarderGrille(Sudoku s, const char * chemin);
