//------------------------------------------------------//
// Bitmap.class.cpp
// Class Bitmap pour ouvrir un bitmap et l'afficher 
// Avec Graphlib
//-----------------------------------------------------//
// Auteur : Natim
// Date de derni�re modification : 18-04-2006
//-----------------------------------------------------//
#include "Bitmap.class.h"
#include <cmath>
#include <cstdlib>

extern "C"{
#include "lib/graphlib.h"
}

// Constructeur de base
Bitmap::Bitmap(){
  couleurs = NULL;
  donnees  = NULL;
  pal      = NULL;
}

// Constructeur permettant de charger un fichier
Bitmap::Bitmap(char *file){
  couleurs = NULL;
  donnees  = NULL;
  pal      = NULL;
  loadBMP(file);
}

// Destructeur de la classe
Bitmap::~Bitmap(){
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

// Charge un fichier bitmap d'un fichier dans la m�moire
bool Bitmap::loadBMP(char *file) {
  FILE * in = NULL;             // Descripteur de l'image � lire
  nom = file;            // Stockage du nom du fichier
  
  // On verifie qu'une image n'a pas d�j� �t� charg�e dans cette instance
  if(couleurs != NULL){
    delete[] couleurs;
  }
  if(donnees != NULL){
    delete[] donnees;
  }
  if(pal != NULL){
    delete[] pal;
  }
  
  // On ouvre le fichier en lecture binaire
  in = fopen(file, "rb");
  
  // Si la lecture n'a pas fonctionn�e, on returne un signal d'erreur
  if(in == NULL) {
    cerr << "Impossible d'ouvrir le fichier " << file << endl;
    fclose(in);
    return false;
  }
  
  // On lit l'integralit� de l'entete du fichier bitmap
  fread(&bmfh, sizeof(BitmapFileHeader), 1, in);
  
  // On verifie que le BitMap est bien au format DIB
  if(bmfh.bfType != BITMAP_MAGIC_NUMBER) {
    cerr << "Ce fichier n'est pas au format DIB";
    fclose(in);
    return false;
  }
  
  // On lit les informations d'ent�te
  fread(&bmih, sizeof(BitmapInfoHeader), 1, in);
  
  // On sauvegarde la largeur, hauteur et resolution du fichier bitmap
  width  = bmih.biWidth;
  height = bmih.biHeight;
  bpp    = bmih.biBitCount;

  cerr << nom << "\t:\t" << width << "x" << height << " - " << bpp << "bits" << endl;


  // On calcule la taille des donn�es avec la r�solution
  tailleDonnees = (width * height * (unsigned int) ceil(bpp/8.0));
  
  // On d�duit le nombre de couleurs
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
  fread(couleurs, sizeof(RVBCoul), nbCouleurs, in);

  // On cr�e la palette graphlib en RVB
  for(int i = 0; i < nbCouleurs; i++){
    pal[i*3]   = couleurs[i].rvbRouge;
    pal[i*3+1] = couleurs[i].rvbVert;
    pal[i*3+2] = couleurs[i].rvbBleu;
    //    pal[i*4+3] = couleurs[i].rvbReserve;
  }
  
  // On alloue un tableau pour charger l'image
  donnees = new char[tailleDonnees];
  
  // On verifie que l'allocation s'est bien pass�e
  if(donnees == NULL) {
    cerr << "Pas assez de m�moire pour charger le fichier" << endl;
    fclose(in);
    return false;
  }
  
  fseek(in, bmfh.bfOffBits, SEEK_SET);
  // On charge l'image
  fread(donnees, sizeof(char), tailleDonnees, in);
  
  // On ferme le fichier car on a fini
  fclose(in);
  // On calcule la taille finale de l'image en bits
  byteWidth = padWidth = (unsigned int)(width * ceil(bpp / 8.0));
  
  // Ajustage du padding si necessaire
  while(padWidth%4 != 0) {
    padWidth++;
  }
  
  // Tout c'est bien pass�
  return true;
}

void Bitmap::affiche(int x, int y) {
  initPal(); //Initialisation de la palette
  unsigned int i;

  for(i = 0; i < tailleDonnees; i++){
    modifierCouleur((unsigned int) donnees[i]%nbCouleurs);
    afficherPoint(x + i%padWidth, y + height-1 - i/padWidth);
  }
}

void Bitmap::initPal(){
  initPalette(pal, nbCouleurs); //Initialisation de la palette
}
