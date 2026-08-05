extern "C"{
#include "graphlib.h"
}
#include "bitmap.h"
#include "sudoku_grid.h"
#include "dialog_box.h"
#include "window.h"

#include <string>
#include <cstdlib>
#include <ctime>

#ifndef SUDOKU_ASSETS_DIR
#define SUDOKU_ASSETS_DIR "assets"
#endif

static std::string asset(const std::string & nom){
  return std::string(SUDOKU_ASSETS_DIR) + "/images/" + nom;
}

// Définition des types //
typedef struct{
  Bitmap * up;
  Bitmap * down;
  int x;
  int nb;
} Bouton;

// Constantes //
const int DECALX_GRILLE = 50;
const int DECALY_GRILLE = 74;
const int FENETRE = 600;
const int DIAL_W = 300;
const int DIAL_H = 100;
const int DIAL_X = (FENETRE - DIAL_W) / 2;
const int DIAL_Y = (FENETRE - DIAL_H) / 2;

// Variable Globale //
Bouton nouv;
Bouton ouvrir;
Bouton rec;
Bouton aide;
Bouton fermer;
Bouton erreur;
Bouton * b;
Bouton * bouton = NULL;   // Bouton sur lequel on appuie
int ecran[9][9];
Sudoku * sudokuAffiche = NULL;  // Grille actuellement a l'ecran

// Prototypes //
void alloueBmp();
void desalloueBmp();
void menu();
void positionCase(int lig, int col, int * x, int * y);
void majEcran(Sudoku s);
void peindreTout(Sudoku s);
void repeindreTout();
void restaurerZone(int x, int y, int larg, int haut);
void apresAlerte(Sudoku s);

int main(){
  srand(time(NULL));
  int x, y;               // Coordonnée de la souris
  int lig, col;
  bool quitter = false;

  alloueBmp();            // Alloue les bitmaps
  Sudoku sudoku = initGrille();
  sudokuAffiche = &sudoku;
  definirRepeindreFond(repeindreTout);

  ouvrirFenetreAdaptable(FENETRE, FENETRE, "Sudoku - Rémy HUBSCHER");
  peindreTout(sudoku);

  while(!quitter){
      if(!attendreClicLogique(&x, &y)){
	peindreTout(sudoku);   // La fenetre a change de taille
	continue;
      }

      if(y >= 7 && y <= 39){ // On est dans la zone du menu
	if(x >= nouv.x && x <= nouv.x+32){
	  bouton = &nouv;
	  // Ici, on génére une grille vierge
	  if(alert(DIAL_X, DIAL_Y, "Voulez vous vraiment initialiser la grille ?"))
	    sudoku = initGrille();
	  bouton = NULL;
	  apresAlerte(sudoku);
	}
	else if(x >= ouvrir.x && x <= ouvrir.x+32){
	  bouton = &ouvrir;
	  // Ici, on génére une grille jouable
	  if(alert(DIAL_X, DIAL_Y, "Voulez vous vraiment charger une grille ?"))
	    sudoku = chargerGrille("grille.sdk");
	  bouton = NULL;
	  apresAlerte(sudoku);
	}
	else if(x >= rec.x && x <= rec.x+32){
	  bouton = &rec;
	  // Ici on enregistre le sudoku en PDF
	  if(alert(DIAL_X, DIAL_Y, "Voulez vous vraiment enregistrer la grille ? La grille précédente sera effacée")){
	    if(!sauvegarderGrille(sudoku, "grille.sdk")){
	      alert(DIAL_X, DIAL_Y, "Impossible d'enregistrer le sudoku. Etes vous sur des droits du fichier grille.sdk ?");
	      alert(DIAL_X, DIAL_Y, "Il serait bon de vérifier");
	    }
	  }
	  bouton = NULL;
	  apresAlerte(sudoku);
	}
	else if(x >= aide.x && x <= aide.x+32){
	  bouton = &aide;
	  // Ici, on affiche les règles du jeu
	  if(alert(DIAL_X, DIAL_Y, "Voulez vous vraiment avoir la solution ?")){
	    if(!resolve(&sudoku)){
	      if(alert(DIAL_X, DIAL_Y, "Aucune solution. Initialiser la grille ?"))
		sudoku = initGrille();
	    }
	  }
	  bouton = NULL;
	  apresAlerte(sudoku);
	}
	else if(x >= fermer.x && x <= fermer.x+32){
	  bouton = &fermer;
	  quitter = true;
	}else if(((x - 140) / 36) >= 0 && ((x - 140) / 36) < 10){
	  bouton = &b[((x - 140) / 36)];
	}else{
	  bouton = NULL;
	}
	menu();
      }else{
	// On a cliqué sur la grille mais sur quelle case ? //
	col = (((x - DECALX_GRILLE) / 182) * 3 + ((x - DECALX_GRILLE) % 182) / 52);
	if(col > 8) col = 8;
	if(col < 0) col = 0;

	lig = (((y - DECALY_GRILLE) / 182) * 3 + ((y - DECALY_GRILLE) % 182) / 52);
	if(lig > 8) lig = 8;
	if(lig < 0) lig = 0;
	
//	cerr << "(" << lig << ", " << col << ")\n";

	if(bouton != NULL && bouton->nb != 10){
	  if(placer(&sudoku, lig, col, bouton->nb))
	    majEcran(sudoku);
	}
      }
      
      if(bouton != NULL)
	bouton->down->affiche(bouton->x, 7);
    }

  desalloueBmp();          // Desalloue les bitmaps

  fermerFenetre();
}

