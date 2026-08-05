#pragma once

#include <string>
#include <vector>

bool alert(int x, int y, std::string message);
bool alert(int x, int y, std::string title, std::string message);
bool alert(int x, int y, int width, int height, std::string title, std::string message);

int choose(int x, int y, int width, int height,
           std::string title, std::string message,
           const std::vector<std::string> & choices);

void setBackgroundRepaint(void (*paint)());

// Repaint the window behind a modal box, after a resize under it.
void paintBackground();
