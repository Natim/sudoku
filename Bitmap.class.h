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
    char * nom;

    int width,height,nbCouleurs;

    // Méthodes
    Bitmap(void);
    Bitmap(char *);
    ~Bitmap();

    bool loadBMP(char *);
    void affiche(int x, int y);
    void initPal();

private:
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
};

#endif //__BITMAP_H_
