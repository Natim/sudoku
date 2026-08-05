#include "celebration.h"
#include "dialog_box.h"
#include "sudoku_grid.h"
#include "window.h"

extern "C"{
#include "graphlib.h"
}

#include <string>
#include <unistd.h>
#include <vector>

using namespace std;

namespace {

// Entries of the palette graphlib installs for initPalette(NULL, 0). The dialog
// boxes already dress in dark red and gold; the panel follows suit.
const int BLACK        = 0;
const int WHITE        = 1;
const int YELLOW       = 5;
const int PALE_YELLOW  = 190;
const int FAINT_YELLOW = 196;
const int BACKGROUND   = 20;
const int TITLE_BAR    = 238;
const int DIM_STAR     = 12;
const int SHADOW       = 242;

/* A band of lit cells travelling across the grid, one anti-diagonal per frame.
   Cells are lit by a halo drawn in the gap around them rather than by redrawing
   the digit: a bitmap costs one rectangle per run of identical pixels, far too
   many to animate. */
const int HALO_COLORS[] = { YELLOW, PALE_YELLOW, FAINT_YELLOW };
const int HALO_BAND     = sizeof HALO_COLORS / sizeof *HALO_COLORS;
const int HALO_MARGIN   = 2;
const int WAVE_DELAY_US = 40000;
const int WAVE_PASSES   = 2;

const int TITLE_HEIGHT = 36;
const int STAR_RADIUS  = 15;
const int STAR_GAP     = 44;
const int LINE_HEIGHT  = 24;

// The panel is drawn over a shadow shifted down and to the right, which is why
// the box itself is smaller than the area the caller has to repaint after.
const int SHADOW_SHIFT = 6;
const int BOX_WIDTH  = PANEL_WIDTH - SHADOW_SHIFT;
const int BOX_HEIGHT = PANEL_HEIGHT - SHADOW_SHIFT;

const double TITLE_TEXT_SCALE = 2.0;
const double SCORE_TEXT_SCALE = 1.7;

// Where celebrate() was asked to draw the panel.
int panelX = 0;
int panelY = 0;

struct ButtonArea {
  int x1, y1, x2, y2;
};

void centeredText(int y, const string & text){
  drawText(panelX + (BOX_WIDTH - textWidth(text.c_str())) / 2, y, text.c_str());
}

// Six-pointed star: two triangles, one pointing up and one pointing down.
void star(int cx, int cy, bool earned){
  // Half the side of the triangle, then how far its base sits from the centre.
  const int half = STAR_RADIUS * 87 / 100;
  const int base = STAR_RADIUS / 2;

  modifierCouleur(earned ? YELLOW : DIM_STAR);
  fillTriangle(cx, cy - STAR_RADIUS, cx - half, cy + base, cx + half, cy + base);
  fillTriangle(cx, cy + STAR_RADIUS, cx - half, cy - base, cx + half, cy - base);
}

vector<ButtonArea> layoutButtons(const vector<string> & labels){
  const int gap = 12;
  const int y1 = panelY + BOX_HEIGHT - 32;
  const int y2 = panelY + BOX_HEIGHT - 10;

  vector<int> widths;
  int total = gap * ((int) labels.size() - 1);
  for(unsigned int i = 0; i < labels.size(); i++){
    int width = textWidth(labels[i].c_str()) + 20;
    widths.push_back(width);
    total += width;
  }

  vector<ButtonArea> areas;
  int current = panelX + (BOX_WIDTH - total) / 2;
  for(unsigned int i = 0; i < labels.size(); i++){
    ButtonArea area = { current, y1, current + widths[i], y2 };
    areas.push_back(area);
    current += widths[i] + gap;
  }
  return areas;
}

void drawPanel(const Score & score, const vector<string> & labels){
  initPalette(NULL, 0);

  modifierCouleur(SHADOW);
  fillRect(panelX + SHADOW_SHIFT, panelY + SHADOW_SHIFT,
	   panelX + PANEL_WIDTH, panelY + PANEL_HEIGHT);

  modifierCouleur(BACKGROUND);
  fillRect(panelX, panelY, panelX + BOX_WIDTH, panelY + BOX_HEIGHT);

  modifierCouleur(TITLE_BAR);
  fillRect(panelX, panelY, panelX + BOX_WIDTH, panelY + TITLE_HEIGHT);

  setTextScale(TITLE_TEXT_SCALE);
  modifierCouleur(BLACK);
  centeredText(panelY + TITLE_HEIGHT - 10, "BRAVO !");
  setTextScale(1.0);

  const int stars = panelY + TITLE_HEIGHT + 36;
  for(int i = 0; i < 3; i++)
    star(panelX + BOX_WIDTH / 2 + (i - 1) * STAR_GAP, stars, i < score.stars);

  modifierCouleur(WHITE);
  int line = stars + STAR_RADIUS + 30;
  centeredText(line, "Difficulté : " + score.level);
  line += LINE_HEIGHT;

  if(score.points > 0){
    centeredText(line, "Temps : " + formatDuration(score.seconds) +
		 "   (prévu : " + formatDuration(score.expected) + ")");
    setTextScale(SCORE_TEXT_SCALE);
    modifierCouleur(YELLOW);
    centeredText(line + 44, to_string(score.points) + " points");
    setTextScale(1.0);
  }else{
    centeredText(line, "Temps : " + formatDuration(score.seconds));
    centeredText(line + 34, "Plusieurs solutions étaient possibles :");
    centeredText(line + 54, "rien à marquer, mais joli travail.");
  }

  vector<ButtonArea> areas = layoutButtons(labels);
  for(unsigned int i = 0; i < labels.size(); i++){
    modifierCouleur(WHITE);
    drawRect(areas[i].x1, areas[i].y1, areas[i].x2, areas[i].y2);
    const int width = textWidth(labels[i].c_str());
    drawText(areas[i].x1 + (areas[i].x2 - areas[i].x1 - width) / 2,
	     areas[i].y2 - 6, labels[i].c_str());
  }

  modifierCouleur(BLACK);
  drawRect(panelX, panelY, panelX + BOX_WIDTH, panelY + BOX_HEIGHT);
  drawRect(panelX, panelY + TITLE_HEIGHT, panelX + BOX_WIDTH, panelY + BOX_HEIGHT);
}

// Ring drawn in the white gap around a cell, the two rectangles making it
// thick enough to be seen once the window is scaled down.
void halo(CellLocator locate, int cellSize, int row, int col, int color){
  int x, y;
  locate(row, col, &x, &y);

  modifierCouleur(color);
  for(int margin = HALO_MARGIN; margin <= HALO_MARGIN + 1; margin++)
    drawRect(x - margin, y - margin, x + cellSize + margin, y + cellSize + margin);
}

void wave(CellLocator locate, int cellSize, bool forward){
  const int last = 2 * (SIZE - 1);

  for(int front = 0; front <= last + HALO_BAND; front++){
    for(int row = 0; row < SIZE; row++){
      for(int col = 0; col < SIZE; col++){
	int age = front - (forward ? row + col : last - row - col);
	if(age < 0)
	  continue;
	if(age < HALO_BAND)
	  halo(locate, cellSize, row, col, HALO_COLORS[age]);
	else if(age == HALO_BAND)
	  halo(locate, cellSize, row, col, WHITE);
      }
    }
    usleep(WAVE_DELAY_US);
  }
}

}

bool celebrate(int x, int y, CellLocator locate, int cellSize, const Score & score){
  panelX = x;
  panelY = y;

  // The last bitmap drawn left its own palette installed.
  initPalette(NULL, 0);
  for(int pass = 0; pass < WAVE_PASSES; pass++)
    wave(locate, cellSize, pass % 2 == 0);

  vector<string> labels;
  labels.push_back("Nouvelle grille");
  labels.push_back("Fermer");

  vector<ButtonArea> areas = layoutButtons(labels);
  drawPanel(score, labels);

  for(;;){
    int clickX, clickY;
    if(!waitForLogicalClick(&clickX, &clickY)){
      paintBackground();
      areas = layoutButtons(labels);
      drawPanel(score, labels);
      continue;
    }

    for(unsigned int i = 0; i < areas.size(); i++)
      if(clickX > areas[i].x1 && clickX < areas[i].x2 &&
	 clickY > areas[i].y1 && clickY < areas[i].y2)
	return i == 0;
  }
}
