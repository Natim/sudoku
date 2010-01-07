//------------------------------------------------------//
// Sudoku.struct.cpp
// Structure Sudoku et ses fonctions pour résoudre et 
// générer des grilles de sudoku
//-----------------------------------------------------//
// Auteur : Natim
// Date de dernière modification : 12-05-2006
//-----------------------------------------------------//

#include "Sudoku.struct.h"

/***************************
 ** Création de Sudoku de **
 ** différents types      **
 ***************************/
Sudoku initGrille(){
/* Initialise une grille vierge de Sudoku */
  Sudoku s;
  int i, j;
  s.ligne   = 0;// hasard(8);
  s.colonne = 0;// hasard(8);

  for(i = 0; i < TAILLE; i++)
    for(j = 0; j < TAILLE; j++){
      s.grille[i][j] = 0;
      s.fixe[i][j]   = false;
    }
  return s;
}

/*****************************
 ** Fonction de résolutions **
 *****************************/
bool testerL(Sudoku s, int lig, int col){
  /* Teste si le nombre posx, posy n'est pas déjà dans la ligne */
  int i;
  
  for(i = 0; i < TAILLE; i++)
    if(i != col)
      if(s.grille[lig][i] == s.grille[lig][col])
	return false;
  
  return true;
}


bool testerC(Sudoku s, int lig, int col){
/* Teste si le nombre posx, posy n'est pas déjà dans la colonne */
  int i;
  
  for(i = 0; i < TAILLE; i++)
    if(i != lig)
      if(s.grille[i][col] == s.grille[lig][col])
	return false;
  
  return true;
}

bool testerR(Sudoku s, int lig, int col){
/* Teste si le nombre posx, posy n'est pas déjà dans la région */
  int i, j;

  int minLig = lig/3 * 3;
  int maxLig = (lig/3 + 1) * 3;
  int minCol = col/3 * 3;
  int maxCol = (col/3 + 1) * 3;
  
  for(i = minLig; i < maxLig; i++){
    for(j = minCol; j < maxCol; j++){
      if(i != lig && j != col)
	if(s.grille[i][j] == s.grille[lig][col])
	  return false;
    }
  }  
  return true;
}

bool tester(Sudoku s, int lig, int col){
/* Test si on peut placer le nombre posx,posy à cette place */
  if (testerL(s, lig, col) && testerC(s, lig, col) && testerR(s, lig, col))
    return true;
  else
    return false;
}

int go(Sudoku * s, bool sens){
/* 
   Utilisé par la fonction resolve,
   Cette fonction avance ou recule d'une case
*/
  if(sens){
    s->ligne++;
    if(s->ligne > 8){
      s->ligne = 0;
      if(s->colonne == 8) return 1; /* VICTOIRE */
      else s->colonne++;
    }
  }else{
    s->ligne--;
    if(s->ligne < 0){
      s->ligne = 8;
      if(s->colonne == 0) return -1; /* IMPOSSIBLE */
      else s->colonne--;
    }
  }
  return 0;
}

bool fin(Sudoku s){
/*
  Retourne true si le Sudoku est résolu
*/
  int i, j;
  for(i = 0; i < TAILLE; i++)
    for(j = 0; j < TAILLE; j++)
      if(s.grille[i][j] == 0)
	return false;
  return true;
}
bool resolve(Sudoku * s){
/* 
   Résoud le Sudoku s 
   Retourne true s'il y a une solution et false sinon
*/
  int nbBoucles = 0;
  int noSolution = 0;

  while(!fin(*s) && noSolution == 0){
    if(s->grille[s->ligne][s->colonne] == 0)
      s->grille[s->ligne][s->colonne] = 1;

    while(!tester(*s, s->ligne, s->colonne)){
      if(s->grille[s->ligne][s->colonne] == 0)
	s->grille[s->ligne][s->colonne] = 1;
      else if(!s->fixe[s->ligne][s->colonne])
	s->grille[s->ligne][s->colonne]++;
      else if(s->fixe[s->ligne][s->colonne]){
	while(s->fixe[s->ligne][s->colonne])
	    noSolution = go(s, BACK);
	s->grille[s->ligne][s->colonne]++;
      }

      while(s->grille[s->ligne][s->colonne] > 9){
	s->grille[s->ligne][s->colonne] = 0;
	noSolution = go(s, BACK);
	
	while(s->fixe[s->ligne][s->colonne])
	    noSolution = go(s, BACK);
	s->grille[s->ligne][s->colonne]++;
      }
    }
    if(noSolution != -1)
	noSolution = go(s, NEXT);
    nbBoucles++;
  }
  cerr << nbBoucles << endl;
  if(noSolution == -1)
    return false;
  else
    return true;
}
/*************************
 ** Fonction de plaçage **
 *************************/
bool placer(Sudoku * s, int lig, int col, int nb){
/* Essaye de placer le nombre, nb, à la position, pos, du Sudoku, s. */
  int temp = 0;

  if(lig >= 0 && lig < 9 && col >= 0 && col < 9 && nb >= 0 && nb < 10){
    temp = s->grille[lig][col];
    s->grille[lig][col] = nb;
    s->fixe[lig][col] = (nb != 0);

    if(nb != 0){
      if(!tester(*s, lig, col)){
	s->grille[lig][col] = temp;
	s->fixe[lig][col]   = false;
	return false;
      }
    }
  }else
    return false;
  return true;
}

void afficherSudoku(Sudoku s){
  int i, j;
  
  cerr << ",-----------," << endl;
    
  for(i = 0; i < 9; i++){        
    for(j = 0; j < 9; j++){
      if(j == 0)
	cerr << "|" << flush;
      if(s.grille[i][j] == 0)
	cerr << " " << flush;
      else
	cerr << s.grille[i][j] << flush;
                            
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

int hasard(int nb){
/* Retourne un nombre entre 0 et nb */
  return (int) ((float) rand() / RAND_MAX * (nb + 1));
}

Sudoku lireGrille(){
  Sudoku s;
  s = initGrille();
  int i, j, nb;
  for(i = 0; i < TAILLE; i++)
    for(j = 0; j < TAILLE; j++){
      cin >> nb;
      placer(&s, i, j, nb);
    }
  afficherSudoku(s);
  return s;
}

Sudoku chargerGrille(char * chemin){
  ifstream fic;
  int code;
  int i, j;

  Sudoku s = initGrille();

  fic.open(chemin, ios::in);
  if(fic.good()){
    for(i = 0; i < 9; i++)
      for(j = 0; j < 9; j++)
	if(!fic.eof()){
	  fic >> code;
	  !placer(&s, i, j, code);
	}
    return s;
  }else
    return initGrille();
  fic.close();
}

bool sauvegarderGrille(Sudoku s, char * chemin){
  ofstream fic;
  int i, j;
  fic.open(chemin, ios::out);
  if(fic.good())
    for(i = 0; i < 9; i++)
      for(j = 0; j < 9; j++)
	fic << s.grille[i][j] << endl;
  fic.close();
  return fic.good();
}
