// Controle temporaire du generateur : regles, unicite, alea.
#include "generator.h"
#include "alea.h"
#include "sudoku_grid.h"

#include <chrono>
#include <cstdio>
#include <set>

static bool reglesRespectees(const Sudoku & s){
  for(int i = 0; i < TAILLE; i++){
    for(int j = 0; j < TAILLE; j++){
      int nb = s.grille[i][j];
      if(nb == 0)
	continue;
      Sudoku copie = s;
      if(!tester(copie, i, j))
	return false;
    }
  }
  return true;
}

static int compterIndices(const Sudoku & s){
  int nb = 0;
  for(int i = 0; i < TAILLE; i++)
    for(int j = 0; j < TAILLE; j++)
      if(s.grille[i][j] != 0)
	nb++;
  return nb;
}

static void testerNiveau(Difficulte niveau, int cible, int essais){
  printf("Niveau %d (%d essais)\n", (int) niveau, essais);
  for(int i = 0; i < essais; i++){
    auto debut = std::chrono::steady_clock::now();
    Sudoku s = genererGrille(niveau);
    auto fin = std::chrono::steady_clock::now();
    double ms = std::chrono::duration<double, std::milli>(fin - debut).count();

    int indices = compterIndices(s);
    int solutions = compterSolutions(s, 2);
    bool regles = reglesRespectees(s);

    printf("  essai %d : %d indices, %d solution(s), regles=%s, %.1f ms\n",
	   i + 1, indices, solutions, regles ? "ok" : "KO", ms);

    if(!regles || solutions != 1){
      afficherSudoku(s);
      return;
    }
    if(indices > cible + 5)
      printf("  (avertissement : au-dessus de la cible %d)\n", cible);
  }
}

static void testerHasard(){
  std::set<int> vus;
  for(int i = 0; i < 500; i++){
    int v = hasard(8);
    if(v < 0 || v > 8){
      printf("hasard(8) hors bornes : %d\n", v);
      return;
    }
    vus.insert(v);
  }
  printf("hasard(8) couvre %zu valeurs sur 9\n", vus.size());
}

static void testerDiversite(){
  Sudoku a = genererGrille(MOYEN);
  Sudoku b = genererGrille(MOYEN);
  bool identiques = true;
  for(int i = 0; i < TAILLE && identiques; i++)
    for(int j = 0; j < TAILLE && identiques; j++)
      if(a.grille[i][j] != b.grille[i][j])
	identiques = false;
  printf("deux grilles moyennes identiques : %s\n", identiques ? "oui" : "non");
}

int main(){
  testerHasard();
  testerDiversite();
  testerNiveau(FACILE, 45, 3);
  testerNiveau(MOYEN, 34, 3);
  testerNiveau(DIFFICILE, 28, 3);
  return 0;
}
