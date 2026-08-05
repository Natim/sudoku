==============
Sudoku Solveur
==============

Encore un projet longtemps sans generateur complet. Voici quand même les
grandes lignes de ce projet.

L'interface a été beaucoup travaillée, avec la création d'une classe
(``Bitmap.class.*``) pour lire les fichiers bitmaps 8 bits. Une erreur de
graphlib affiche un message à chaque fois que l'on change de palette de
couleurs, c'est à dire à chaque affichage d'une image. Pour la corriger,
utiliser le fichier ``launch.sh``.

L'interface fonctionne aussi avec 3 fonctions pour afficher et gérer les
boites de dialogues.

Un ensemble de fonctions permet de gérer les sudokus, de les résoudre et
d'en generer de nouvelles. On peut enregistrer un sudoku dans un fichier et
ouvrir la grille par la suite. Mais aussi le résoudre, et placer les chiffres
en vérifiant s'ils sont bien à la bonne place.

Enfin, le programme en lui même gère l'interface entre l'utilisateur et le
sudoku.

Ce sudoku a été un travail très intéressant à réaliser sur beaucoup de plans
(lecture de bitmaps, recherche méthodique de solutions, boites de dialogue,
etc.).

1/ La lecture d'un bitmap
=========================

L'idée n'est pas de moi, mais de Germain Desvigne qui m'a proposé un
challenge : afficher des images dans Graphlib. Et on a chacun réalisé pour le
Sudoku une classe permettant de charger des bitmaps. Moi, uniquement des
8 bits, et lui, tous les bitmaps d'au plus 8 bits.

Il faut savoir que l'on peut décomposer un bitmap en 4 parties :

- **Le header du fichier** : contient ``BM`` au début, et l'offset sur les
  données.
- **Le header du bitmap** : contient le nombre de couleurs, la largeur et la
  hauteur.
- **La palette du BMP** : elle contient toutes les couleurs en BGR (et pas
  RGB).
- **Les pixels** : ils contiennent un caractère pointant sur une couleur de la
  palette.

2/ Le Sudoku
============

Le Sudoku étant très en vogue, il y avait beaucoup de solutions sur internet,
de méthodes plus ou moins farfelues. Mais la plupart essayaient de faire
réfléchir un ordinateur comme un être humain :

- recherche des candidats, suppression des solutions impossibles, etc.

J'ai décidé de trouver une méthode beaucoup plus simple, qui repose sur la
puissance de la machine, en lui faisant parcourir les cases jusqu'à ce
qu'elles soient pleines. Ou que l'on sorte du tableau en 0,0, ce qui
signifierait qu'il n'y a pas de solution.

Ainsi, ce solveur peut résoudre toutes les grilles solvables.

2.1/ La méthode
---------------

On commence à la case 0,0, et on incrémente les cases non fixes tant que l'on
ne peut pas fixer le chiffre dessus. Si l'on incrémente un 9, on met 0 dans la
case et on recule d'une case. Et ce, tant que le sudoku n'est pas résolu.

Si l'on sort du sudoku en 0,0, c'est que le sudoku n'a pas de solution.

2.2/ Le generateur
------------------

Le bouton « Nouveau » propose des grilles jouables en trois niveaux, plus une
grille vierge. Chaque grille generee respecte les regles du jeu et n'admet
qu'une seule solution.

La methode part d'une grille complete tiree au hasard, puis creuse des cases
deux a deux en symetrie centrale. Un trou n'est conserve que si la grille
incomplete garde exactement une solution ; sinon les chiffres sont remis en
place. Les niveaux visent environ 45 indices (facile), 34 (moyen) et 28
(difficile).

Le comptage de solutions et le remplissage aleatoire passent par un solveur
dedie a masques de bits, plus rapide que ``resolve()`` pour enchainer les
verifications. Les tirages aleatoires sont centralises dans ``alea.h``
(``hasard``, ``melanger``, ``moteurAleatoire``).

Les indices generes sont marques ``donnee`` : ils s'affichent comme des
indices fixes et ne peuvent pas etre effaces par le joueur.

3/ Les boites de dialogues
==========================

Elles rajoutent un plus au programme. ``alert()`` retourne ``true`` si l'on
clique sur « oui » et ``false`` dans le cas contraire. ``choisir()`` affiche
plusieurs boutons et renvoie l'indice clique. Ceci permet de gérer simplement
l'interface utilisateur, y compris le choix de la difficulte d'une nouvelle
grille.

Sur ce sudoku, vous pouvez generer une grille, en ouvrir une enregistree,
enregistrer une grille, mettre les chiffres dans les cases, resoudre la
grille et quitter.

----

Si vous en avez l'occasion, je ne peux que vous suggérer de regarder les
Sudokus de Germain Desvignes et Rémy Burney, qui ont fait tous deux un travail
excellent sur ce projet.
