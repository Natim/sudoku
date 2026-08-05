#include "alea.h"

#include <algorithm>

std::mt19937 & moteurAleatoire(){
  static std::mt19937 moteur([]{
    std::random_device rd;
    return rd();
  }());
  return moteur;
}

int hasard(int nb){
  if(nb < 0)
    return 0;
  std::uniform_int_distribution<int> tirage(0, nb);
  return tirage(moteurAleatoire());
}

void melanger(int * tab, int n){
  if(tab == NULL || n <= 1)
    return;
  for(int i = n - 1; i > 0; i--){
    int j = hasard(i);
    std::swap(tab[i], tab[j]);
  }
}
