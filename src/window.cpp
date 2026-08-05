//------------------------------------------------------//
// Fenetre.cpp
// Fenetre redimensionnable a ratio constant pour Graphlib
//-----------------------------------------------------//

extern "C"{
#include "graphlib.h"
}
#include "window.h"
#include <cmath>

/*
  Graphlib expose les objets X11 qu'elle manipule. On s'en sert pour deux
  choses qu'elle ne sait pas faire : demander au gestionnaire de fenetres de
  conserver le ratio, et redimensionner le pixmap de double tampon dans lequel
  toutes ses primitives dessinent.
*/
extern "C" {
  extern Display  * mydisplay;
  extern Window     mywindow;
  extern Pixmap     dblbuff;
  extern XSizeHints myhint;
  extern int        profondeur;
}

// Taille de police correspondant a une echelle de 1.
static const int TAILLE_TEXTE = 12;

static const long MASQUE_EVENEMENTS =
  KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask |
  EnterWindowMask | LeaveWindowMask | KeymapStateMask | ExposureMask |
  FocusChangeMask | StructureNotifyMask;

static int largeurRef = 1, hauteurRef = 1;  // taille de la fenetre de reference
static double facteur = 1.0;                // pixels ecran par unite logique
static int margeX = 0, margeY = 0;          // centrage si le ratio n'est pas respecte

double echelle(){
  return facteur;
}

int pixX(int x){
  return margeX + (int) lround(x * facteur);
}

int pixY(int y){
  return margeY + (int) lround(y * facteur);
}

static int logiqueX(int x){
  return (int) floor((x - margeX) / facteur);
}

static int logiqueY(int y){
  return (int) floor((y - margeY) / facteur);
}

static void adapterTaille(int larg, int haut){
  if(larg <= 0 || haut <= 0)
    return;

  double fx = larg / (double) largeurRef;
  double fy = haut / (double) hauteurRef;
  facteur = (fx < fy) ? fx : fy;
  margeX = (larg - (int) lround(largeurRef * facteur)) / 2;
  margeY = (haut - (int) lround(hauteurRef * facteur)) / 2;

  // Sans ce nouveau pixmap, les zones reaffichees apres un Expose seraient
  // tronquees a l'ancienne taille de la fenetre.
  XFreePixmap(mydisplay, dblbuff);
  dblbuff = XCreatePixmap(mydisplay, mywindow, larg, haut, profondeur);
  myhint.width  = larg;
  myhint.height = haut;
  viderFenetre();

  // Sans les polices adobe-times 75/100 dpi la taille reste celle par defaut.
  modifierTailleTexte((int) lround(TAILLE_TEXTE * facteur));
}

void ouvrirFenetreAdaptable(int larg, int haut, const char * titre){
  largeurRef = larg;
  hauteurRef = haut;

  ouvrirFenetreTailleTitre(larg, haut, (char *) titre);

  XSizeHints * contraintes = XAllocSizeHints();
  contraintes->flags = PSize | PMinSize | PBaseSize | PAspect;
  contraintes->width  = larg;
  contraintes->height = haut;
  contraintes->min_width  = largeurRef / 4;
  contraintes->min_height = hauteurRef / 4;
  // Les ratios s'appliquent a la taille diminuee de la taille de base.
  contraintes->base_width = contraintes->base_height = 0;
  contraintes->min_aspect.x = contraintes->max_aspect.x = larg;
  contraintes->min_aspect.y = contraintes->max_aspect.y = haut;
  XSetWMNormalHints(mydisplay, mywindow, contraintes);
  XFree(contraintes);

  // Graphlib n'ecoute pas les ConfigureNotify.
  XSelectInput(mydisplay, mywindow, MASQUE_EVENEMENTS);

  adapterTaille(larg, haut);
}

bool attendreClicLogique(int * x, int * y){
  bool redimensionnee = false;
  bool aRafraichir    = false;

  for(;;){
    // Une fois la file vide, les redimensionnements successifs d'un
    // glissement de souris ont ete fusionnes en un seul repeignage.
    if(!XPending(mydisplay)){
      if(redimensionnee)
	return false;
      if(aRafraichir){
	rafraichirFenetre();
	aRafraichir = false;
      }
    }

    XEvent evenement;
    XNextEvent(mydisplay, &evenement);

    switch(evenement.type){
    case ButtonPress:
      if(redimensionnee){
	XPutBackEvent(mydisplay, &evenement);
	return false;
      }
      *x = logiqueX(evenement.xbutton.x);
      *y = logiqueY(evenement.xbutton.y);
      return true;

    case ConfigureNotify:
      if(evenement.xconfigure.width  != myhint.width ||
	 evenement.xconfigure.height != myhint.height){
	adapterTaille(evenement.xconfigure.width, evenement.xconfigure.height);
	redimensionnee = true;
      }
      break;

    case Expose:
      aRafraichir = true;
      break;
    }
  }
}

void ligne(int x1, int y1, int x2, int y2){
  tracerLigne(pixX(x1), pixY(y1), pixX(x2), pixY(y2));
}

void rectangle(int x1, int y1, int x2, int y2){
  tracerRectangle(pixX(x1), pixY(y1), pixX(x2), pixY(y2));
}

void rectanglePlein(int x1, int y1, int x2, int y2){
  remplirRectangle(pixX(x1), pixY(y1), pixX(x2), pixY(y2));
}

void ecrire(int x, int y, const char * texte){
  ecrireSurImpression(pixX(x), pixY(y), (char *) texte);
}