void alloueBmp(){
  int i;
  char * chemin = new char[255];
  nouv.up   = new Bitmap(asset("new.bmp").c_str());
  nouv.down = new Bitmap(asset("newd.bmp").c_str());
  nouv.x    = 10;
  nouv.nb   = 10;

  ouvrir.up    = new Bitmap(asset("ouvrir.bmp").c_str());
  ouvrir.down  = new Bitmap(asset("ouvrird.bmp").c_str());
  ouvrir.x     = 50;
  ouvrir.nb   = 10;
  
  rec.up       = new Bitmap(asset("enregistrer.bmp").c_str());
  rec.down     = new Bitmap(asset("enregistrerd.bmp").c_str());
  rec.x        = 90;
  rec.nb   = 10;

  aide.up      = new Bitmap(asset("aide.bmp").c_str());
  aide.down    = new Bitmap(asset("aided.bmp").c_str());
  aide.x       = 510;
  aide.nb   = 10;

  fermer.up    = new Bitmap(asset("fermer.bmp").c_str());
  fermer.down  = new Bitmap(asset("fermerd.bmp").c_str());
  fermer.x     = 550;
  fermer.nb   = 10;

  erreur.up   = new Bitmap(asset("erreur.bmp").c_str());

  b = new Bouton[10];

  for(i=0; i < 10; i++){
    snprintf(chemin, 255, "%s", asset(std::to_string(i) + ".bmp").c_str());
    b[i].up = new Bitmap(chemin);

    snprintf(chemin, 255, "%s", asset(std::to_string(i) + "d.bmp").c_str());
    b[i].down = new Bitmap(chemin);

    b[i].x = 140 + i*36;
    b[i].nb= i;
  }

  delete[] chemin;
}

void desalloueBmp(){
  int i;
  delete nouv.up;
  delete ouvrir.up;
  delete rec.up;
  delete aide.up;
  delete fermer.up;
  delete erreur.up;

  delete nouv.down;
  delete ouvrir.down;
  delete rec.down;
  delete aide.down;
  delete fermer.down;

  for(i=0; i < 10; i++){
    delete b[i].up;
  }

  for(i=0; i < 10; i++){
    delete b[i].down;
  }

  delete[] b;
}

void menu(){
  int i;
    // Limite du menu //
  ligne(0, 46, FENETRE, 46);
  
  nouv.up->affiche(nouv.x, 7);
  ouvrir.up->affiche(ouvrir.x, 7);
  rec.up->affiche(rec.x, 7);
  aide.up->affiche(aide.x, 7);
  fermer.up->affiche(fermer.x, 7);


  for(i=0; i < 10; i++){
    b[i].up->affiche(140 + i*36, 7);
  }
}

void positionCase(int lig, int col, int * x, int * y){
  *x = DECALX_GRILLE + (col / 3) * 182 + (col % 3) * 52;
  *y = DECALY_GRILLE + (lig / 3) * 182 + (lig % 3) * 52;
}

void majEcran(Sudoku s){
  for(int i = 0; i < 9; i++){
    for(int j = 0; j < 9; j++){
      int code = s.grille[i][j] * 2 + (s.fixe[i][j] ? 1 : 0);
      if(code == ecran[i][j])
	continue;
      int x, y;
      positionCase(i, j, &x, &y);
      if(s.fixe[i][j])
	b[s.grille[i][j]].down->affiche(x, y);
      else
	b[s.grille[i][j]].up->affiche(x, y);
      ecran[i][j] = code;
    }
  }
}

void peindreTout(Sudoku s){
  viderFenetre();
  Bitmap::invaliderTout();
  for(int i = 0; i < 9; i++)
    for(int j = 0; j < 9; j++)
      ecran[i][j] = -1;
  menu();
  majEcran(s);
  if(bouton != NULL)
    bouton->down->affiche(bouton->x, 7);
}

void repeindreTout(){
  if(sudokuAffiche != NULL)
    peindreTout(*sudokuAffiche);
}

const int TAILLE_CASE = 32;

/* Efface la zone du dialogue ainsi que les cases qu'il recouvre, puis marque
   ces cases comme a redessiner. */
void restaurerZone(int x, int y, int larg, int haut){
  int minX = x, minY = y, maxX = x + larg, maxY = y + haut;

  for(int i = 0; i < 9; i++){
    for(int j = 0; j < 9; j++){
      int cx, cy;
      positionCase(i, j, &cx, &cy);
      if(cx < x + larg && cx + TAILLE_CASE > x && cy < y + haut && cy + TAILLE_CASE > y){
	if(cx < minX) minX = cx;
	if(cy < minY) minY = cy;
	if(cx + TAILLE_CASE > maxX) maxX = cx + TAILLE_CASE;
	if(cy + TAILLE_CASE > maxY) maxY = cy + TAILLE_CASE;
	ecran[i][j] = -1;
      }
    }
  }

  b[0].up->remplirFond(minX, minY, maxX, maxY);
  Bitmap::invaliderZone(minX, minY, maxX + 1, maxY + 1);
}

void apresAlerte(Sudoku s){
  restaurerZone(DIAL_X, DIAL_Y, DIAL_W, DIAL_H);
  majEcran(s);
}
