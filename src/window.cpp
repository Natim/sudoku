//------------------------------------------------------//
// window.cpp
// Fenetre redimensionnable a ratio constant pour Graphlib
//-----------------------------------------------------//

extern "C"{
#include "graphlib.h"
}
#include "window.h"
#include "encoding.h"
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <sys/select.h>

/*
  Graphlib expose les objets X11 qu'elle manipule. On s'en sert pour deux
  choses qu'elle ne sait pas faire : demander au gestionnaire de fenetres de
  conserver le ratio, et redimensionner le pixmap de double tampon dans lequel
  toutes ses primitives dessinent.
*/
extern "C" {
  extern Display  * mydisplay;
  extern Window     mywindow;
  extern GC         mygc;
  extern Pixmap     dblbuff;
  extern XSizeHints myhint;
  extern int        profondeur;
}

// Taille de police correspondant a une echelle de 1.
static const int TAILLE_TEXTE = 12;

// Duree d'accalmie avant de repeindre : un glissement de souris produit
// beaucoup plus de ConfigureNotify que le trace ne peut en suivre.
static const int DELAI_REDIM_MS = 150;

// Au dela, on renonce a imposer le ratio et on se contente de centrer.
static const int MAX_CORRECTIONS = 3;

static const long MASQUE_EVENEMENTS =
  KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask |
  EnterWindowMask | LeaveWindowMask | KeymapStateMask | ExposureMask |
  FocusChangeMask | StructureNotifyMask;

static int largeurRef = 1, hauteurRef = 1;  // taille de la fenetre de reference
static double facteur = 1.0;                // pixels ecran par unite logique
static int margeX = 0, margeY = 0;          // centrage si le ratio n'est pas respecte
static XFontStruct * police = NULL;         // police au facteur courant

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

/*
  modifierTailleTexte() de graphlib est inutilisable : son motif XLFD omet le
  champ de largeur moyenne, si bien qu'il ne correspond a aucune police. On
  charge donc la police nous-memes. Les familles vectorielles du serveur X
  acceptent n'importe quelle taille en pixels ; les motifs sont essayes dans
  l'ordre pour s'adapter aux polices reellement installees.
*/
static void adapterPolice(){
  static const char * motifs[] = {
    "-*-times-medium-r-normal--%d-*-*-*-*-*-iso8859-1",
    "-*-helvetica-medium-r-normal--%d-*-*-*-*-*-iso8859-1",
    "-*-fixed-medium-r-normal--%d-*-*-*-*-*-iso8859-1"
  };

  int taille = (int) lround(TAILLE_TEXTE * facteur);
  if(taille < 1)
    taille = 1;

  for(unsigned int i = 0; i < sizeof motifs / sizeof *motifs; i++){
    char nom[128];
    snprintf(nom, sizeof nom, motifs[i], taille);

    // XLoadQueryFont renvoie NULL au lieu de declencher une erreur X.
    XFontStruct * nouvelle = XLoadQueryFont(mydisplay, nom);
    if(nouvelle == NULL)
      continue;

    XSetFont(mydisplay, mygc, nouvelle->fid);
    if(police != NULL)
      XFreeFont(mydisplay, police);
    police = nouvelle;
    return;
  }
}

int largeurTexte(const char * texte){
  std::string latin1 = utf8ToLatin1(texte);
  if(police == NULL)
    return latin1.size() * TAILLE_TEXTE / 2;
  return (int) lround(XTextWidth(police, latin1.c_str(), latin1.size()) / facteur);
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

  adapterPolice();
}

void ouvrirFenetreAdaptable(int larg, int haut, const char * titre){
  largeurRef = larg;
  hauteurRef = haut;

  ouvrirFenetreTailleTitre(larg, haut, (char *) titre);

  // graphlib sets the title via XSetStandardProperties (Latin-1).
  Xutf8SetWMProperties(mydisplay, mywindow, titre, titre, NULL, 0, NULL, NULL, NULL);

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

// Attend un evenement, au plus delaiMs millisecondes.
static bool attendreEvenement(int delaiMs){
  if(XPending(mydisplay))
    return true;

  int fd = ConnectionNumber(mydisplay);
  fd_set lecture;
  FD_ZERO(&lecture);
  FD_SET(fd, &lecture);

  struct timeval delai;
  delai.tv_sec  = delaiMs / 1000;
  delai.tv_usec = (delaiMs % 1000) * 1000;

  return select(fd + 1, &lecture, NULL, NULL, &delai) > 0;
}

/*
  Taille la plus proche respectant le ratio de reference. On suit l'axe que
  l'utilisateur a le plus deplace, sinon tirer un bord horizontal annulerait
  son geste en ramenant la fenetre a sa largeur d'origine.
*/
static void tailleAuRatio(int larg, int haut, int * cibleL, int * cibleH){
  double f;
  if(abs(larg - myhint.width) >= abs(haut - myhint.height))
    f = larg / (double) largeurRef;
  else
    f = haut / (double) hauteurRef;

  int l = (int) lround(largeurRef * f);
  int h = (int) lround(hauteurRef * f);

  // Rester au dessus du minimum annonce, sinon le gestionnaire de fenetres
  // corrigerait a son tour et on repartirait pour un tour.
  if(l < largeurRef / 4 || h < hauteurRef / 4){
    l = largeurRef / 4;
    h = hauteurRef / 4;
  }

  *cibleL = l;
  *cibleH = h;
}

bool attendreClicLogique(int * x, int * y){
  bool aRafraichir = false;
  int corrections  = 0;
  int largeurVoulue = myhint.width, hauteurVoulue = myhint.height;

  for(;;){
    bool aRedimensionner = (largeurVoulue != myhint.width ||
			    hauteurVoulue != myhint.height);

    if(!XPending(mydisplay)){
      if(aRedimensionner){
	// Un glissement de souris enchaine les ConfigureNotify : on attend
	// une accalmie pour ne repeindre qu'une fois, a la taille finale.
	if(!attendreEvenement(DELAI_REDIM_MS)){
	  int cibleL, cibleH;
	  tailleAuRatio(largeurVoulue, hauteurVoulue, &cibleL, &cibleH);

	  // PAspect n'est pas toujours applique lors d'un redimensionnement
	  // a la souris : on rectifie une fois le geste termine.
	  if((cibleL != largeurVoulue || cibleH != hauteurVoulue) &&
	     corrections < MAX_CORRECTIONS){
	    corrections++;
	    XResizeWindow(mydisplay, mywindow, cibleL, cibleH);
	    continue;
	  }

	  adapterTaille(largeurVoulue, hauteurVoulue);
	  return false;
	}
      }else if(aRafraichir){
	rafraichirFenetre();
	aRafraichir = false;
      }
    }

    XEvent evenement;
    XNextEvent(mydisplay, &evenement);

    switch(evenement.type){
    case ButtonPress:
      if(aRedimensionner){
	XPutBackEvent(mydisplay, &evenement);
	adapterTaille(largeurVoulue, hauteurVoulue);
	return false;
      }
      *x = logiqueX(evenement.xbutton.x);
      *y = logiqueY(evenement.xbutton.y);
      return true;

    case ConfigureNotify:
      largeurVoulue = evenement.xconfigure.width;
      hauteurVoulue = evenement.xconfigure.height;
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
  std::string latin1 = utf8ToLatin1(texte);
  ecrireSurImpression(pixX(x), pixY(y), (char *) latin1.c_str());
}
