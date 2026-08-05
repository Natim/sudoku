#include "score.h"

#include <cstdio>

namespace {

// Points a grid is worth when the player takes exactly the time it is expected
// to take. The unit is arbitrary; a factor of twelve over the complexity keeps
// scores in the hundreds to the thousands, which reads better than a bare
// complexity of 47.
const int POINTS_PER_COMPLEXITY = 12;

// Time the grid is expected to take, per complexity point: about five minutes
// for an easy grid, twenty for a hard one.
const int SECONDS_PER_COMPLEXITY = 8;

// An instant solve is worth twice the base score, the expected time exactly the
// base, and the score then keeps decaying without ever reaching this fraction
// of it: finishing a grid always counts for something.
const int FLOOR_DIVISOR = 5;

const char * levelName(int complexity){
  if(complexity < 45)
    return "Facile";
  if(complexity < 70)
    return "Moyen";
  if(complexity < 110)
    return "Difficile";
  return "Diabolique";
}

int starsFor(int seconds, int expected){
  if(2 * seconds <= expected)
    return 3;
  if(seconds <= expected)
    return 2;
  return 1;
}

}

Score rateGame(int complexity, int seconds){
  Score score;
  score.complexity = complexity;
  score.seconds = (seconds > 0) ? seconds : 0;
  score.expected = complexity * SECONDS_PER_COMPLEXITY;
  score.points = 0;
  score.stars = 0;
  score.level = "Aucune";

  if(complexity <= 0)
    return score;

  score.level = levelName(complexity);
  score.stars = starsFor(score.seconds, score.expected);

  const int base = complexity * POINTS_PER_COMPLEXITY;
  int points = 2 * base * score.expected / (score.expected + score.seconds);
  if(points < base / FLOOR_DIVISOR)
    points = base / FLOOR_DIVISOR;

  // Round numbers look like a score rather than a measurement.
  score.points = (points + 5) / 10 * 10;
  return score;
}

std::string formatDuration(int seconds){
  if(seconds < 0)
    seconds = 0;

  char text[32];
  if(seconds < 3600)
    snprintf(text, sizeof text, "%d:%02d", seconds / 60, seconds % 60);
  else
    snprintf(text, sizeof text, "%d:%02d:%02d",
	     seconds / 3600, (seconds / 60) % 60, seconds % 60);
  return std::string(text);
}
