==============
Sudoku Solver
==============


.. raw:: html

   <div align="center">
     <img src="docs/screenshot.png" alt="Sudoku solver interface" width="400" />
   </div>

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

4/ Finishing a grid
===================

Every digit is checked as it is placed, so a full grid is a solved one: the
last digit ends the game. A wave of light sweeps the grid, then a panel gives
the difficulty, the time and the score. Asking the program for the solution
gives the grid up, and the panel does not appear.

The difficulty is measured rather than declared, so a grid opened from a file
is graded like a generated one. ``gridComplexity()`` counts the empty cells and
the digits a constraint solver has to try where elimination alone does not
decide. A grid with several solutions is not a puzzle and is worth nothing.

The score turns that complexity into points: twice as many for an instant
solve, exactly the base for a grid solved in the time it was expected to take,
and less and less after that without ever reaching zero. The three stars
compare the time taken with the time expected.

5/ Installing
=============

Every release carries two files, both for Linux on x86_64 since the graphlib
that comes with the project is a prebuilt library. The Debian package puts the
game, its images, its icon and its menu entry under ``/usr``::

  sudo apt install ./sudoku_1.0.0_amd64.deb

The archive holds the same tree and is unpacked wherever you like: the program
looks for its images from its own location, so it needs no install step and no
particular working directory::

  tar xzf sudoku-1.0.0-linux-x86_64.tar.gz
  ./sudoku-1.0.0-linux-x86_64/bin/sudoku

``SUDOKU_ASSETS_DIR`` in the environment overrides that search, should you keep
the images elsewhere.

``make package`` builds both files in ``build-package/``. That is what the
release workflow does before checking that each of them starts and reads its
images on a headless display; it is started by hand from the Actions tab, with
the version to publish.

An installed game saves grids in ``~/.local/share/sudoku/grille.sdk``. A
``grille.sdk`` in the current directory is used instead when there is one, which
is the case in the source tree.

6/ Licence
==========

The code of this repository is under the MIT licence, in ``LICENSE``.

``third_party/graphlib`` is not mine and carries no licence text: it is the
teaching library the project was written against, kept here as a prebuilt
static library so that the game still compiles. The MIT licence covers the
sources around it, not that library, and it is the one thing to settle before
handing out binaries widely, since a release embeds it in the executable.
