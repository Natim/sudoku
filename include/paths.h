#pragma once

#include <string>

// Directory holding the images, wherever the program was installed, unpacked or
// built. SUDOKU_ASSETS_DIR in the environment overrides the search.
std::string assetsDir();

// File the grid is saved to and read back from.
std::string savedGridPath();

// Create the directory holding the saved grid. False when it cannot be made.
bool createSavedGridDirectory();
