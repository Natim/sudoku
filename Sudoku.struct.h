//------------------------------------------------------//
// Sudoku.struct.cpp
// Structure Sudoku et ses fonctions pour résoudre et 
// générer des grilles de sudoku
//-----------------------------------------------------//
// Auteur : Natim
// Date de dernière modification : 12-05-2006
//-----------------------------------------------------//

#include <iostream>
#include <cstdlib>
#include <ctime>
#include <fstream>

using namespace std;

const int TAILLE = 9;
const bool BACK  = false;
const bool NEXT  = true;

typedef struct{
  int ligne, colonne;
  int grille[TAILLE][TAILLE];
  bool fixe[TAILLE][TAILLE];
} Sudoku;

/***************************
 ** Création de Sudoku de **
 ** différents types      **
 ***************************/
Sudoku initGrille();
/* Initialise une grille vierge de Sudoku */

/*****************************
 ** Fonction de résolutions **
 *****************************/
bool testerL(Sudoku s, int lig, int col);
/* Teste si le nombre posx, posy n'est pas déjà dans la ligne */

bool testerC(Sudoku s, int lig, int col);
/* Teste si le nombre posx, posy n'est pas déjà dans la colonne */

bool testerR(Sudoku s, int lig, int col);
/* Teste si le nombre posx, posy n'est pas déjà dans la région */

bool tester(Sudoku s, int lig, int col);
/* Test si on peut placer le nombre posx,posy à cette place */

int go(Sudoku * s, bool sens);
/* 
   Utilisé par la fonction resolve,
   Cette fonction avance ou recule d'une case
*/

bool fin(Sudoku s);
/*
  Retourne true si le Sudoku est résolu
*/

bool resolve(Sudoku * s);
/* 
   Résoud le Sudoku s 
   Retourne true s'il y a une solution et false sinon
*/
/*************************
 ** Fonction de plaçage **
 *************************/
bool placer(Sudoku * s, int lig, int col, int nb);
/* Essaye de placer le nombre, nb, à la position, pos, du Sudoku, s. */


void afficherSudoku(Sudoku s);

int hasard(int nb);
Sudoku lireGrille();

Sudoku chargerGrille(char * chemin);
bool sauvegarderGrille(Sudoku s, char * chemin);
