#pragma once

#include <string>

const short BITMAP_MAGIC_NUMBER = 19778;
const int   RGB_BYTE_SIZE = 3;

#pragma pack(push, bitmap_data, 1)

typedef struct {
  char blue;
  char green;
  char red;
  char reserved;
} RgbColor;

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
    std::string name;

    int width, height, colorCount;

    Bitmap();
    Bitmap(const char *);
    ~Bitmap();

    bool loadBMP(const char *);
    void draw(int x, int y);
    void applyPalette();
    void fillBackground(int x1, int y1, int x2, int y2);
    static void invalidateArea(int x1, int y1, int x2, int y2);
    static void invalidateAll();

private:
    void drawPixels(int x, int y, int width, int height);
    void registerInstance();
    void unregisterInstance();
    static void invalidatePixels(int x1, int y1, int x2, int y2);

    BitmapFileHeader bmfh;
    BitmapInfoHeader bmih;
    RgbColor       * colors;
    char           * data;
    unsigned char  * palette;
    unsigned int dataSize;
    unsigned short bpp;

    int byteWidth;
    int padWidth;
    int cacheX, cacheY;
    int cacheWidth, cacheHeight;
    int whiteIndex, blackIndex;

    static Bitmap * instances[64];
    static int instanceCount;
};
