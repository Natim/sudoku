#pragma once

void ouvrirFenetreAdaptable(int larg, int haut, const char * titre);

double echelle();
int pixX(int x);
int pixY(int y);

bool attendreClicLogique(int * x, int * y);

void ligne(int x1, int y1, int x2, int y2);
void rectangle(int x1, int y1, int x2, int y2);
void rectanglePlein(int x1, int y1, int x2, int y2);
void ecrire(int x, int y, const char * texte);
