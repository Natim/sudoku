#include "generator.h"
#include "alea.h"

#include <algorithm>
#include <vector>

static const int NB_CASES = TAILLE * TAILLE;
static const unsigned TOUS_CHIFFRES = 0x1FF;

struct Etat {
  int val[NB_CASES];
  unsigned libLigne[TAILLE];
  unsigned libCol[TAILLE];
  unsigned libBloc[TAILLE];
};

static int indice(int lig, int col){
  return lig * TAILLE + col;
}

static int bloc(int lig, int col){
  return (lig / 3) * 3 + col / 3;
}

static void initEtatVide(Etat & e){
  for(int i = 0; i < NB_CASES; i++)
    e.val[i] = 0;
  for(int i = 0; i < TAILLE; i++){
    e.libLigne[i] = TOUS_CHIFFRES;
    e.libCol[i]   = TOUS_CHIFFRES;
    e.libBloc[i]  = TOUS_CHIFFRES;
  }
}

static void etatDepuisSudoku(const Sudoku & s, Etat & e){
  initEtatVide(e);
  for(int i = 0; i < TAILLE; i++){
    for(int j = 0; j < TAILLE; j++){
      int nb = s.grille[i][j];
      if(nb == 0)
	continue;
      int p = indice(i, j);
      int b = bloc(i, j);
      e.val[p] = nb;
      unsigned masque = ~(1u << (nb - 1));
      e.libLigne[i] &= masque;
      e.libCol[j]   &= masque;
      e.libBloc[b]  &= masque;
    }
  }
}

static unsigned candidats(const Etat & e, int p){
  int lig = p / TAILLE;
  int col = p % TAILLE;
  return e.libLigne[lig] & e.libCol[col] & e.libBloc[bloc(lig, col)];
}

static void poser(Etat & e, int p, int nb){
  int lig = p / TAILLE;
  int col = p % TAILLE;
  int b = bloc(lig, col);
  e.val[p] = nb;
  unsigned masque = ~(1u << (nb - 1));
  e.libLigne[lig] &= masque;
  e.libCol[col]   &= masque;
  e.libBloc[b]    &= masque;
}

static void retirer(Etat & e, int p){
  int nb = e.val[p];
  if(nb == 0)
    return;
  int lig = p / TAILLE;
  int col = p % TAILLE;
  int b = bloc(lig, col);
  e.val[p] = 0;
  unsigned bit = 1u << (nb - 1);
  e.libLigne[lig] |= bit;
  e.libCol[col]   |= bit;
  e.libBloc[b]    |= bit;
}

static int caseMRV(const Etat & e){
  int meilleure = -1;
  int minCand = 10;
  for(int p = 0; p < NB_CASES; p++){
    if(e.val[p] != 0)
      continue;
    unsigned cand = candidats(e, p);
    if(cand == 0)
      return p;
    int nbCand = 0;
    for(unsigned bit = cand; bit != 0; bit &= bit - 1)
      nbCand++;
    if(nbCand < minCand){
      minCand = nbCand;
      meilleure = p;
    }
  }
  return meilleure;
}

static void remplirCandidats(unsigned cand, int * chiffres, int & nb){
  nb = 0;
  for(int d = 1; d <= 9; d++){
    if(cand & (1u << (d - 1)))
      chiffres[nb++] = d;
  }
}

static int compterRec(Etat & e, int limite){
  int p = caseMRV(e);
  if(p < 0)
    return 1;
  if(candidats(e, p) == 0)
    return 0;

  int total = 0;
  unsigned cand = candidats(e, p);
  for(int d = 1; d <= 9; d++){
    if((cand & (1u << (d - 1))) == 0)
      continue;
    poser(e, p, d);
    total += compterRec(e, limite - total);
    retirer(e, p);
    if(total >= limite)
      return total;
  }
  return total;
}

static bool remplirRec(Etat & e){
  int p = caseMRV(e);
  if(p < 0)
    return true;
  unsigned cand = candidats(e, p);
  if(cand == 0)
    return false;

  int chiffres[9];
  int nb;
  remplirCandidats(cand, chiffres, nb);
  melanger(chiffres, nb);

  for(int i = 0; i < nb; i++){
    poser(e, p, chiffres[i]);
    if(remplirRec(e))
      return true;
    retirer(e, p);
  }
  return false;
}

static bool remplirSolution(Etat & e){
  initEtatVide(e);
  return remplirRec(e);
}

static int compterEtat(Etat e, int limite){
  return compterRec(e, limite);
}

static int compterIndices(const Etat & e){
  int nb = 0;
  for(int p = 0; p < NB_CASES; p++)
    if(e.val[p] != 0)
      nb++;
  return nb;
}

static int cibleIndices(Difficulte niveau){
  switch(niveau){
  case FACILE:    return 45;
  case MOYEN:     return 34;
  case DIFFICILE: return 28;
  }
  return 34;
}

static Etat creuser(Etat e, int cible){
  int paires[41];
  for(int i = 0; i < 41; i++)
    paires[i] = i;
  melanger(paires, 41);

  for(int k = 0; k < 41; k++){
    if(compterIndices(e) <= cible)
      break;

    int p = paires[k];
    int q = 80 - p;
    int sauvP = e.val[p];
    int sauvQ = e.val[q];

    retirer(e, p);
    if(p != q)
      retirer(e, q);

    if(compterEtat(e, 2) == 1)
      continue;

    if(sauvP != 0)
      poser(e, p, sauvP);
    if(p != q && sauvQ != 0)
      poser(e, q, sauvQ);
  }
  return e;
}

static Sudoku sudokuDepuisEtat(const Etat & e){
  Sudoku s = initGrille();
  for(int i = 0; i < TAILLE; i++){
    for(int j = 0; j < TAILLE; j++){
      int nb = e.val[indice(i, j)];
      if(nb == 0)
	continue;
      s.grille[i][j] = nb;
      s.fixe[i][j]   = true;
      s.donnee[i][j] = true;
    }
  }
  return s;
}

int compterSolutions(Sudoku s, int limite){
  if(limite <= 0)
    return 0;
  Etat e;
  etatDepuisSudoku(s, e);
  return compterEtat(e, limite);
}

Sudoku genererGrille(Difficulte niveau){
  int cible = cibleIndices(niveau);
  Sudoku meilleure = initGrille();
  int meilleurNb = NB_CASES + 1;

  for(int essai = 0; essai < 3; essai++){
    Etat solution;
    if(!remplirSolution(solution))
      continue;

    Etat creusee = creuser(solution, cible);
    int nb = compterIndices(creusee);
    if(nb < meilleurNb){
      meilleurNb = nb;
      meilleure = sudokuDepuisEtat(creusee);
      if(nb <= cible)
	break;
    }
  }
  return meilleure;
}
