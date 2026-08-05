#pragma once

#include <random>

// Entier tire uniformement dans 0..nb inclus.
int hasard(int nb);

// Melange les n premiers elements du tableau (Fisher-Yates).
void melanger(int * tab, int n);

// Pour les algorithmes de <algorithm> qui reclament un generateur.
std::mt19937 & moteurAleatoire();
