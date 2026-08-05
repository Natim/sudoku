#include "dialog_box.h"
#include "window.h"

extern "C"{
#include "graphlib.h"
}

#include <iostream>
#include <string>

using namespace std;

static void (*repeindreFond)() = NULL;

void definirRepeindreFond(void (*peindre)()){
  repeindreFond = peindre;
}

static void dessinerBoite(int x, int y, int LARGEUR, int HAUTEUR,
			  const string & message, const string & titre){
  initPalette(NULL, 0);

  /* Fond */
  modifierCouleur(20);
  rectanglePlein(x, y, x+LARGEUR, y+HAUTEUR);

  /* Titre */
  modifierCouleur(238);
  rectanglePlein(x, y, x+LARGEUR, y+20);
  modifierCouleur(1);
  const int largeurTitre = largeurTexte(titre.c_str());
  const int debutTitre   = x + (LARGEUR - largeurTitre)/2;
  ecrire(debutTitre, y+15, titre.c_str());
  ligne(debutTitre, y+17, debutTitre + largeurTitre, y+17);

  /* Texte */
  modifierCouleur(1);
  ecrire(x + 20, y + 45, message.c_str());

  /* Boutons */
  modifierCouleur(1);
  rectangle(x + 25, y + HAUTEUR - 30, x + 55, y + HAUTEUR - 10);
  ecrire(x + 32, y + HAUTEUR - 15, "Oui");

  modifierCouleur(1);
  rectangle(x + LARGEUR - 55, y + HAUTEUR - 30, x + LARGEUR - 25, y + HAUTEUR - 10);
  ecrire(x + LARGEUR - 48, y +  HAUTEUR - 15, "Non");

  /* Contours */
  modifierCouleur(0);
  rectangle(x, y, x+LARGEUR, y+HAUTEUR);
  rectangle(x, y+20, x+LARGEUR, y+HAUTEUR);
}

bool alert(int x, int y, string message){
  return alert(x, y, "/!\\ Attention /!\\", message);
}

bool alert(int x, int y, string titre, string message){
  return alert(x, y, 300, 100, message, titre);
}

bool alert(int x, int y, int width, int height, string message, string titre){
  const int LARGEUR = width;
  const int HAUTEUR = height;

  int X, Y;
  dessinerBoite(x, y, LARGEUR, HAUTEUR, message, titre);

  while(true){
      if(!attendreClicLogique(&X, &Y)){
	// La fenetre a change de taille : le fond puis la boite sont a refaire.
	if(repeindreFond != NULL)
	  repeindreFond();
	dessinerBoite(x, y, LARGEUR, HAUTEUR, message, titre);
	continue;
      }

      if(Y > y + HAUTEUR - 30 && Y < y + HAUTEUR - 10){
	if(X > x + 25 && X < x + 55)
	  return true;
	else if(X > x + LARGEUR - 55 && X < x + LARGEUR - 25)
	  return false;
      }
    }
}
