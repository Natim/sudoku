//------------------------------------------------------//
// Bitmap.class.cpp
// Class Bitmap pour ouvrir un bitmap et l'afficher 
// Avec Graphlib
//-----------------------------------------------------//
// Auteur : Natim
// Date de dernière modification : 18-04-2006
//-----------------------------------------------------//
#include "bitmap.h"
#include "window.h"
#include <cmath>
#include <cstdlib>
#include <cstdio>
#include <iostream>

extern "C"{
#include "graphlib.h"
}

using namespace std;

Bitmap * Bitmap::instances[64];
int Bitmap::nbInstances = 0;

// Constructeur de base
Bitmap::Bitmap(){
  couleurs = NULL;
  donnees  = NULL;
  pal      = NULL;
  width = height = 0;
  copieX = copieY = -1;
  copieL = copieH = 0;
  indexBlanc = indexNoir = 0;
  enregistrer();
}

// Constructeur permettant de charger un fichier
Bitmap::Bitmap(const char *file){
  couleurs = NULL;
  donnees  = NULL;
  pal      = NULL;
  copieX = copieY = -1;
  copieL = copieH = 0;
  indexBlanc = indexNoir = 0;
  loadBMP(file);
  enregistrer();
}

// Destructeur de la classe
Bitmap::~Bitmap(){
  desenregistrer();
  if(couleurs != NULL) {
    delete[] couleurs;
  }
  if(donnees  != NULL) {
    delete[] donnees;
  }
  if(pal  != NULL){
    delete[] pal;
  }
}

void Bitmap::enregistrer(){
  if(nbInstances < 64)
    instances[nbInstances++] = this;
}

void Bitmap::desenregistrer(){
  for(int i = 0; i < nbInstances; i++){
    if(instances[i] == this){
      instances[i] = instances[--nbInstances];
      return;
    }
  }
}

// Les copies de reference sont reperees en pixels ecran, pas en coordonnees
// logiques : leur taille depend de l'echelle en cours.
void Bitmap::invaliderPixels(int x1, int y1, int x2, int y2){
  for(int i = 0; i < nbInstances; i++){
    Bitmap * bmp = instances[i];
    if(bmp->copieX < 0)
      continue;
    if(bmp->copieX + bmp->copieL > x1 && bmp->copieX < x2 &&
       bmp->copieY + bmp->copieH > y1 && bmp->copieY < y2)
      bmp->copieX = bmp->copieY = -1;
  }
}

void Bitmap::invaliderZone(int x1, int y1, int x2, int y2){
  invaliderPixels(pixX(x1), pixY(y1), pixX(x2), pixY(y2));
}

void Bitmap::invaliderTout(){
  for(int i = 0; i < nbInstances; i++){
    instances[i]->copieX = -1;
    instances[i]->copieY = -1;
  }
}

// Charge un fichier bitmap d'un fichier dans la mémoire
bool Bitmap::loadBMP(const char *file) {
  FILE * in = NULL;             // Descripteur de l'image à lire
  nom = file;            // Stockage du nom du fichier
  
  // On verifie qu'une image n'a pas déjà été chargée dans cette instance
  delete[] couleurs;
  couleurs = NULL;
  delete[] donnees;
  donnees  = NULL;
  delete[] pal;
  pal      = NULL;
  
  // On ouvre le fichier en lecture binaire
  in = fopen(file, "rb");
  
  // Si la lecture n'a pas fonctionnée, on returne un signal d'erreur
  if(in == NULL) {
    cerr << "Impossible d'ouvrir le fichier " << file << endl;
    return false;
  }
  
  // On lit l'integralité de l'entete du fichier bitmap
  if(fread(&bmfh, sizeof(BitmapFileHeader), 1, in) != 1) {
    cerr << "Entete de fichier illisible dans " << file << endl;
    fclose(in);
    return false;
  }
  
  // On verifie que le BitMap est bien au format DIB
  if(bmfh.bfType != BITMAP_MAGIC_NUMBER) {
    cerr << "Ce fichier n'est pas au format DIB";
    fclose(in);
    return false;
  }
  
  // On lit les informations d'entête
  if(fread(&bmih, sizeof(BitmapInfoHeader), 1, in) != 1) {
    cerr << "Entete bitmap illisible dans " << file << endl;
    fclose(in);
    return false;
  }
  
  // On sauvegarde la largeur, hauteur et resolution du fichier bitmap
  width  = bmih.biWidth;
  height = bmih.biHeight;
  bpp    = bmih.biBitCount;

  cerr << nom << "\t:\t" << width << "x" << height << " - " << bpp << "bits" << endl;


  // On calcule la taille des données avec la résolution
  tailleDonnees = (width * height * (unsigned int) ceil(bpp/8.0));
  
  // On déduit le nombre de couleurs
  nbCouleurs = (bmih.biClrUsed != 0) ? bmih.biClrUsed : 256;

  // Si le fichier n'est pas en 8 bits par pixel, on ne sait pas le lire
  if(bpp > 8) {
    cerr << "On ne sait lire que les fichiers bitmap d'au plus 8 bits" << endl;
    fclose(in);
    exit(1);
    return false;
  }
  // Chargement de la palette
  pal = new unsigned char[nbCouleurs*3];
  couleurs = new RVBCoul[nbCouleurs];

  // Le format est en BVR. On lit chaque couleurs
  if(fread(couleurs, sizeof(RVBCoul), nbCouleurs, in) != (size_t) nbCouleurs) {
    cerr << "Palette illisible dans " << file << endl;
    fclose(in);
    return false;
  }

  // On crée la palette graphlib en RVB
  int sommeMax = -1, sommeMin = 3 * 255 + 1;
  indexBlanc = indexNoir = 0;
  for(int i = 0; i < nbCouleurs; i++){
    pal[i*3]   = couleurs[i].rvbRouge;
    pal[i*3+1] = couleurs[i].rvbVert;
    pal[i*3+2] = couleurs[i].rvbBleu;
    //    pal[i*4+3] = couleurs[i].rvbReserve;

    int somme = pal[i*3] + pal[i*3+1] + pal[i*3+2];
    if(somme > sommeMax){
      sommeMax = somme;
      indexBlanc = i;
    }
    if(somme < sommeMin){
      sommeMin = somme;
      indexNoir = i;
    }
  }
  
  // On alloue un tableau pour charger l'image
  donnees = new char[tailleDonnees];
  
  // On verifie que l'allocation s'est bien passée
  if(donnees == NULL) {
    cerr << "Pas assez de mémoire pour charger le fichier" << endl;
    fclose(in);
    return false;
  }
  
  fseek(in, bmfh.bfOffBits, SEEK_SET);
  // On charge l'image
  if(fread(donnees, sizeof(char), tailleDonnees, in) != tailleDonnees) {
    cerr << "Donnees d'image illisibles dans " << file << endl;
    fclose(in);
    return false;
  }
  
  // On ferme le fichier car on a fini
  fclose(in);
  // On calcule la taille finale de l'image en bits
  byteWidth = padWidth = (unsigned int)(width * ceil(bpp / 8.0));
  
  // Ajustage du padding si necessaire
  while(padWidth%4 != 0) {
    padWidth++;
  }
  
  // Tout c'est bien passé
  return true;
}

