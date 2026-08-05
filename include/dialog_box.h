#pragma once

#include <string>

bool alert(int x, int y, std::string message);
bool alert(int x, int y, std::string message, std::string titre);
bool alert(int x, int y, int width, int height, std::string message, std::string titre);

void definirRepeindreFond(void (*peindre)());
