extern "C"{
#include "lib/graphlib.h"
}
#include <iostream>
#include <string>
#include <unistd.h>

using namespace std;

bool alert(int x, int y, string message);
/* 
   Affiche une boite de dialogue contenant le message 
   et proposant deux boutons oui et non 

   Retoure true si on clique sur oui et false si non !
*/

bool alert(int x, int y, string message, string titre);
/*
  On peut mettre un titre a la boite de dialogue
*/

bool alert(int x, int y, int width, int height, string message, string titre);
/*
  On peut mettre un titre a la boite de dialogue et specifier une taille
*/

void definirRepeindreFond(void (*peindre)());
/*
  Indique comment repeindre ce que la boite recouvre, pour que celle-ci puisse
  se reafficher apres un redimensionnement de la fenetre.
*/
