//------------------------------------------------------//
// Bitmap.class.h
// Prototype de la classe Bitmap pour ouvrir un bitmap
//-----------------------------------------------------//
// Auteur : Natim
// Date de dernière modification : 18-04-2006
//-----------------------------------------------------//
#ifndef __BITMAP_H_
#define __BITMAP_H_
#include <iostream>
#include <cstdio>
#include <string>

using namespace std;

const short BITMAP_MAGIC_NUMBER=19778;
const int   RVB_BYTE_SIZE=3;

#pragma pack(push,bitmap_data,1)

typedef struct {
  char rvbBleu;
  char rvbVert;
  char rvbRouge;
  char rvbReserve;
} RVBCoul;

typedef struct {
  unsigned short bfType;
  unsigned int   bfSize;
  unsigned short bfReserved1;
  unsigned short bfReserved2;
  unsigned int   bfOffBits;
} BitmapFileHeader;

typedef struct {
  unsigned int   biSize;
  int            biWidth;
  int            biHeight;
  unsigned short biPlanes;
  unsigned short biBitCount;
  unsigned int   biCompression;
  unsigned int   biSizeImage;
  int            biXPelsPerMeter;
  int            biYPelsPerMeter;
  unsigned int   biClrUsed;
  unsigned int   biClrImportant;
} BitmapInfoHeader;

#pragma pack(pop,bitmap_data)

class Bitmap {
public:
    // Attributs
    string nom;

    int width,height,nbCouleurs;

    // Méthodes
    Bitmap(void);
    Bitmap(const char *);
    ~Bitmap();

    bool loadBMP(const char *);
    void affiche(int x, int y);
    void initPal();
    void remplirFond(int x1, int y1, int x2, int y2);
    static void invaliderZone(int x1, int y1, int x2, int y2);
    static void invaliderTout();

private:
    void dessinePixels(int x, int y, int larg, int haut);
    void enregistrer();
    void desenregistrer();
    static void invaliderPixels(int x1, int y1, int x2, int y2);

    // Attributs
    BitmapFileHeader bmfh;
    BitmapInfoHeader bmih;
    RVBCoul       * couleurs;
    char          * donnees;
    unsigned char * pal;
    unsigned int tailleDonnees;    // La taille des données bitmap
    unsigned short bpp;

    int byteWidth;                 // La taille en bytes de l'image
    int padWidth;                  // La taille en bytes de l'image modifiée
    int copieX, copieY;            // -1 when no valid master tile on screen
    int copieL, copieH;            // taille en pixels ecran de cette copie
    int indexBlanc, indexNoir;     // Indices palette du blanc et du noir

    static Bitmap * instances[64];
    static int nbInstances;
};

#endif //__BITMAP_H_
