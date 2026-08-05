#pragma once

#include <cstdio>
#include <string>

const unsigned short BITMAP_MAGIC_NUMBER = 19778;
const int            RGB_BYTE_SIZE = 3;

// On-disk header sizes. The info header comes in several versions: 12 bytes for
// the OS/2 core header, 40 for BITMAPINFOHEADER, and 52, 56, 108 or 124 for the
// later versions, which extend it without moving the first fields.
const unsigned int BITMAP_FILE_HEADER_SIZE = 14;
const unsigned int BITMAP_CORE_HEADER_SIZE = 12;
const unsigned int BITMAP_INFO_HEADER_SIZE = 40;

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

class Bitmap {
public:
    std::string name;

    int width, height, colorCount;

    Bitmap();
    Bitmap(const char *);
    ~Bitmap();

    bool loadBMP(const char *);
    bool isLoaded() const;
    // Palette index of one pixel, (0, 0) being the top left corner. Returns -1
    // outside the image.
    int indexAt(int x, int y) const;
    void draw(int x, int y);
    void applyPalette();
    void fillBackground(int x1, int y1, int x2, int y2);
    static void invalidateArea(int x1, int y1, int x2, int y2);
    static void invalidateAll();

private:
    void reset();
    bool decode(std::FILE *);
    bool readHeaders(std::FILE *, unsigned int * paletteEntrySize);
    bool readPalette(std::FILE *, unsigned int paletteEntrySize);
    bool readPixels(std::FILE *, bool topDown);
    bool readRlePixels(std::FILE *);
    void unpackRow(const unsigned char * row, unsigned char * pixels) const;
    void putPixel(int x, int row, unsigned char index);
    void clampIndices();
    void drawPixels(int x, int y, int width, int height);
    void registerInstance();
    void unregisterInstance();
    static void invalidatePixels(int x1, int y1, int x2, int y2);

    BitmapFileHeader bmfh;
    BitmapInfoHeader bmih;
    // One palette index per pixel, top row first, whatever the depth and the
    // row order of the file: the drawing code needs no knowledge of the format.
    unsigned char  * data;
    unsigned char  * palette;
    unsigned short bpp;

    int cacheX, cacheY;
    int cacheWidth, cacheHeight;
    int whiteIndex, blackIndex;

    static Bitmap * instances[64];
    static int instanceCount;
};
