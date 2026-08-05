#include "dialog_box.h"
#include "window.h"

extern "C"{
#include "graphlib.h"
}

#include <string>
#include <vector>

using namespace std;

static void (*repeindreFond)() = NULL;

struct ZoneBouton {
  int x1, y1, x2, y2;
};

void definirRepeindreFond(void (*peindre)()){
  repeindreFond = peindre;
}

static vector<ZoneBouton> disposerBoutons(int x, int y, int largeur, int hauteur,
					  const vector<string> & libelles){
  const int marge = 10;
  const int y1 = y + hauteur - 30;
  const int y2 = y + hauteur - 10;
  const int inter = 8;

  vector<int> largeurs;
  largeurs.reserve(libelles.size());
  int total = 0;
  for(unsigned int i = 0; i < libelles.size(); i++){
    int larg = largeurTexte(libelles[i].c_str()) + 16;
    if(larg < 30)
      larg = 30;
    largeurs.push_back(larg);
    total += larg;
  }
  if(!libelles.empty())
    total += inter * ((int) libelles.size() - 1);

  int debut = x + marge;
  if(debut + total < x + largeur - marge)
    debut = x + (largeur - total) / 2;

  vector<ZoneBouton> zones;
  int courant = debut;
  for(unsigned int i = 0; i < libelles.size(); i++){
    ZoneBouton zone;
    zone.x1 = courant;
    zone.y1 = y1;
    zone.x2 = courant + largeurs[i];
    zone.y2 = y2;
    zones.push_back(zone);
    courant += largeurs[i] + inter;
  }
  return zones;
}

static void dessinerBoite(int x, int y, int largeur, int hauteur,
			  const string & message, const string & titre,
			  const vector<string> & libelles){
  initPalette(NULL, 0);

  modifierCouleur(20);
  rectanglePlein(x, y, x + largeur, y + hauteur);

  modifierCouleur(238);
  rectanglePlein(x, y, x + largeur, y + 20);
  modifierCouleur(1);
  const int largeurTitre = largeurTexte(titre.c_str());
  const int debutTitre   = x + (largeur - largeurTitre) / 2;
  ecrire(debutTitre, y + 15, titre.c_str());
  ligne(debutTitre, y + 17, debutTitre + largeurTitre, y + 17);

  modifierCouleur(1);
  ecrire(x + 20, y + 45, message.c_str());

  vector<ZoneBouton> zones = disposerBoutons(x, y, largeur, hauteur, libelles);
  for(unsigned int i = 0; i < libelles.size(); i++){
    modifierCouleur(1);
    rectangle(zones[i].x1, zones[i].y1, zones[i].x2, zones[i].y2);
    const int largeurLib = largeurTexte(libelles[i].c_str());
    const int texteX = zones[i].x1 + (zones[i].x2 - zones[i].x1 - largeurLib) / 2;
    ecrire(texteX, zones[i].y2 - 5, libelles[i].c_str());
  }

  modifierCouleur(0);
  rectangle(x, y, x + largeur, y + hauteur);
  rectangle(x, y + 20, x + largeur, y + hauteur);
}

static int boutonClique(int X, int Y, const vector<ZoneBouton> & zones){
  for(unsigned int i = 0; i < zones.size(); i++){
    if(X > zones[i].x1 && X < zones[i].x2 &&
       Y > zones[i].y1 && Y < zones[i].y2)
      return (int) i;
  }
  return -1;
}

int choisir(int x, int y, int largeur, int hauteur,
	    string titre, string message,
	    const vector<string> & choix){
  int X, Y;
  vector<ZoneBouton> zones = disposerBoutons(x, y, largeur, hauteur, choix);
  dessinerBoite(x, y, largeur, hauteur, message, titre, choix);

  while(true){
    if(!attendreClicLogique(&X, &Y)){
      if(repeindreFond != NULL)
	repeindreFond();
      zones = disposerBoutons(x, y, largeur, hauteur, choix);
      dessinerBoite(x, y, largeur, hauteur, message, titre, choix);
      continue;
    }

    int idx = boutonClique(X, Y, zones);
    if(idx >= 0)
      return idx;
  }
}

bool alert(int x, int y, string message){
  return alert(x, y, "/!\\ Attention /!\\", message);
}

bool alert(int x, int y, string titre, string message){
  return alert(x, y, 300, 100, message, titre);
}

bool alert(int x, int y, int largeur, int hauteur, string message, string titre){
  vector<string> boutons;
  boutons.push_back("Oui");
  boutons.push_back("Non");
  return choisir(x, y, largeur, hauteur, titre, message, boutons) == 0;
}
