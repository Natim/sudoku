//------------------------------------------------------//
// bitmap.cpp
// Bitmap class to load and display 8-bit BMP files with Graphlib
//-----------------------------------------------------//
// Author: Natim
// Last modified: 18-04-2006
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
int Bitmap::instanceCount = 0;

Bitmap::Bitmap(){
  colors = NULL;
  data  = NULL;
  palette = NULL;
  width = height = 0;
  cacheX = cacheY = -1;
  cacheWidth = cacheHeight = 0;
  whiteIndex = blackIndex = 0;
  registerInstance();
}

Bitmap::Bitmap(const char *file){
  colors = NULL;
  data  = NULL;
  palette = NULL;
  cacheX = cacheY = -1;
  cacheWidth = cacheHeight = 0;
  whiteIndex = blackIndex = 0;
  loadBMP(file);
  registerInstance();
}

Bitmap::~Bitmap(){
  unregisterInstance();
  if(colors != NULL) {
    delete[] colors;
  }
  if(data  != NULL) {
    delete[] data;
  }
  if(palette  != NULL){
    delete[] palette;
  }
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

bool Bitmap::loadBMP(const char *file) {
  FILE * in = NULL;
  name = file;

  delete[] colors;
  colors = NULL;
  delete[] data;
  data  = NULL;
  delete[] palette;
  palette = NULL;

  in = fopen(file, "rb");

  if(in == NULL) {
    cerr << "Cannot open file " << file << endl;
    return false;
  }

  if(fread(&bmfh, sizeof(BitmapFileHeader), 1, in) != 1) {
    cerr << "Unreadable file header in " << file << endl;
    fclose(in);
    return false;
  }

  if(bmfh.bfType != BITMAP_MAGIC_NUMBER) {
    cerr << "This file is not in DIB format";
    fclose(in);
    return false;
  }

  if(fread(&bmih, sizeof(BitmapInfoHeader), 1, in) != 1) {
    cerr << "Unreadable bitmap header in " << file << endl;
    fclose(in);
    return false;
  }

  width  = bmih.biWidth;
  height = bmih.biHeight;
  bpp    = bmih.biBitCount;

  cerr << name << "\t:\t" << width << "x" << height << " - " << bpp << "bits" << endl;

  dataSize = (width * height * (unsigned int) ceil(bpp/8.0));

  colorCount = (bmih.biClrUsed != 0) ? bmih.biClrUsed : 256;

  if(bpp > 8) {
    cerr << "Only bitmaps of at most 8 bits are supported" << endl;
    fclose(in);
    exit(1);
    return false;
  }

  palette = new unsigned char[colorCount*3];
  colors = new RgbColor[colorCount];

  if(fread(colors, sizeof(RgbColor), colorCount, in) != (size_t) colorCount) {
    cerr << "Unreadable palette in " << file << endl;
    fclose(in);
    return false;
  }

  int maxSum = -1, minSum = 3 * 255 + 1;
  whiteIndex = blackIndex = 0;
  for(int i = 0; i < colorCount; i++){
    palette[i*3]   = colors[i].red;
    palette[i*3+1] = colors[i].green;
    palette[i*3+2] = colors[i].blue;

    int sum = palette[i*3] + palette[i*3+1] + palette[i*3+2];
    if(sum > maxSum){
      maxSum = sum;
      whiteIndex = i;
    }
    if(sum < minSum){
      minSum = sum;
      blackIndex = i;
    }
  }

  data = new char[dataSize];

  if(data == NULL) {
    cerr << "Not enough memory to load the file" << endl;
    fclose(in);
    return false;
  }

  fseek(in, bmfh.bfOffBits, SEEK_SET);
  if(fread(data, sizeof(char), dataSize, in) != dataSize) {
    cerr << "Unreadable image data in " << file << endl;
    fclose(in);
    return false;
  }

  fclose(in);
  byteWidth = padWidth = (unsigned int)(width * ceil(bpp / 8.0));

  while(padWidth%4 != 0) {
    padWidth++;
  }

  return true;
}

/* Draw the image, in screen pixels, into the width x height rectangle: each
   file pixel becomes a block, scaling the image to the window size. Bounds
   are computed incrementally so blocks tile the rectangle exactly, without
   gaps or overlap. */
void Bitmap::drawPixels(int x, int y, int width, int height){
  applyPalette();
  for(int row = 0; row < this->height; row++){
    // The first row in a BMP file is the bottom of the image.
    int y1 = y + (this->height - 1 - row) * height / this->height;
    int y2 = y + (this->height - row) * height / this->height;
    if(y2 <= y1)
      continue;

    int col = 0;
    while(col < this->width){
      unsigned char idx = (unsigned char) data[row * padWidth + col];
      idx = idx % colorCount;
      int runStart = col;
      while(col + 1 < this->width &&
            (unsigned char) data[row * padWidth + col + 1] % colorCount == idx)
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
  applyPalette();
  modifierCouleur(whiteIndex);
  remplirRectangle(pixX(x1), pixY(y1), pixX(x2) + 1, pixY(y2) + 1);
  modifierCouleur(blackIndex);
}
