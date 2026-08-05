// Checks the BMP reader against the variants it supports, by writing files
// whose expected content is known and comparing the decoded pixels.
//
//   make verify-bitmap
//
// Nothing here opens a window, so it runs without a display.
#include "bitmap.h"

#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

using namespace std;

static const char * TMP = "build/verify_bitmap.bmp";

static int failures = 0;

static void check(bool ok, const string & what){
  printf("  %-46s %s\n", what.c_str(), ok ? "ok" : "FAIL");
  if(!ok)
    failures++;
}

//---------------------------------------------------------------------------//
// Reference image, one palette index per pixel, top row first.
//---------------------------------------------------------------------------//
struct Image {
  int width, height;
  vector<unsigned char> pixels;

  Image(int w, int h) : width(w), height(h), pixels((size_t) w * h, 0) {}

  unsigned char & at(int x, int y) { return pixels[(size_t) y * width + x]; }
  unsigned char at(int x, int y) const { return pixels[(size_t) y * width + x]; }
};

// Runs of identical pixels keep the run length encoders busy, and a width that
// is not a multiple of four exercises the row padding. The corners break the
// symmetry of the pattern, which a two color image would otherwise keep when
// flipped upside down.
static Image reference(int width, int height, int maxIndex){
  Image img(width, height);
  for(int y = 0; y < height; y++)
    for(int x = 0; x < width; x++)
      img.at(x, y) = (unsigned char)((x / 3 + y) % (maxIndex + 1));
  img.at(0, 0) = (unsigned char) maxIndex;
  img.at(width - 1, height - 1) = 0;
  return img;
}

//---------------------------------------------------------------------------//
// BMP writer
//---------------------------------------------------------------------------//
struct Writer {
  vector<unsigned char> bytes;

  void u8(unsigned int v){ bytes.push_back((unsigned char)(v & 0xFF)); }
  void u16(unsigned int v){ u8(v); u8(v >> 8); }
  void u32(unsigned int v){ u16(v); u16(v >> 16); }
  void s32(int v){ u32((unsigned int) v); }
  void patchU32(size_t at, unsigned int v){
    for(int i = 0; i < 4; i++)
      bytes[at + i] = (unsigned char)((v >> (8 * i)) & 0xFF);
  }
  void align(size_t start){
    while((bytes.size() - start) % 4 != 0)
      u8(0);
  }
};

struct Options {
  int bpp;
  unsigned int headerSize;
  bool topDown;
  unsigned int compression;
  unsigned int clrUsed;
  // Number of entries actually stored, -1 for the depth's full table.
  int paletteEntries;

  Options() : bpp(8), headerSize(40), topDown(false), compression(0),
	      clrUsed(0), paletteEntries(-1) {}
};

static void writeHeaders(Writer & w, const Image & img, const Options & opt){
  w.u16(19778);
  w.u32(0);                                  // patched: file size
  w.u16(0);
  w.u16(0);
  w.u32(0);                                  // patched: pixel data offset

  w.u32(opt.headerSize);

  if(opt.headerSize == 12){
    w.u16((unsigned int) img.width);
    w.u16((unsigned int) img.height);
    w.u16(1);
    w.u16((unsigned int) opt.bpp);
  } else {
    w.s32(img.width);
    w.s32(opt.topDown ? -img.height : img.height);
    w.u16(1);
    w.u16((unsigned int) opt.bpp);
    w.u32(opt.compression);
    w.u32(0);
    w.s32(2835);
    w.s32(2835);
    w.u32(opt.clrUsed);
    w.u32(0);
    for(unsigned int i = 40; i < opt.headerSize; i++)
      w.u8(0);                               // V2 to V5 masks and color space
  }

  const int entries = (opt.paletteEntries >= 0) ? opt.paletteEntries : (1 << opt.bpp);
  const int entrySize = (opt.headerSize == 12) ? 3 : 4;

  for(int i = 0; i < entries; i++){
    w.u8((unsigned int)(i * 3) & 0xFF);      // blue
    w.u8((unsigned int)(i * 5) & 0xFF);      // green
    w.u8((unsigned int)(i * 7) & 0xFF);      // red
    if(entrySize == 4)
      w.u8(0);
  }

  w.patchU32(10, (unsigned int) w.bytes.size());
}

static void writeUncompressed(Writer & w, const Image & img, const Options & opt){
  const int perByte = 8 / opt.bpp;

  for(int r = 0; r < img.height; r++){
    const int y = opt.topDown ? r : img.height - 1 - r;
    const size_t rowStart = w.bytes.size();
    unsigned int acc = 0;
    int packed = 0;

    for(int x = 0; x < img.width; x++){
      acc = (acc << opt.bpp) | img.at(x, y);
      if(++packed == perByte){
	w.u8(acc);
	acc = 0;
	packed = 0;
      }
    }

    if(packed != 0)
      w.u8(acc << (8 - opt.bpp * packed));

    w.align(rowStart);
  }
}

