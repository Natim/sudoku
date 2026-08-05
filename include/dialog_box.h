#pragma once

#include <string>
#include <vector>

bool alert(int x, int y, std::string message);
bool alert(int x, int y, std::string message, std::string titre);
bool alert(int x, int y, int width, int height, std::string message, std::string titre);

int choisir(int x, int y, int width, int height,
            std::string titre, std::string message,
            const std::vector<std::string> & choix);

void definirRepeindreFond(void (*peindre)());
