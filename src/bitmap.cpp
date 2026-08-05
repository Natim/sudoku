//------------------------------------------------------//
// bitmap.cpp
// Bitmap class to load and display palette BMP files with Graphlib
//-----------------------------------------------------//
// Author: Natim
// Last modified: 18-04-2006
//-----------------------------------------------------//
#include "bitmap.h"
#include "window.h"
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <new>

extern "C"{
#include "graphlib.h"
}

using namespace std;

namespace {

const unsigned int BI_RGB  = 0;
const unsigned int BI_RLE8 = 1;
const unsigned int BI_RLE4 = 2;

// Escape codes read in place of a run length of 0.
const int RLE_END_OF_LINE   = 0;
const int RLE_END_OF_BITMAP = 1;
const int RLE_DELTA         = 2;

// graphlib stores the palette in a fixed table of 256 X colors, and a single
// byte per pixel could not index more than that anyway.
const int MAX_COLORS = 256;

// The drawing path paints one rectangle per run of identical pixels, so a huge
// image would be unusable long before it fit on screen: bound the dimensions
// rather than trust what the header claims.
const int MAX_DIMENSION = 8192;

unsigned short readU16(const unsigned char * p){
  return (unsigned short)(p[0] | (p[1] << 8));
}

unsigned int readU32(const unsigned char * p){
  return (unsigned int) p[0] | ((unsigned int) p[1] << 8) |
         ((unsigned int) p[2] << 16) | ((unsigned int) p[3] << 24);
}

int readS32(const unsigned char * p){
  return (int) readU32(p);
}

bool readExact(FILE * in, unsigned char * dst, size_t size){
  return fread(dst, 1, size, in) == size;
}

}

Bitmap * Bitmap::instances[64];
int Bitmap::instanceCount = 0;

Bitmap::Bitmap(){
  data = NULL;
  palette = NULL;
  reset();
  registerInstance();
}

Bitmap::Bitmap(const char *file){
  data = NULL;
  palette = NULL;
  reset();
  loadBMP(file);
  registerInstance();
}

Bitmap::~Bitmap(){
  unregisterInstance();
  delete[] data;
  delete[] palette;
}

// Back to the state of a freshly built, empty bitmap. Called before a load and
// after a failed one so a caller that ignores the return value draws nothing
// instead of reading a half-filled image.
void Bitmap::reset(){
  delete[] data;
  data = NULL;
  delete[] palette;
  palette = NULL;
  memset(&bmfh, 0, sizeof bmfh);
  memset(&bmih, 0, sizeof bmih);
  width = height = 0;
  colorCount = 0;
  bpp = 0;
  cacheX = cacheY = -1;
  cacheWidth = cacheHeight = 0;
  whiteIndex = blackIndex = 0;
}

void Bitmap::registerInstance(){
  if(instanceCount < 64)
    instances[instanceCount++] = this;
}

void Bitmap::unregisterInstance(){
  for(int i = 0; i < instanceCount; i++){
    if(instances[i] == this){
      instances[i] = instances[--instanceCount];
      return;
    }
  }
}

// Reference copies are tracked in screen pixels, not logical coordinates:
// their size depends on the current scale factor.
void Bitmap::invalidatePixels(int x1, int y1, int x2, int y2){
  for(int i = 0; i < instanceCount; i++){
    Bitmap * bmp = instances[i];
    if(bmp->cacheX < 0)
      continue;
    if(bmp->cacheX + bmp->cacheWidth > x1 && bmp->cacheX < x2 &&
       bmp->cacheY + bmp->cacheHeight > y1 && bmp->cacheY < y2)
      bmp->cacheX = bmp->cacheY = -1;
  }
}

void Bitmap::invalidateArea(int x1, int y1, int x2, int y2){
  invalidatePixels(pixX(x1), pixY(y1), pixX(x2), pixY(y2));
}

void Bitmap::invalidateAll(){
  for(int i = 0; i < instanceCount; i++){
    instances[i]->cacheX = -1;
    instances[i]->cacheY = -1;
  }
}

