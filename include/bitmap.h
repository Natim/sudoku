#pragma once

#include <string>

const short BITMAP_MAGIC_NUMBER = 19778;
const int   RVB_BYTE_SIZE = 3;

#pragma pack(push, bitmap_data, 1)

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

#pragma pack(pop, bitmap_data)

class Bitmap {
public:
    std::string nom;

    int width, height, nbCouleurs;

    Bitmap();
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

    BitmapFileHeader bmfh;
    BitmapInfoHeader bmih;
    RVBCoul       * couleurs;
    char          * donnees;
    unsigned char * pal;
    unsigned int tailleDonnees;
    unsigned short bpp;

    int byteWidth;
    int padWidth;
    int copieX, copieY;
    int copieL, copieH;
    int indexBlanc, indexNoir;

    static Bitmap * instances[64];
    static int nbInstances;
};
