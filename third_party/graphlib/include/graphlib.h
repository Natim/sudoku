/*********************************************************************/
/* Bibliothèque graphique graphlib pour l'enseignement universitaire */
/* D'après les idées de Françoise et Jean-Paul Bertrandias sur Atari */
/* Implémentation sous X Windows: Michel Bonin et Xavier Girod       */
/* UFR IMA - Université Joseph Fourier (Grenoble)                    */
/* Centre Joseph Fourier Drôme-Ardèche (Valence)                     */
/* email: Michel.Bonin@ujf-grenoble.fr                               */
/* modification : 1999-2001                                          */
/*                Sylvain Contassot-Vivier                           */
/*                IUT Belfort-Montbeliard                            */
/*                Sylvain.Contassot-Vivier@iut-bm.univ-fcomte.fr     */
/*********************************************************************/


/* Fichier d'en-tête */

#ifndef _GRAPHLIB_H
#define _GRAPHLIB_H

/* Fichiers X inclus */

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

#define XSize  640
#define YSize  480

/*************************************************************************/

/* initialisation de l'ecran graphique */

void ouvrirFenetre(void);

/*************************************************************************/

/* initialisation de l'e'cran graphique 
   cree une fenetre X-Window de taille larg X haut */

void ouvrirFenetreTaille(int larg, int haut);

/*************************************************************************/

/* initialisation de l'e'cran graphique 
   cree une fenetre X-Window de taille larg X haut avec un titre */

void ouvrirFenetreTailleTitre(int larg, int haut, char titre[]);

/*************************************************************************/

/* fermeture de l'e'cran graphique */

void fermerFenetre(void);

/*************************************************************************/

int modifierTailleTexte(int taille);

/*************************************************************************/

/* ecriture avec couleur coul sur fond noir */
void modifierCouleur(int coul);

/*************************************************************************/

/* fond avec couleur coul sur fond noir */
void modifierFond(int coul);

/*************************************************************************/

void videoInv(void);

/* ecriture de texte en blanc sur fond noir */

/*************************************************************************/

void graphInv(void);

/* dessin en blanc sur fond noir */

/*************************************************************************/

void videoNor(void);

/* ecriture de texte en noir sur fond blanc */

/*************************************************************************/

void graphNor(void);

/* dessin en noir sur fond blanc */

/*************************************************************************/

/* effacement de l'e'cran */

void viderFenetre(void);

/*************************************************************************/

/* rafraichissement de l'e'cran */

void rafraichirFenetre(void);

/*************************************************************************/

/* rafraichirZone : rafraichi uen zone rectangulaire de l'e'cran */

void rafraichirZone(int x,int y,int width,int height);

/*************************************************************************/

/* trace une ligne */

void tracerLigne(int x1,int y1,int x2,int y2);

/*************************************************************************/

/* efface une ligne */

void effacerLigne(int x1,int y1,int x2,int y2);

/*************************************************************************/

/* affichage du point (x,y) */

void afficherPoint(int x,int y);

/*************************************************************************/

/* effacement du point (x,y) */

void effacerPoint(int x,int y);

/***********************************************************************/

/* remplit un triangle */

void remplirTriangle(int x1,int y1,int x2,int y2,int x3,int y3);

/***********************************************************************/

/* trace un triangle */

void tracerTriangle(int x1,int y1,int x2,int y2,int x3,int y3);

/*************************************************************************/

/* trace un cercle */

void tracerCercle (int x,int y,int R);

/*************************************************************************/

/* remplit un cercle */

void remplirCercle (int x,int y,int R);

/*************************************************************************/

/* efface un cercle */

void effacerCercle(int x,int y,int R);

/*************************************************************************/

/* trace' d'un rectangle dont les extre'mite's de la diagonale sont      */
/* (x1, y1) et (x2, y2)                                                  */

void tracerRectangle (int x1,int y1,int x2,int y2);

/*************************************************************************/

/* remplit un rectangle dont les extre'mite's de la diagonale sont        */
/* (x1, y1) et (x2, y2)                                                   */

void remplirRectangle (int x1,int y1,int x2,int y2);

/*************************************************************************/

/* effacement d'un rectangle dont les extre'mite's de la diagonale sont   */
/* (x1, y1) et (x2, y2)                                                   */

void effacerRectangle (int x1,int y1,int x2,int y2);

/*************************************************************************/

/* trace une ellipse de centre (x,y) et de rayons rx et ry  */

void tracerEllipse (int x,int y,int rx,int ry);

/*************************************************************************/

/* remplit une ellipse de centre (x,y) et de rayons rx et ry  */

void remplirEllipse (int x,int y,int rx,int ry);

/*************************************************************************/

/* efface l'ellipse de centre (x,y) et de rayons rx et ry */

void effacerEllipse (int x,int y,int rx,int ry);

/*************************************************************************/

/* e'criture sur l'e'cran graphique en superposition */

void ecrireSurImpression(int x,int y, char str[]);

/*************************************************************************/

/* e'criture sur l'e'cran graphique avec effacement */

void ecrireSurFond(int x,int y, char str[]);

/*************************************************************************/

void attendreClic(void);

/*************************************************************************/

Bool testerClic(void);

/*************************************************************************/

/* Attend une frappe clavier                                             */
void attendreClavier(void);

/***********************************************************************/

/* teste l'enfoncement d'une touche du clavier */

Bool testerClavier(void);

/***********************************************************************/

/* re-injecte le dernier evenement dans la file*/
void rappelEvenement(void);

/***********************************************************************/

/* teste le masquage de la fenetre*/

void testerFenetre(void);

/***********************************************************************/

/* fournit le code de la touche s'il y en a une, et son etat */

void toucheCode(KeySym *touche,int *etat);

/***********************************************************************/	

/* fournit le code et le nom de la touche s'il y en a une, et 0 et NULL sinon */

KeySym toucheCodeNom(char **str);

/***********************************************************************/	

/* gere deux touches enfoncees en meme temps, renvoie le nombre de touches actuellement enfoncees 
   et donne dans str et liste les noms et codes des touches avec NULL et 0 s'il n'y en a pas */

int toucheCodeNomBuff(char **str,KeySym **liste);

/***********************************************************************/	

/* fournit le caractère présent s'il y en a un, le caractère nul sinon */

char toucheCar(void);

/***********************************************************************/	

/* fournit le nom de la touche enfoncee s'il y en a une, et NULL sinon */

char *toucheNom(void);

/***********************************************************************/	

/* fournit le numéro de bouton qui a été enfoncé */

int boutonSouris(void);

/***********************************************************************/

/* attend un clic souris et fournit sa position */

void positionSouris(int *x,int *y);

/***********************************************************************/

void recupereSousImage(int x1,int y1,int x2,int y2);

/***********************************************************************/

void afficheSousImage(int x1,int y1);

/***********************************************************************/

void initPalette(unsigned char * palette, int nbcolors);

#endif