bool Bitmap::isLoaded() const {
  return data != NULL;
}

int Bitmap::indexAt(int x, int y) const {
  if(data == NULL || x < 0 || y < 0 || x >= width || y >= height)
    return -1;
  return data[(size_t) y * width + x];
}

bool Bitmap::loadBMP(const char *file) {
  reset();
  name = file;

  FILE * in = fopen(file, "rb");

  if(in == NULL) {
    cerr << "Cannot open file " << file << endl;
    return false;
  }

  bool ok = decode(in);
  fclose(in);

  if(!ok)
    reset();

  return ok;
}

bool Bitmap::decode(FILE * in){
  unsigned int paletteEntrySize = 0;

  if(!readHeaders(in, &paletteEntrySize))
    return false;

  if(!readPalette(in, paletteEntrySize))
    return false;

  data = new (nothrow) unsigned char[(size_t) width * height];

  if(data == NULL) {
    cerr << "Not enough memory to load the file " << name << endl;
    return false;
  }

  // Pixels the file does not cover stay on the first palette entry: an RLE
  // stream may legally skip over them, and a truncated file must not show
  // uninitialised memory.
  memset(data, 0, (size_t) width * height);

  if(bmfh.bfOffBits < BITMAP_FILE_HEADER_SIZE + bmih.biSize ||
     fseek(in, (long) bmfh.bfOffBits, SEEK_SET) != 0) {
    cerr << "Bad image data offset in " << name << endl;
    return false;
  }

  bool rle = (bmih.biCompression == BI_RLE8 || bmih.biCompression == BI_RLE4);
  bool ok = rle ? readRlePixels(in) : readPixels(in, bmih.biHeight < 0);

  if(ok)
    clampIndices();

  return ok;
}