// Straightforward encoder: one run per group of identical pixels, so the reader
// is checked on encoded runs and end of line markers. Absolute mode and deltas
// are covered by the hand written streams below.
static void writeRle(Writer & w, const Image & img, const Options & opt){
  const bool nibbles = (opt.compression == 2);

  for(int r = 0; r < img.height; r++){
    const int y = img.height - 1 - r;
    int x = 0;

    while(x < img.width){
      const unsigned char value = img.at(x, y);
      int run = 1;
      while(x + run < img.width && img.at(x + run, y) == value && run < 255)
	run++;

      w.u8((unsigned int) run);
      w.u8(nibbles ? (unsigned int)((value << 4) | value) : value);
      x += run;
    }

    w.u8(0);
    w.u8(0);                                 // end of line
  }

  w.u8(0);
  w.u8(1);                                   // end of bitmap
}

static void writeFile(const Writer & w){
  Writer copy = w;
  copy.patchU32(2, (unsigned int) copy.bytes.size());
  FILE * out = fopen(TMP, "wb");
  fwrite(copy.bytes.data(), 1, copy.bytes.size(), out);
  fclose(out);
}

static void writeBMP(const Image & img, const Options & opt){
  Writer w;
  writeHeaders(w, img, opt);
  if(opt.compression == 0)
    writeUncompressed(w, img, opt);
  else
    writeRle(w, img, opt);
  writeFile(w);
}

//---------------------------------------------------------------------------//
// Cases
//---------------------------------------------------------------------------//
static bool matches(const Bitmap & bmp, const Image & img){
  if(!bmp.isLoaded() || bmp.width != img.width || bmp.height != img.height)
    return false;

  for(int y = 0; y < img.height; y++)
    for(int x = 0; x < img.width; x++)
      if(bmp.indexAt(x, y) != img.at(x, y))
	return false;

  return true;
}

static void roundTrip(const string & what, const Image & img, const Options & opt,
		      int expectedColors = -1){
  writeBMP(img, opt);
  Bitmap bmp(TMP);
  bool ok = matches(bmp, img);
  if(ok && expectedColors >= 0 && bmp.colorCount != expectedColors)
    ok = false;
  check(ok, what);
}

static void checkVariants(){
  printf("supported variants\n");

  for(int bpp = 1; bpp <= 8; bpp *= 2){
    Options opt;
    opt.bpp = bpp;
    // 13 is not a multiple of four at any depth, so every row is padded.
    Image img = reference(13, 7, (1 << bpp) - 1);
    char label[64];
    snprintf(label, sizeof label, "%d bpp, padded rows", bpp);
    roundTrip(label, img, opt, 1 << bpp);
  }

  Image img = reference(13, 7, 255);

  Options topDown;
  topDown.topDown = true;
  roundTrip("8 bpp, stored top down", img, topDown);

  const unsigned int laterHeaders[] = {52, 56, 108, 124};
  for(size_t i = 0; i < sizeof laterHeaders / sizeof laterHeaders[0]; i++){
    Options later;
    later.headerSize = laterHeaders[i];
    char label[64];
    snprintf(label, sizeof label, "info header of %u bytes", laterHeaders[i]);
    roundTrip(label, img, later);
  }

  Options core;
  core.headerSize = 12;
  roundTrip("OS/2 core header, 3 byte palette", img, core);

  Options counted;
  counted.clrUsed = 200;
  counted.paletteEntries = 200;
  roundTrip("biClrUsed smaller than the depth", reference(13, 7, 199), counted, 200);

  // Without biClrUsed the entry count can only come from the data offset.
  Options shortTable;
  shortTable.paletteEntries = 5;
  roundTrip("palette shorter than the depth allows", reference(13, 7, 4), shortTable, 5);

  Options rle8;
  rle8.compression = 1;
  roundTrip("RLE8, encoded runs", img, rle8);

  Options rle4;
  rle4.bpp = 4;
  rle4.compression = 2;
  roundTrip("RLE4, encoded runs", reference(13, 7, 15), rle4, 16);
}

