#include "random.h"

#include <algorithm>

std::mt19937 & randomEngine(){
  static std::mt19937 engine([]{
    std::random_device rd;
    return rd();
  }());
  return engine;
}

int randomInt(int max){
  if(max < 0)
    return 0;
  std::uniform_int_distribution<int> distribution(0, max);
  return distribution(randomEngine());
}

void shuffle(int * values, int count){
  if(values == NULL || count <= 1)
    return;
  for(int i = count - 1; i > 0; i--){
    int j = randomInt(i);
    std::swap(values[i], values[j]);
  }
}
