==============
Sudoku Solver
==============

Another project that went a long time without a complete generator. Here are
the main outlines anyway.

The interface was heavily worked on, with a class (``src/bitmap.cpp``) to read
8-bit bitmap files. A graphlib bug prints a message every time the color
palette changes, that is on every image display. Use ``launch.sh`` to suppress
it.

The interface also provides three functions to display and manage dialog boxes.

A set of functions manages sudokus, solves them, and generates new ones. You
can save a sudoku to a file and reopen the grid later, solve it, and place
digits while checking they belong in the right cell.

Finally, the program itself handles the interface between the user and the
sudoku.

This sudoku was a very interesting project on many levels (bitmap reading,
systematic solution search, dialog boxes, etc.).

1/ Reading a bitmap
=========================

The idea is not mine but Germain Desvigne's, who proposed a challenge: display
images in Graphlib. We each implemented a class for the Sudoku to load
bitmaps. Mine handles 8-bit files only; his handles all bitmaps of at most 8
bits.

A bitmap can be split into four parts:

- **The file header**: starts with ``BM`` and holds the offset to the pixel
  data.
- **The bitmap header**: holds the color count, width, and height.
- **The BMP palette**: all colors in BGR order (not RGB).
- **The pixels**: one byte per pixel pointing at a palette entry.

2/ The Sudoku
============

Sudoku was very popular, so many solutions existed on the internet, with more
or less fanciful methods. Most tried to make a computer think like a human:

- candidate search, elimination of impossible values, etc.

I chose a much simpler method that relies on raw machine power, walking the
cells until they are all filled, or stepping out at (0,0) which means there is
no solution.

This solver can solve every solvable grid.

2.1/ The method
---------------

Start at cell (0,0) and advance through non-fixed cells until a digit can be
placed. If a cell reaches 9, set it to 0 and step back one cell. Repeat until
the sudoku is solved.

Stepping out at (0,0) means the sudoku has no solution.

2.2/ The generator
------------------

The "New" button offers playable grids at three levels, plus a blank grid.
Each generated grid follows the rules and has exactly one solution.

The method starts from a complete random grid, then carves cells two at a
time in central symmetry. A hole is kept only if the incomplete grid still has
exactly one solution; otherwise the digits are put back. The levels target
about 45 clues (easy), 34 (medium), and 28 (hard).

Solution counting and random filling use a dedicated bit-mask solver, faster
than ``resolve()`` for chaining checks. Random draws are centralized in
``random.h`` (``randomInt``, ``shuffle``, ``randomEngine``).

Generated clues are marked ``given``: they display as fixed clues and cannot
be erased by the player.

3/ Dialog boxes
==========================

They add a lot to the program. ``alert()`` returns ``true`` when "Oui" is
clicked and ``false`` otherwise. ``choose()`` shows several buttons and
returns the clicked index. This keeps the user interface simple, including
the choice of difficulty for a new grid.

In this sudoku you can generate a grid, open a saved one, save a grid, place
digits in cells, solve the grid, and quit.

----

If you get the chance, I can only suggest looking at the Sudokus by Germain
Desvignes and Rémy Burney, who both did excellent work on this project.