bool Bitmap::readHeaders(FILE * in, unsigned int * paletteEntrySize){
  unsigned char fileHeader[BITMAP_FILE_HEADER_SIZE];

  if(!readExact(in, fileHeader, sizeof fileHeader)) {
    cerr << "Unreadable file header in " << name << endl;
    return false;
  }

  bmfh.bfType      = readU16(fileHeader);
  bmfh.bfSize      = readU32(fileHeader + 2);
  bmfh.bfReserved1 = readU16(fileHeader + 6);
  bmfh.bfReserved2 = readU16(fileHeader + 8);
  bmfh.bfOffBits   = readU32(fileHeader + 10);

  if(bmfh.bfType != BITMAP_MAGIC_NUMBER) {
    cerr << name << " is not in DIB format" << endl;
    return false;
  }

  unsigned char sizeField[4];

  if(!readExact(in, sizeField, sizeof sizeField)) {
    cerr << "Unreadable bitmap header in " << name << endl;
    return false;
  }

  const unsigned int headerSize = readU32(sizeField);

  if(headerSize == BITMAP_CORE_HEADER_SIZE) {
    // OS/2 core header: unsigned 16-bit dimensions, no compression and no color
    // count, and its palette entries hold no reserved byte.
    unsigned char core[BITMAP_CORE_HEADER_SIZE - 4];

    if(!readExact(in, core, sizeof core)) {
      cerr << "Unreadable bitmap header in " << name << endl;
      return false;
    }

    bmih.biWidth       = readU16(core);
    bmih.biHeight      = readU16(core + 2);
    bmih.biPlanes      = readU16(core + 4);
    bmih.biBitCount    = readU16(core + 6);
    bmih.biCompression = BI_RGB;
    bmih.biClrUsed     = 0;
    *paletteEntrySize  = RGB_BYTE_SIZE;
  } else if(headerSize >= BITMAP_INFO_HEADER_SIZE) {
    // BITMAPINFOHEADER, and the V2 to V5 versions that extend it: the extra
    // fields describe channel masks and color spaces, which only apply above
    // 8 bits per pixel, so reading the common part is enough.
    unsigned char info[BITMAP_INFO_HEADER_SIZE - 4];

    if(!readExact(in, info, sizeof info)) {
      cerr << "Unreadable bitmap header in " << name << endl;
      return false;
    }

    bmih.biWidth         = readS32(info);
    bmih.biHeight        = readS32(info + 4);
    bmih.biPlanes        = readU16(info + 8);
    bmih.biBitCount      = readU16(info + 10);
    bmih.biCompression   = readU32(info + 12);
    bmih.biSizeImage     = readU32(info + 16);
    bmih.biXPelsPerMeter = readS32(info + 20);
    bmih.biYPelsPerMeter = readS32(info + 24);
    bmih.biClrUsed       = readU32(info + 28);
    bmih.biClrImportant  = readU32(info + 32);
    *paletteEntrySize = 4;
  } else {
    cerr << "Unsupported bitmap header size " << headerSize << " in " << name << endl;
    return false;
  }

  bmih.biSize = headerSize;

  // A negative height means the rows are stored top down. Take the absolute
  // value through a wider type, INT_MIN having no positive counterpart.
  long long rows = bmih.biHeight;

  if(rows < 0)
    rows = -rows;

  if(bmih.biWidth <= 0 || bmih.biWidth > MAX_DIMENSION ||
     rows <= 0 || rows > MAX_DIMENSION) {
    cerr << "Unsupported bitmap size " << bmih.biWidth << "x" << bmih.biHeight
	 << " in " << name << endl;
    return false;
  }

  width  = bmih.biWidth;
  height = (int) rows;
  bpp    = bmih.biBitCount;

  if(bmih.biPlanes != 1) {
    cerr << "Unsupported plane count " << bmih.biPlanes << " in " << name << endl;
    return false;
  }

  // Above 8 bits a pixel carries its own color instead of a palette index,
  // which the palette based drawing path cannot represent.
  if(bpp != 1 && bpp != 2 && bpp != 4 && bpp != 8) {
    cerr << "Only palette bitmaps of at most 8 bits are supported, " << name
	 << " has " << bpp << endl;
    return false;
  }

  if(bmih.biCompression == BI_RLE8 || bmih.biCompression == BI_RLE4) {
    unsigned int expected = (bmih.biCompression == BI_RLE8) ? 8 : 4;

    if(bpp != expected) {
      cerr << "RLE" << expected << " compression needs " << expected
	   << " bits per pixel in " << name << endl;
      return false;
    }

    // A run length encoded image is scanned from its bottom row up, so the top
    // down flag has no meaning here.
    if(bmih.biHeight < 0) {
      cerr << "Compressed bitmaps cannot be stored top down in " << name << endl;
      return false;
    }
  } else if(bmih.biCompression != BI_RGB) {
    cerr << "Unsupported compression " << bmih.biCompression << " in " << name << endl;
    return false;
  }

  return true;
}

