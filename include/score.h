#pragma once

#include <string>

// What a finished grid is worth. A grid that had no unique solution was never
// a puzzle: it is worth no points and no stars.
struct Score {
  int complexity;      // as measured by gridComplexity()
  int seconds;         // time spent on the grid
  int expected;        // time the grid is expected to take
  int points;
  int stars;           // 0 to 3, speed against the expected time
  std::string level;   // complexity in words
};

Score rateGame(int complexity, int seconds);

// mm:ss, or h:mm:ss past the hour.
std::string formatDuration(int seconds);
