#include "generator.h"
#include "random.h"

#include <algorithm>
#include <vector>

static const int CELL_COUNT = SIZE * SIZE;
static const unsigned ALL_DIGITS = 0x1FF;

struct State {
  int val[CELL_COUNT];
  unsigned freeInRow[SIZE];
  unsigned freeInCol[SIZE];
  unsigned freeInBlock[SIZE];
};

static int index(int row, int col){
  return row * SIZE + col;
}

static int block(int row, int col){
  return (row / 3) * 3 + col / 3;
}

static void clearState(State & state){
  for(int i = 0; i < CELL_COUNT; i++)
    state.val[i] = 0;
  for(int i = 0; i < SIZE; i++){
    state.freeInRow[i] = ALL_DIGITS;
    state.freeInCol[i] = ALL_DIGITS;
    state.freeInBlock[i] = ALL_DIGITS;
  }
}

static void stateFromSudoku(const Sudoku & s, State & state){
  clearState(state);
  for(int i = 0; i < SIZE; i++){
    for(int j = 0; j < SIZE; j++){
      int value = s.cells[i][j];
      if(value == 0)
	continue;
      int p = index(i, j);
      int b = block(i, j);
      state.val[p] = value;
      unsigned mask = ~(1u << (value - 1));
      state.freeInRow[i] &= mask;
      state.freeInCol[j] &= mask;
      state.freeInBlock[b] &= mask;
    }
  }
}

static unsigned candidates(const State & state, int p){
  int row = p / SIZE;
  int col = p % SIZE;
  return state.freeInRow[row] & state.freeInCol[col] & state.freeInBlock[block(row, col)];
}

static void setCell(State & state, int p, int value){
  int row = p / SIZE;
  int col = p % SIZE;
  int b = block(row, col);
  state.val[p] = value;
  unsigned mask = ~(1u << (value - 1));
  state.freeInRow[row] &= mask;
  state.freeInCol[col] &= mask;
  state.freeInBlock[b] &= mask;
}

static void clearCell(State & state, int p){
  int value = state.val[p];
  if(value == 0)
    return;
  int row = p / SIZE;
  int col = p % SIZE;
  int b = block(row, col);
  state.val[p] = 0;
  unsigned bit = 1u << (value - 1);
  state.freeInRow[row] |= bit;
  state.freeInCol[col] |= bit;
  state.freeInBlock[b] |= bit;
}

static int mrvCell(const State & state){
  int best = -1;
  int minCand = 10;
  for(int p = 0; p < CELL_COUNT; p++){
    if(state.val[p] != 0)
      continue;
    unsigned cand = candidates(state, p);
    if(cand == 0)
      return p;
    int nbCand = 0;
    for(unsigned bit = cand; bit != 0; bit &= bit - 1)
      nbCand++;
    if(nbCand < minCand){
      minCand = nbCand;
      best = p;
    }
  }
  return best;
}

static void listCandidates(unsigned cand, int * digits, int & count){
  count = 0;
  for(int d = 1; d <= 9; d++){
    if(cand & (1u << (d - 1)))
      digits[count++] = d;
  }
}

static int countRec(State & state, int limit){
  int p = mrvCell(state);
  if(p < 0)
    return 1;
  if(candidates(state, p) == 0)
    return 0;

  int total = 0;
  unsigned cand = candidates(state, p);
  for(int d = 1; d <= 9; d++){
    if((cand & (1u << (d - 1))) == 0)
      continue;
    setCell(state, p, d);
    total += countRec(state, limit - total);
    clearCell(state, p);
    if(total >= limit)
      return total;
  }
  return total;
}

static bool fillRec(State & state){
  int p = mrvCell(state);
  if(p < 0)
    return true;
  unsigned cand = candidates(state, p);
  if(cand == 0)
    return false;

  int digits[9];
  int count;
  listCandidates(cand, digits, count);
  shuffle(digits, count);

  for(int i = 0; i < count; i++){
    setCell(state, p, digits[i]);
    if(fillRec(state))
      return true;
    clearCell(state, p);
  }
  return false;
}

static bool fillSolution(State & state){
  clearState(state);
  return fillRec(state);
}

static int countState(State state, int limit){
  return countRec(state, limit);
}

static int countClues(const State & state){
  int count = 0;
  for(int p = 0; p < CELL_COUNT; p++)
    if(state.val[p] != 0)
      count++;
  return count;
}

static int targetClues(Difficulty level){
  switch(level){
  case EASY:    return 45;
  case MEDIUM:  return 34;
  case HARD:    return 28;
  }
  return 34;
}

static State carve(State state, int target){
  int pairs[41];
  for(int i = 0; i < 41; i++)
    pairs[i] = i;
  shuffle(pairs, 41);

  for(int k = 0; k < 41; k++){
    if(countClues(state) <= target)
      break;

    int p = pairs[k];
    int q = 80 - p;
    int savedP = state.val[p];
    int savedQ = state.val[q];

    clearCell(state, p);
    if(p != q)
      clearCell(state, q);

    if(countState(state, 2) == 1)
      continue;

    if(savedP != 0)
      setCell(state, p, savedP);
    if(p != q && savedQ != 0)
      setCell(state, q, savedQ);
  }
  return state;
}

static Sudoku sudokuFromState(const State & state){
  Sudoku s = emptyGrid();
  for(int i = 0; i < SIZE; i++){
    for(int j = 0; j < SIZE; j++){
      int value = state.val[index(i, j)];
      if(value == 0)
	continue;
      s.cells[i][j] = value;
      s.fixed[i][j] = true;
      s.given[i][j] = true;
    }
  }
  return s;
}

int countSolutions(Sudoku s, int limit){
  if(limit <= 0)
    return 0;
  State state;
  stateFromSudoku(s, state);
  return countState(state, limit);
}

Sudoku generateGrid(Difficulty level){
  int target = targetClues(level);
  Sudoku best = emptyGrid();
  int bestClues = CELL_COUNT + 1;

  for(int attempt = 0; attempt < 3; attempt++){
    State solution;
    if(!fillSolution(solution))
      continue;

    State carved = carve(solution, target);
    int count = countClues(carved);
    if(count < bestClues){
      bestClues = count;
      best = sudokuFromState(carved);
      if(count <= target)
	break;
    }
  }
  return best;
}