bool Bitmap::readPalette(FILE * in, unsigned int paletteEntrySize){
  const unsigned int paletteOffset = BITMAP_FILE_HEADER_SIZE + bmih.biSize;

  // An indexed image cannot hold more entries than its depth can address, and
  // a file is free to store fewer than that.
  unsigned int count = (1u << bpp);

  if(bmih.biClrUsed != 0 && bmih.biClrUsed < count)
    count = bmih.biClrUsed;

  // The image data marks the end of the palette: without this the default count
  // would read pixels as colors when the file stores a shorter table.
  if(bmfh.bfOffBits > paletteOffset) {
    unsigned int stored = (bmfh.bfOffBits - paletteOffset) / paletteEntrySize;

    if(stored < count)
      count = stored;
  }

  if(count == 0 || count > (unsigned int) MAX_COLORS) {
    cerr << "Unsupported palette of " << count << " colors in " << name << endl;
    return false;
  }

  colorCount = (int) count;

  if(fseek(in, (long) paletteOffset, SEEK_SET) != 0) {
    cerr << "Cannot reach the palette of " << name << endl;
    return false;
  }

  unsigned char * entries = new unsigned char[count * paletteEntrySize];

  if(!readExact(in, entries, count * paletteEntrySize)) {
    cerr << "Unreadable palette in " << name << endl;
    delete[] entries;
    return false;
  }

  palette = new unsigned char[count * RGB_BYTE_SIZE];

  int maxSum = -1, minSum = 3 * 255 + 1;
  whiteIndex = blackIndex = 0;

  for(unsigned int i = 0; i < count; i++){
    // Palette entries are stored blue first.
    palette[i*RGB_BYTE_SIZE]     = entries[i*paletteEntrySize + 2];
    palette[i*RGB_BYTE_SIZE + 1] = entries[i*paletteEntrySize + 1];
    palette[i*RGB_BYTE_SIZE + 2] = entries[i*paletteEntrySize];

    int sum = palette[i*RGB_BYTE_SIZE] + palette[i*RGB_BYTE_SIZE + 1] +
              palette[i*RGB_BYTE_SIZE + 2];
    if(sum > maxSum){
      maxSum = sum;
      whiteIndex = (int) i;
    }
    if(sum < minSum){
      minSum = sum;
      blackIndex = (int) i;
    }
  }

  delete[] entries;
  return true;
}

// Spread the pixels of one file row, which pack several of them per byte below
// 8 bits, over one byte each.
void Bitmap::unpackRow(const unsigned char * row, unsigned char * pixels) const {
  if(bpp == 8) {
    memcpy(pixels, row, (size_t) width);
    return;
  }

  const int perByte = 8 / bpp;
  const unsigned char mask = (unsigned char)((1u << bpp) - 1);

  for(int col = 0; col < width; col++){
    const int shift = 8 - bpp * (col % perByte + 1);
    pixels[col] = (unsigned char)((row[col / perByte] >> shift) & mask);
  }
}

bool Bitmap::readPixels(FILE * in, bool topDown){
  // Rows are padded to a multiple of four bytes, which the unpacked buffer does
  // not keep: reading row by row also avoids trusting the declared image size.
  const size_t stride = (((size_t) width * bpp + 31) / 32) * 4;
  unsigned char * row = new unsigned char[stride];
  bool ok = true;

  for(int r = 0; r < height && ok; r++){
    if(readExact(in, row, stride)) {
      // The first row of a BMP file is the bottom of the image, unless the
      // height was negative.
      unpackRow(row, data + (size_t) (topDown ? r : height - 1 - r) * width);
    } else {
      cerr << "Unreadable image data in " << name << endl;
      ok = false;
    }
  }

  delete[] row;
  return ok;
}

void Bitmap::putPixel(int x, int row, unsigned char index){
  if(x < 0 || row < 0 || x >= width || row >= height)
    return;
  data[(size_t) row * width + x] = index;
}

// Decode a BI_RLE8 or BI_RLE4 stream. Runs that would leave the image are
// clipped rather than rejected: a truncated or overlong stream then shows the
// part that did decode instead of nothing at all.
bool Bitmap::readRlePixels(FILE * in){
  const bool nibbles = (bmih.biCompression == BI_RLE4);
  int x = 0;
  // Rows are counted from the bottom of the image.
  int y = 0;

  for(;;){
    const int count = fgetc(in);
    const int value = fgetc(in);

    if(count == EOF || value == EOF)
      return true;

    if(count > 0) {
      for(int i = 0; i < count; i++){
	unsigned char index = (unsigned char) value;

	if(nibbles)
	  index = (i % 2 == 0) ? (unsigned char)(value >> 4)
			       : (unsigned char)(value & 0x0F);

	putPixel(x + i, height - 1 - y, index);
      }

      x += count;
      continue;
    }

    if(value == RLE_END_OF_LINE) {
      x = 0;
      y++;
    } else if(value == RLE_END_OF_BITMAP) {
      return true;
    } else if(value == RLE_DELTA) {
      const int dx = fgetc(in);
      const int dy = fgetc(in);

      if(dx == EOF || dy == EOF)
	return true;

      x += dx;
      y += dy;
    } else {
      // Absolute mode: as many pixels as the escape announces are stored
      // literally, then padded to a 16 bit boundary.
      const int pixels = value;
      const int bytes = nibbles ? (pixels + 1) / 2 : pixels;

      for(int i = 0; i < bytes; i++){
	const int byte = fgetc(in);

	if(byte == EOF)
	  return true;

	if(nibbles) {
	  putPixel(x + i*2, height - 1 - y, (unsigned char)(byte >> 4));

	  if(i*2 + 1 < pixels)
	    putPixel(x + i*2 + 1, height - 1 - y, (unsigned char)(byte & 0x0F));
	} else {
	  putPixel(x + i, height - 1 - y, (unsigned char) byte);
	}
      }

      if(bytes % 2 != 0)
	fgetc(in);

      x += pixels;
    }
  }
}

