Sudoku Solveur :
----------------

Encore un projet sans algorithme complet.
Voici quand même les grandes lignes de ce projet :

L'interface a été, beaucoup travaillé avec la création
d'une classe (Bitmap.class.*) pour lire les fichiers
bitmaps 8bits. Une erreur de graphlib affiche un message
à chaque fois que l'on change de palette de couleurs.
C'est à dire à chaque affichage d'une image.
Pour la corriger, utiliser le fichier launch.sh

L'interface fonctionne aussi avec 3 fonctions pour afficher
et gerer les boites de dialogues.

Un ensemble de fonction permet de gerer les sudokus, et de 
les résoudres. On peut enregistrer un sudoku dans un fichier
et ouvrir la grille par la suite. Mais aussi, le résoudre
et placer les chiffres en vérifiant s'ils sont bien à la bonne
place.

Enfin, le programme en lui même gere l'interface entre l'utilisateur
et le sudoku.

Ce sudoku a été un travail très interessant à réaliser sur beaucoup
de plans. (Lecture bitmaps, recherche méthodique de solutions, boites de dialogue, ...)

1/ La lecture d'un bitmap :
---------------------------
L'idée n'est pas de moi, mais de Germain Desvigne qui m'a proposé un challenge:
 Afficher des images dans Graphlib. Et on a chacun réalisé pour le Sudoku
une classe permettant de charger des bitmaps. Moi, uniquement des 8bits et lui, tous les bitmaps d'au plus 8 bits.

Il faut s'avoir que l'on peut décomposer un bitmap en 4 parties :
 - Le Header du fichier : Contenant BM au début et l'offset sur les données
 - Le Header du Bitmaps : Contenant le nombre de couleur, la largeur, la hauteur
 - La palette du BMP    : Elle contient toutes les couleurs en BGR (Et pas RGB)
 - Les pixels qui contiennent un charactère pointant sur une couleur de la palette.

2/ Le Sudoku
------------
Le Sudoku étant très en vogue, il y avait beaucoup de solution sur internet
de méthodes plus ou moins farfelue. Mais la plupart, essayé de faire réfléchir
 un ordinateur comme une être humain :
  - Recherche des candidats, suppression des solutions impossibles ...

J'ai décidé, de trouver une méthode beaucoup plus simple qui repose sur la 
puissance de la machine. En lui faisant parcourir les cases jusqu'à ce qu'elles 
soient pleines. Ou que l'on sorte du tableau en 0,0 ce qui signifierait qu'il 
n'y ai pas de solutions.

Ainsi, ce solveur peut résoudre toutes les grilles solvables.

2.1/ La méthode
---------------
On commence à la case 0,0, et on incrémente les cases non fixe tant que l'on ne
peut pas fixer le chiffre dessus. Si l'on incrémente un 9, on mets 0 dans la case
Et on recule d'une case. Et ce tant que le sudoku n'est pas résolu.
Si l'on sort du sudoku en 0,0. C'est que le sudoku n'a pas de solution.

3/ Les boites de dialogues
--------------------------
Elles rajoutent un plus au programme et retourne juste true si l'on clique 
sur 'oui' et false dans le cas contraire. Ceci est relativement simple et 
permet de gerer simplement l'interface utilisateur.


Sur ce sudoku, vous pouvez créer une grille vierge, ouvrir un sudoku enregistrée,
enregistrer une grille, mettre les chiffres dans les cases, solver la grille et quitter

----------------------------------------------------------------------------------------
Si vous avez l'occasion, je ne peux que vous suggerer de regarder les Sudokus de 
Germain Desvignes et Rémy Burney, qui ont fait tout deux un travail excellent sur ce projet.