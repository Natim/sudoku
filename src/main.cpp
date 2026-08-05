extern "C"{
#include "graphlib.h"
}
#include "bitmap.h"
#include "sudoku_grid.h"
#include "generator.h"
#include "dialog_box.h"
#include "window.h"

#include <string>
#include <vector>
#include <cstdlib>

#ifndef SUDOKU_ASSETS_DIR
#define SUDOKU_ASSETS_DIR "assets"
#endif

static std::string asset(const std::string & name){
  return std::string(SUDOKU_ASSETS_DIR) + "/images/" + name;
}

typedef struct{
  Bitmap * up;
  Bitmap * down;
  int x;
  int digit;
} Button;

const int GRID_X = 50;
const int GRID_Y = 74;
const int WINDOW_SIZE = 600;
const int DIAL_W = 300;
const int DIAL_H = 100;
const int DIAL_X = (WINDOW_SIZE - DIAL_W) / 2;
const int DIAL_Y = (WINDOW_SIZE - DIAL_H) / 2;
const int GEN_W = 460;
const int GEN_H = 110;
const int GEN_X = (WINDOW_SIZE - GEN_W) / 2;
const int GEN_Y = (WINDOW_SIZE - GEN_H) / 2;

Button newGame;
Button openBtn;
Button saveBtn;
Button helpBtn;
Button quitBtn;
Button errorBmp;
Button * digits;
Button * pressed = NULL;
int screen[9][9];
Sudoku * displayedSudoku = NULL;

void loadBitmaps();
void freeBitmaps();
void menu();
void cellPosition(int row, int col, int * x, int * y);
void updateScreen(Sudoku s);
void paintAll(Sudoku s);
void repaintAll();
void restoreArea(int x, int y, int width, int height);
void afterDialog(Sudoku s);

int main(){
  int x, y;
  int row, col;
  bool quit = false;

  loadBitmaps();
  Sudoku sudoku = emptyGrid();
  if(const char * path = std::getenv("SUDOKU_AUTO_LOAD"))
    sudoku = loadGrid(path);
  displayedSudoku = &sudoku;
  setBackgroundRepaint(repaintAll);

  openScalableWindow(WINDOW_SIZE, WINDOW_SIZE, "Sudoku - Rémy HUBSCHER");
  paintAll(sudoku);

  while(!quit){
      if(!waitForLogicalClick(&x, &y)){
	paintAll(sudoku);
	continue;
      }

      if(y >= 7 && y <= 39){
	if(x >= newGame.x && x <= newGame.x+32){
	  pressed = &newGame;
	  std::vector<std::string> choices;
	  choices.push_back("Facile");
	  choices.push_back("Moyen");
	  choices.push_back("Difficile");
	  choices.push_back("Vierge");
	  choices.push_back("Annuler");
	  int selection = choose(GEN_X, GEN_Y, GEN_W, GEN_H,
				  "Nouvelle grille",
				  "Quelle difficulte ?",
				  choices);
	  if(selection == 0)
	    sudoku = generateGrid(EASY);
	  else if(selection == 1)
	    sudoku = generateGrid(MEDIUM);
	  else if(selection == 2)
	    sudoku = generateGrid(HARD);
	  else if(selection == 3)
	    sudoku = emptyGrid();
	  pressed = NULL;
	  if(selection >= 0 && selection <= 3)
	    paintAll(sudoku);
	  else
	    afterDialog(sudoku);
	}
	else if(x >= openBtn.x && x <= openBtn.x+32){
	  pressed = &openBtn;
	  if(alert(DIAL_X, DIAL_Y, "Voulez vous vraiment charger une grille ?"))
	    sudoku = loadGrid("grille.sdk");
	  pressed = NULL;
	  afterDialog(sudoku);
	}
	else if(x >= saveBtn.x && x <= saveBtn.x+32){
	  pressed = &saveBtn;
	  if(alert(DIAL_X, DIAL_Y, "Voulez vous vraiment enregistrer la grille ? La grille précédente sera effacée")){
	    if(!saveGrid(sudoku, "grille.sdk")){
	      alert(DIAL_X, DIAL_Y, "Impossible d'enregistrer le sudoku. Etes vous sur des droits du fichier grille.sdk ?");
	      alert(DIAL_X, DIAL_Y, "Il serait bon de vérifier");
	    }
	  }
	  pressed = NULL;
	  afterDialog(sudoku);
	}
	else if(x >= helpBtn.x && x <= helpBtn.x+32){
	  pressed = &helpBtn;
	  if(alert(DIAL_X, DIAL_Y, "Voulez vous vraiment avoir la solution ?")){
	    if(!resolve(&sudoku)){
	      if(alert(DIAL_X, DIAL_Y, "Aucune solution. Initialiser la grille ?"))
		sudoku = emptyGrid();
	    }
	  }
	  pressed = NULL;
	  afterDialog(sudoku);
	}
	else if(x >= quitBtn.x && x <= quitBtn.x+32){
	  pressed = &quitBtn;
	  quit = true;
	}else if(((x - 140) / 36) >= 0 && ((x - 140) / 36) < 10){
	  pressed = &digits[((x - 140) / 36)];
	}else{
	  pressed = NULL;
	}
	menu();
      }else{
	col = (((x - GRID_X) / 182) * 3 + ((x - GRID_X) % 182) / 52);
	if(col > 8) col = 8;
	if(col < 0) col = 0;

	row = (((y - GRID_Y) / 182) * 3 + ((y - GRID_Y) % 182) / 52);
	if(row > 8) row = 8;
	if(row < 0) row = 0;

	if(pressed != NULL && pressed->digit != 10){
	  if(place(&sudoku, row, col, pressed->digit))
	    updateScreen(sudoku);
	}
      }

      if(pressed != NULL)
	pressed->down->draw(pressed->x, 7);
    }

  freeBitmaps();

  fermerFenetre();
}

