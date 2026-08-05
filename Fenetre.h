//------------------------------------------------------//
// Fenetre.h
// Fenetre redimensionnable a ratio constant pour Graphlib
//-----------------------------------------------------//

#ifndef __FENETRE_H_
#define __FENETRE_H_

/*
  Le programme raisonne toujours dans les coordonnees de la fenetre de
  reference passee a ouvrirFenetreAdaptable(). Ce module les convertit en
  pixels ecran selon la taille courante de la fenetre, et signale les
  redimensionnements pour que l'appelant se repeigne.
*/

void ouvrirFenetreAdaptable(int larg, int haut, const char * titre);

double echelle();
int pixX(int x);
int pixY(int y);

bool attendreClicLogique(int * x, int * y);
/*
  Attend un clic et fournit sa position en coordonnees logiques.
  Renvoie false si la fenetre a change de taille : l'appelant doit alors
  tout repeindre avant de rappeler la fonction.
*/

// Primitives graphlib prenant des coordonnees logiques.
void ligne(int x1, int y1, int x2, int y2);
void rectangle(int x1, int y1, int x2, int y2);
void rectanglePlein(int x1, int y1, int x2, int y2);
void ecrire(int x, int y, const char * texte);

#endif //__FENETRE_H_
