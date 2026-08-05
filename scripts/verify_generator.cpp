// Temporary generator checks: rules, uniqueness, randomness.
#include "generator.h"
#include "random.h"
#include "sudoku_grid.h"

#include <chrono>
#include <cstdio>
#include <set>

static bool followsRules(const Sudoku & s){
  for(int i = 0; i < SIZE; i++){
    for(int j = 0; j < SIZE; j++){
      int value = s.cells[i][j];
      if(value == 0)
	continue;
      Sudoku copy = s;
      if(!checkCell(copy, i, j))
	return false;
    }
  }
  return true;
}

static int countClues(const Sudoku & s){
  int count = 0;
  for(int i = 0; i < SIZE; i++)
    for(int j = 0; j < SIZE; j++)
      if(s.cells[i][j] != 0)
	count++;
  return count;
}

static void checkLevel(Difficulty level, int target, int attempts){
  printf("Level %d (%d attempts)\n", (int) level, attempts);
  for(int i = 0; i < attempts; i++){
    auto start = std::chrono::steady_clock::now();
    Sudoku s = generateGrid(level);
    auto end = std::chrono::steady_clock::now();
    double ms = std::chrono::duration<double, std::milli>(end - start).count();

    int clues = countClues(s);
    int solutions = countSolutions(s, 2);
    bool rules = followsRules(s);

    printf("  attempt %d: %d clues, %d solution(s), rules=%s, %.1f ms\n",
	   i + 1, clues, solutions, rules ? "ok" : "fail", ms);

    if(!rules || solutions != 1){
      displaySudoku(s);
      return;
    }
    if(clues > target + 5)
      printf("  (warning: above target %d)\n", target);
  }
}

static void checkRandom(){
  std::set<int> seen;
  for(int i = 0; i < 500; i++){
    int v = randomInt(8);
    if(v < 0 || v > 8){
      printf("randomInt(8) out of range: %d\n", v);
      return;
    }
    seen.insert(v);
  }
  printf("randomInt(8) covers %zu values out of 9\n", seen.size());
}

static void checkDiversity(){
  Sudoku a = generateGrid(MEDIUM);
  Sudoku b = generateGrid(MEDIUM);
  bool identical = true;
  for(int i = 0; i < SIZE && identical; i++)
    for(int j = 0; j < SIZE && identical; j++)
      if(a.cells[i][j] != b.cells[i][j])
	identical = false;
  printf("two medium grids identical: %s\n", identical ? "yes" : "no");
}

int main(){
  checkRandom();
  checkDiversity();
  checkLevel(EASY, 45, 3);
  checkLevel(MEDIUM, 34, 3);
  checkLevel(HARD, 28, 3);
  return 0;
}