/* Dessine l'image, en pixels ecran, dans le rectangle larg x haut : chaque
   pixel du fichier devient un bloc, ce qui met l'image a l'echelle de la
   fenetre. Les bornes sont calculees de proche en proche pour que les blocs
   pavent exactement le rectangle, sans trou ni recouvrement. */
void Bitmap::dessinePixels(int x, int y, int larg, int haut){
  initPal();
  for(int row = 0; row < height; row++){
    // La premiere ligne du fichier bitmap est celle du bas de l'image.
    int y1 = y + (height - 1 - row) * haut / height;
    int y2 = y + (height - row) * haut / height;
    if(y2 <= y1)
      continue;

    int col = 0;
    while(col < width){
      unsigned char idx = (unsigned char) donnees[row * padWidth + col];
      idx = idx % nbCouleurs;
      int runStart = col;
      while(col + 1 < width &&
            (unsigned char) donnees[row * padWidth + col + 1] % nbCouleurs == idx)
        col++;

      int x1 = x + runStart * larg / width;
      int x2 = x + (col + 1) * larg / width;
      if(x2 > x1){
        modifierCouleur(idx);
        remplirRectangle(x1, y1, x2, y2);
      }
      col++;
    }
  }
}

void Bitmap::affiche(int x, int y) {
  int px = pixX(x), py = pixY(y);
  int larg = pixX(x + width) - px;
  int haut = pixY(y + height) - py;

  // Apres un changement d'echelle la copie de reference n'est plus a la bonne
  // taille : il faut redessiner l'image pixel par pixel.
  bool tuile = (copieX >= 0 && copieL == larg && copieH == haut);
  if(tuile)
    recupereSousImage(copieX, copieY, copieX + copieL, copieY + copieH);

  invaliderPixels(px, py, px + larg, py + haut);

  if(tuile)
    afficheSousImage(px, py);
  else
    dessinePixels(px, py, larg, haut);

  copieX = px;
  copieY = py;
  copieL = larg;
  copieH = haut;
}

void Bitmap::initPal(){
  initPalette(pal, nbCouleurs); //Initialisation de la palette
}

/* remplirRectangle peint la fenetre et le double buffer avec la couleur
   d'avant-plan courante : on la force au blanc de la palette, puis on
   restaure le noir pour ne pas perturber les traces suivants.

   Le remplissage s'arrete un pixel avant x2 alors que tracerRectangle dessine
   son contour sur x2 : on ajoute ce pixel pour effacer aussi les contours. Le
   calcul est fait en pixels ecran, une unite logique pouvant valoir moins d'un
   pixel quand la fenetre est reduite. */
void Bitmap::remplirFond(int x1, int y1, int x2, int y2){
  initPal();
  modifierCouleur(indexBlanc);
  remplirRectangle(pixX(x1), pixY(y1), pixX(x2) + 1, pixY(y2) + 1);
  modifierCouleur(indexNoir);
}
