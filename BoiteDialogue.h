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
  On peut mettre un titre à la boite de dialogue
*/

bool alert(int x, int y, int width, int height, string message, string titre);
/*
  On peut mettre un titre à la boite de dialogue et spécifier une taille
*/
