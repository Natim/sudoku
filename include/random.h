#pragma once

#include <random>

// Uniform integer in the inclusive range 0..max.
int randomInt(int max);

// Shuffle the first count elements of values (Fisher-Yates).
void shuffle(int * values, int count);

// For <algorithm> routines that need a generator.
std::mt19937 & randomEngine();