// Streams written by hand, to reach the absolute runs, the padding of an odd
// absolute run and the deltas that an encoder rarely produces.
static void checkRleEscapes(){
  printf("RLE escape codes\n");

  {
    Image expected(8, 2);
    const unsigned char row0[8] = {0, 9, 9, 9, 9, 0, 0, 0};
    const unsigned char row1[8] = {5, 5, 10, 11, 12, 0, 0, 0};
    for(int x = 0; x < 8; x++){
      expected.at(x, 0) = row0[x];
      expected.at(x, 1) = row1[x];
    }

    Options opt;
    opt.compression = 1;
    Writer w;
    writeHeaders(w, expected, opt);
    const unsigned char stream[] = {
      0x02, 0x05,                              // bottom row: two pixels of 5
      0x00, 0x03, 0x0A, 0x0B, 0x0C, 0x00,      // absolute run of 3, one pad byte
      0x00, 0x00,                              // end of line
      0x00, 0x02, 0x01, 0x00,                  // delta, one pixel to the right
      0x04, 0x09,                              // four pixels of 9
      0x00, 0x01                               // end of bitmap
    };
    for(size_t i = 0; i < sizeof stream; i++)
      w.u8(stream[i]);
    writeFile(w);

    Bitmap bmp(TMP);
    check(matches(bmp, expected), "RLE8, absolute run, padding and delta");
  }

  {
    Image expected(8, 2);
    const unsigned char row0[8] = {0, 0, 12, 12, 0, 0, 0, 0};
    const unsigned char row1[8] = {10, 11, 10, 1, 2, 3, 4, 5};
    for(int x = 0; x < 8; x++){
      expected.at(x, 0) = row0[x];
      expected.at(x, 1) = row1[x];
    }

    Options opt;
    opt.bpp = 4;
    opt.compression = 2;
    Writer w;
    writeHeaders(w, expected, opt);
    const unsigned char stream[] = {
      0x03, 0xAB,                              // bottom row: A, B, A
      0x00, 0x05, 0x12, 0x34, 0x50, 0x00,      // absolute run of 5, one pad byte
      0x00, 0x00,                              // end of line
      0x00, 0x02, 0x02, 0x00,                  // delta, two pixels to the right
      0x02, 0xCC,                              // two pixels of C
      0x00, 0x01                               // end of bitmap
    };
    for(size_t i = 0; i < sizeof stream; i++)
      w.u8(stream[i]);
    writeFile(w);

    Bitmap bmp(TMP);
    check(matches(bmp, expected), "RLE4, absolute run, padding and delta");
  }
}

static void rejects(const string & what, const Writer & w){
  writeFile(w);
  Bitmap bmp(TMP);
  check(!bmp.isLoaded() && bmp.width == 0 && bmp.height == 0, what);
}

static void checkRejections(){
  printf("files that must be refused, without leaving a half loaded image\n");

  {
    Bitmap bmp("build/there_is_no_such_file.bmp");
    check(!bmp.isLoaded(), "missing file");
  }

  Image img = reference(13, 7, 255);
  Options opt;

  {
    Writer w;
    writeHeaders(w, img, opt);
    writeUncompressed(w, img, opt);
    w.bytes[0] = 'P';
    rejects("wrong magic number", w);
  }

  {
    Options deep;
    deep.bpp = 24;
    deep.paletteEntries = 0;
    Writer w;
    writeHeaders(w, img, deep);
    for(int i = 0; i < img.width * img.height * 3; i++)
      w.u8(0);
    rejects("24 bits per pixel", w);
  }

  {
    Options masks;
    masks.compression = 3;                     // BI_BITFIELDS
    Writer w;
    writeHeaders(w, img, masks);
    writeUncompressed(w, img, masks);
    rejects("unsupported compression", w);
  }

  {
    Options bad;
    bad.headerSize = 20;
    Writer w;
    writeHeaders(w, img, bad);
    writeUncompressed(w, img, bad);
    rejects("unknown header size", w);
  }

  {
    Options rle;
    rle.compression = 1;
    rle.topDown = true;
    Writer w;
    writeHeaders(w, img, rle);
    writeRle(w, img, rle);
    rejects("compressed and top down at once", w);
  }

  {
    Writer w;
    writeHeaders(w, img, opt);
    writeUncompressed(w, img, opt);
    w.bytes.resize(w.bytes.size() - 20);
    rejects("truncated image data", w);
  }

  {
    Writer w;
    writeHeaders(w, img, opt);
    writeUncompressed(w, img, opt);
    w.patchU32(10, 4);                         // data offset inside the headers
    rejects("image data offset inside the headers", w);
  }

  {
    Image empty(1, 1);
    Writer w;
    writeHeaders(w, empty, opt);
    w.patchU32(18, 0);                         // width of zero
    writeUncompressed(w, empty, opt);
    rejects("empty image", w);
  }
}

// A failed load must leave an object that still refuses to draw rather than one
// that reads freed or absent pixels.
static void checkReload(){
  printf("state after a failed load\n");

  Image img = reference(13, 7, 255);
  Options opt;
  writeBMP(img, opt);

  Bitmap bmp(TMP);
  check(matches(bmp, img), "first load");
  check(!bmp.loadBMP("build/there_is_no_such_file.bmp"), "reload of a missing file fails");
  check(!bmp.isLoaded() && bmp.indexAt(0, 0) == -1, "pixels are gone, not stale");
  check(bmp.loadBMP(TMP) && matches(bmp, img), "loading again works");
}

int main(){
  checkVariants();
  checkRleEscapes();
  checkRejections();
  checkReload();
  remove(TMP);

  printf("\n%s\n", failures == 0 ? "all checks passed" : "some checks failed");
  return failures == 0 ? 0 : 1;
}