void loadBitmaps(){
  int i;
  char * path = new char[255];
  newGame.up   = new Bitmap(asset("new.bmp").c_str());
  newGame.down = new Bitmap(asset("newd.bmp").c_str());
  newGame.x    = 10;
  newGame.digit = 10;

  openBtn.up    = new Bitmap(asset("ouvrir.bmp").c_str());
  openBtn.down  = new Bitmap(asset("ouvrird.bmp").c_str());
  openBtn.x     = 50;
  openBtn.digit = 10;

  saveBtn.up       = new Bitmap(asset("enregistrer.bmp").c_str());
  saveBtn.down     = new Bitmap(asset("enregistrerd.bmp").c_str());
  saveBtn.x        = 90;
  saveBtn.digit = 10;

  helpBtn.up      = new Bitmap(asset("aide.bmp").c_str());
  helpBtn.down    = new Bitmap(asset("aided.bmp").c_str());
  helpBtn.x       = 510;
  helpBtn.digit = 10;

  quitBtn.up    = new Bitmap(asset("fermer.bmp").c_str());
  quitBtn.down  = new Bitmap(asset("fermerd.bmp").c_str());
  quitBtn.x     = 550;
  quitBtn.digit = 10;

  errorBmp.up   = new Bitmap(asset("erreur.bmp").c_str());

  digits = new Button[10];

  for(i=0; i < 10; i++){
    snprintf(path, 255, "%s", asset(std::to_string(i) + ".bmp").c_str());
    digits[i].up = new Bitmap(path);

    snprintf(path, 255, "%s", asset(std::to_string(i) + "d.bmp").c_str());
    digits[i].down = new Bitmap(path);

    digits[i].x = 140 + i*36;
    digits[i].digit = i;
  }

  delete[] path;
}

void freeBitmaps(){
  int i;
  delete newGame.up;
  delete openBtn.up;
  delete saveBtn.up;
  delete helpBtn.up;
  delete quitBtn.up;
  delete errorBmp.up;

  delete newGame.down;
  delete openBtn.down;
  delete saveBtn.down;
  delete helpBtn.down;
  delete quitBtn.down;

  for(i=0; i < 10; i++){
    delete digits[i].up;
  }

  for(i=0; i < 10; i++){
    delete digits[i].down;
  }

  delete[] digits;
}

void menu(){
  int i;
  drawLine(0, 46, WINDOW_SIZE, 46);

  newGame.up->draw(newGame.x, 7);
  openBtn.up->draw(openBtn.x, 7);
  saveBtn.up->draw(saveBtn.x, 7);
  helpBtn.up->draw(helpBtn.x, 7);
  quitBtn.up->draw(quitBtn.x, 7);

  for(i=0; i < 10; i++){
    digits[i].up->draw(140 + i*36, 7);
  }
}

void cellPosition(int row, int col, int * x, int * y){
  *x = GRID_X + (col / 3) * 182 + (col % 3) * 52;
  *y = GRID_Y + (row / 3) * 182 + (row % 3) * 52;
}

void updateScreen(Sudoku s){
  for(int i = 0; i < 9; i++){
    for(int j = 0; j < 9; j++){
      int style = 0;
      if(s.cells[i][j] != 0){
	if(s.given[i][j])
	  style = 2;
	else
	  style = 1;
      }
      int code = s.cells[i][j] * 3 + style;
      if(code == screen[i][j])
	continue;
      int x, y;
      cellPosition(i, j, &x, &y);
      if(s.cells[i][j] == 0)
	digits[0].up->draw(x, y);
      else if(s.given[i][j])
	digits[s.cells[i][j]].down->draw(x, y);
      else
	digits[s.cells[i][j]].up->draw(x, y);
      screen[i][j] = code;
    }
  }
}

void paintAll(Sudoku s){
  viderFenetre();
  Bitmap::invalidateAll();
  for(int i = 0; i < 9; i++)
    for(int j = 0; j < 9; j++)
      screen[i][j] = -1;
  menu();
  updateScreen(s);
  if(pressed != NULL)
    pressed->down->draw(pressed->x, 7);
}

void repaintAll(){
  if(displayedSudoku != NULL)
    paintAll(*displayedSudoku);
}

const int CELL_SIZE = 32;

/* Clear the dialog area and any grid cells it covers, then mark those cells
   for redraw. */
void restoreArea(int x, int y, int width, int height){
  int minX = x, minY = y, maxX = x + width, maxY = y + height;

  for(int i = 0; i < 9; i++){
    for(int j = 0; j < 9; j++){
      int cx, cy;
      cellPosition(i, j, &cx, &cy);
      if(cx < x + width && cx + CELL_SIZE > x && cy < y + height && cy + CELL_SIZE > y){
	if(cx < minX) minX = cx;
	if(cy < minY) minY = cy;
	if(cx + CELL_SIZE > maxX) maxX = cx + CELL_SIZE;
	if(cy + CELL_SIZE > maxY) maxY = cy + CELL_SIZE;
	screen[i][j] = -1;
      }
    }
  }

  digits[0].up->fillBackground(minX, minY, maxX, maxY);
  Bitmap::invalidateArea(minX, minY, maxX + 1, maxY + 1);
}

void afterDialog(Sudoku s){
  restoreArea(DIAL_X, DIAL_Y, DIAL_W, DIAL_H);
  restoreArea(GEN_X, GEN_Y, GEN_W, GEN_H);
  updateScreen(s);
}