// A file may name more colors in its pixels than in its palette. Fold those on
// the last entry so drawing never indexes outside the table.
void Bitmap::clampIndices(){
  const size_t pixels = (size_t) width * height;
  const unsigned char last = (unsigned char) (colorCount - 1);

  for(size_t i = 0; i < pixels; i++)
    if(data[i] > last)
      data[i] = last;
}

/* Draw the image, in screen pixels, into the width x height rectangle: each
   file pixel becomes a block, scaling the image to the window size. Bounds
   are computed incrementally so blocks tile the rectangle exactly, without
   gaps or overlap. */
void Bitmap::drawPixels(int x, int y, int width, int height){
  applyPalette();
  for(int row = 0; row < this->height; row++){
    int y1 = y + row * height / this->height;
    int y2 = y + (row + 1) * height / this->height;
    if(y2 <= y1)
      continue;

    const unsigned char * pixels = data + (size_t) row * this->width;
    int col = 0;
    while(col < this->width){
      unsigned char idx = pixels[col];
      int runStart = col;
      while(col + 1 < this->width && pixels[col + 1] == idx)
        col++;

      int x1 = x + runStart * width / this->width;
      int x2 = x + (col + 1) * width / this->width;
      if(x2 > x1){
        modifierCouleur(idx);
        remplirRectangle(x1, y1, x2, y2);
      }
      col++;
    }
  }
}

void Bitmap::draw(int x, int y) {
  if(!isLoaded())
    return;

  int px = pixX(x), py = pixY(y);
  int width = pixX(x + this->width) - px;
  int height = pixY(y + this->height) - py;

  // After a scale change the reference copy is no longer the right size:
  // redraw the image pixel by pixel.
  bool canBlit = (cacheX >= 0 && cacheWidth == width && cacheHeight == height);
  if(canBlit)
    recupereSousImage(cacheX, cacheY, cacheX + cacheWidth, cacheY + cacheHeight);

  invalidatePixels(px, py, px + width, py + height);

  if(canBlit)
    afficheSousImage(px, py);
  else
    drawPixels(px, py, width, height);

  cacheX = px;
  cacheY = py;
  cacheWidth = width;
  cacheHeight = height;
}

void Bitmap::applyPalette(){
  if(palette == NULL)
    return;
  initPalette(palette, colorCount);
}

/* remplirRectangle paints the window and double buffer with the current
   foreground color: force it to the palette white, then restore black so
   later drawing is unaffected.

   The fill stops one pixel before x2 while tracerRectangle draws its outline
   on x2: add that pixel to erase the outline too. The calculation is done in
   screen pixels because one logical unit can be less than one pixel when the
   window is shrunk. */
void Bitmap::fillBackground(int x1, int y1, int x2, int y2){
  if(palette == NULL)
    return;
  applyPalette();
  modifierCouleur(whiteIndex);
  remplirRectangle(pixX(x1), pixY(y1), pixX(x2) + 1, pixY(y2) + 1);
  modifierCouleur(blackIndex);
}
