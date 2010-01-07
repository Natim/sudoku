#include "BoiteDialogue.h"


bool alert(int x, int y, string message){
  return alert(x, y, "/!\\ Attention /!\\", message);
}

bool alert(int x, int y, string titre, string message){
  return alert(x, y, 300, 100, message, titre);
}

bool alert(int x, int y, int width, int height, string message, string titre){
  initPalette(NULL, 0);
  const int LARGEUR = width;
  const int HAUTEUR = height;

  int X, Y;
  /* Fond */
  modifierCouleur(20);
  remplirRectangle(x, y, x+LARGEUR, y+HAUTEUR);
  
  /* Titre */
  modifierCouleur(238);
  remplirRectangle(x, y, x+LARGEUR, y+20);
  modifierCouleur(1);
  ecrireSurImpression(x + (LARGEUR-titre.size()*6)/2, y+15, (char *) titre.c_str());
  tracerLigne(x + (LARGEUR-titre.size()*6)/2, y+17, x +(LARGEUR-titre.size()*6)/2 + titre.size()*6, y+17);
  
  /* Texte */
  modifierCouleur(1);
  ecrireSurImpression(x + 20, y + 45, (char *) message.c_str());
  
  /* Boutons */
  modifierCouleur(1);
  tracerRectangle(x + 25, y + HAUTEUR - 30, x + 55, y + HAUTEUR - 10);
  ecrireSurImpression(x + 32, y + HAUTEUR - 15, "Oui");
  
  modifierCouleur(1);
  tracerRectangle(x + LARGEUR - 55, y + HAUTEUR - 30, x + LARGEUR - 25, y + HAUTEUR - 10);
  ecrireSurImpression(x + LARGEUR - 48, y +  HAUTEUR - 15, "Non");
  
  /* Contours */
  modifierCouleur(0);
  tracerRectangle(x, y, x+LARGEUR, y+HAUTEUR);
  tracerRectangle(x, y+20, x+LARGEUR, y+HAUTEUR);
  while(true){
    if(testerClic()){
      positionSouris(&X, &Y);
      if(Y > y + HAUTEUR - 30 && Y < y + HAUTEUR - 10){
	if(X > x + 25 && X < x + 55)
	  return true;
	else if(X > x + LARGEUR - 55 && X < x + LARGEUR - 25)
	  return false;
      }
    }
    usleep(10000);
    rafraichirFenetre();
  }
}
