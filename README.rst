Sudoku Solveur :
----------------

Encore un projet sans algorithme complet.
Voici quand m�me les grandes lignes de ce projet :

L'interface a �t�, beaucoup travaill� avec la cr�ation
d'une classe (Bitmap.class.*) pour lire les fichiers
bitmaps 8bits. Une erreur de graphlib affiche un message
� chaque fois que l'on change de palette de couleurs.
C'est � dire � chaque affichage d'une image.
Pour la corriger, utiliser le fichier launch.sh

L'interface fonctionne aussi avec 3 fonctions pour afficher
et gerer les boites de dialogues.

Un ensemble de fonction permet de gerer les sudokus, et de 
les r�soudres. On peut enregistrer un sudoku dans un fichier
et ouvrir la grille par la suite. Mais aussi, le r�soudre
et placer les chiffres en v�rifiant s'ils sont bien � la bonne
place.

Enfin, le programme en lui m�me gere l'interface entre l'utilisateur
et le sudoku.

Ce sudoku a �t� un travail tr�s interessant � r�aliser sur beaucoup
de plans. (Lecture bitmaps, recherche m�thodique de solutions, boites de dialogue, ...)

1/ La lecture d'un bitmap :
---------------------------
L'id�e n'est pas de moi, mais de Germain Desvigne qui m'a propos� un challenge:
 Afficher des images dans Graphlib. Et on a chacun r�alis� pour le Sudoku
une classe permettant de charger des bitmaps. Moi, uniquement des 8bits et lui, tous les bitmaps d'au plus 8 bits.

Il faut s'avoir que l'on peut d�composer un bitmap en 4 parties :
 - Le Header du fichier : Contenant BM au d�but et l'offset sur les donn�es
 - Le Header du Bitmaps : Contenant le nombre de couleur, la largeur, la hauteur
 - La palette du BMP    : Elle contient toutes les couleurs en BGR (Et pas RGB)
 - Les pixels qui contiennent un charact�re pointant sur une couleur de la palette.

2/ Le Sudoku
------------
Le Sudoku �tant tr�s en vogue, il y avait beaucoup de solution sur internet
de m�thodes plus ou moins farfelue. Mais la plupart, essay� de faire r�fl�chir
 un ordinateur comme une �tre humain :
  - Recherche des candidats, suppression des solutions impossibles ...

J'ai d�cid�, de trouver une m�thode beaucoup plus simple qui repose sur la 
puissance de la machine. En lui faisant parcourir les cases jusqu'� ce qu'elles 
soient pleines. Ou que l'on sorte du tableau en 0,0 ce qui signifierait qu'il 
n'y ai pas de solutions.

Ainsi, ce solveur peut r�soudre toutes les grilles solvables.

2.1/ La m�thode
---------------
On commence � la case 0,0, et on incr�mente les cases non fixe tant que l'on ne
peut pas fixer le chiffre dessus. Si l'on incr�mente un 9, on mets 0 dans la case
Et on recule d'une case. Et ce tant que le sudoku n'est pas r�solu.
Si l'on sort du sudoku en 0,0. C'est que le sudoku n'a pas de solution.

3/ Les boites de dialogues
--------------------------
Elles rajoutent un plus au programme et retourne juste true si l'on clique 
sur 'oui' et false dans le cas contraire. Ceci est relativement simple et 
permet de gerer simplement l'interface utilisateur.


Sur ce sudoku, vous pouvez cr�er une grille vierge, ouvrir un sudoku enregistr�e,
enregistrer une grille, mettre les chiffres dans les cases, solver la grille et quitter

----------------------------------------------------------------------------------------
Si vous avez l'occasion, je ne peux que vous suggerer de regarder les Sudokus de 
Germain Desvignes et R�my Burney, qui ont fait tout deux un travail excellent sur ce projet.