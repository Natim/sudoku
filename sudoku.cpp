extern "C"{
#include "lib/graphlib.h"
}
#include "Bitmap.class.h"
#include "Sudoku.struct.h"
#include "BoiteDialogue.h"
#include <unistd.h>

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
const int LARGEUR_3CASES= 104;
const int FENETRE = 600;

// Variable Globale //
Bouton nouv;
Bouton ouvrir;
Bouton rec;
Bouton aide;
Bouton fermer;
Bouton erreur;
Bouton * b;
Bouton * bouton = NULL;   // Bouton sur lequel on appuie

// Prototypes //
void alloueBmp();
void desalloueBmp();
void afficheSudoku(Sudoku s);
void affiche(int x, int y, int nb);
void menu();
void refresh(Sudoku s);

int main(){
  srand(time(NULL));
  int x, y;               // Coordonnée de la souris
  int lig, col;
  bool quitter = false;

  alloueBmp();            // Alloue les bitmaps
  Sudoku sudoku = initGrille();

  ouvrirFenetreTailleTitre(FENETRE, FENETRE, "Sudoku - Rémy HUBSCHER(c)");
  refresh(sudoku);

  while(!quitter){
    if(testerClic()){
      positionSouris(&x, &y);
      
      if(y >= 7 && y <= 39){ // On est dans la zone du menu
	if(x >= nouv.x && x <= nouv.x+32){
	  bouton = &nouv;
	  // Ici, on génére une grille vierge
	  if(alert((FENETRE - 300)/2, (FENETRE - 100)/2, "Voulez vous vraiment initialiser la grille ?"))
	    sudoku = initGrille();
	  
	  refresh(sudoku);
	  bouton = NULL;
	}
	else if(x >= ouvrir.x && x <= ouvrir.x+32){
	  bouton = &ouvrir;
	  // Ici, on génére une grille jouable
	  if(alert((FENETRE - 300)/2, (FENETRE - 100)/2, "Voulez vous vraiment charger une grille ?"))
	    sudoku = chargerGrille("grille.sdk");
	  refresh(sudoku);
	  bouton = NULL;
	}
	else if(x >= rec.x && x <= rec.x+32){
	  bouton = &rec;
	  // Ici on enregistre le sudoku en PDF
	  if(alert((FENETRE - 300)/2, (FENETRE - 100)/2, "Voulez vous vraiment enregistrer la grille ? La grille précédente sera effacée"))
	    if(!sauvegarderGrille(sudoku, "grille.sdk")){
	      alert((FENETRE - 300)/2, (FENETRE - 100)/2, "Impossible d'enregistrer le sudoku. Etes vous sur des droits du fichier grille.sdk ?");
	      alert((FENETRE - 300)/2, (FENETRE - 100)/2, "Il serait bon de vérifier");
	    }
	  refresh(sudoku);
	  bouton = NULL;
	}
	else if(x >= aide.x && x <= aide.x+32){
	  bouton = &aide;
	  // Ici, on affiche les règles du jeu
	  if(alert((FENETRE - 300)/2, (FENETRE - 100)/2, "Voulez vous vraiment avoir la solution ?")){
	    if(!resolve(&sudoku))
	      	  if(alert((FENETRE - 300)/2, (FENETRE - 100)/2, "Aucune solution. Initialiser la grille ?"))
		    sudoku = initGrille();
	  }
	  refresh(sudoku);
	  bouton = NULL;
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
	    bouton->down->affiche((col / 3) * 182 + DECALX_GRILLE + (col % 3) * 52, (lig / 3) * 182 + DECALY_GRILLE + (lig % 3) * 52);
	}
	//refresh(sudoku);
      }
      
      if(bouton != NULL)
	bouton->down->affiche(bouton->x, 7);
    }
    usleep(10000);
    rafraichirFenetre();
  }

  desalloueBmp();          // Desalloue les bitmaps

  fermerFenetre();
}

void alloueBmp(){
  int i;
  char * chemin = new char[255];
  nouv.up   = new Bitmap("images/new.bmp");
  nouv.down = new Bitmap("images/newd.bmp");
  nouv.x    = 10;
  nouv.nb   = 10;

  ouvrir.up    = new Bitmap("images/ouvrir.bmp");
  ouvrir.down  = new Bitmap("images/ouvrird.bmp");
  ouvrir.x     = 50;
  ouvrir.nb   = 10;
  
  rec.up       = new Bitmap("images/enregistrer.bmp");
  rec.down     = new Bitmap("images/enregistrerd.bmp");
  rec.x        = 90;
  rec.nb   = 10;

  aide.up      = new Bitmap("images/aide.bmp");
  aide.down    = new Bitmap("images/aided.bmp");
  aide.x       = 510;
  aide.nb   = 10;

  fermer.up    = new Bitmap("images/fermer.bmp");
  fermer.down  = new Bitmap("images/fermerd.bmp");
  fermer.x     = 550;
  fermer.nb   = 10;

  erreur.up   = new Bitmap("images/erreur.bmp");

  b = new Bouton[10];

  for(i=0; i < 10; i++){
    sprintf(chemin, "images/%d.bmp", i);
    b[i].up = new Bitmap(chemin);

    sprintf(chemin, "images/%dd.bmp", i);
    b[i].down = new Bitmap(chemin);

    b[i].x = 140 + i*36;
    b[i].nb= i;
  }
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

  delete b;
}

void menu(){
  int i;
    // Limite du menu //
  tracerLigne(0, 46, 600, 46);
  
  nouv.up->affiche(nouv.x, 7);
  ouvrir.up->affiche(ouvrir.x, 7);
  rec.up->affiche(rec.x, 7);
  aide.up->affiche(aide.x, 7);
  fermer.up->affiche(fermer.x, 7);


  for(i=0; i < 10; i++){
    b[i].up->affiche(140 + i*36, 7);
  }
}

void afficheSudoku(Sudoku s){
  int i, j;
  int decalX = DECALX_GRILLE, decalY = DECALY_GRILLE;

  for(i = 0; i < 9; i++){
    for(j = 0; j < 9; j++){
      if(s.fixe[i][j])
	b[s.grille[i][j]].down->affiche(decalX, decalY);
      else
	affiche(decalX, decalY, s.grille[i][j]);
	//b[s.grille[i][j]].up->affiche(decalX, decalY);

      if(j == 2 || j == 5 || j == 8)
	decalX += 78;
      else{
	decalX += 52;
      }
    }
    decalX = DECALX_GRILLE;
    if(i == 2 || i == 5)
      decalY += 78;
    else
      decalY += 52;      
    
  }
}

void affiche(int x, int y, int nb){
  recupereSousImage(b[nb].x, 7, b[nb].x + 32, 39);
  afficheSousImage(x, y);
}

void refresh(Sudoku s){
  viderFenetre();
  menu();
  afficheSudoku(s);
  if(bouton != NULL)
    bouton->down->affiche(bouton->x, 7);
}
