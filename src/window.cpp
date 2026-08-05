//------------------------------------------------------//
// window.cpp
// Constant-aspect resizable window wrapper for Graphlib
//-----------------------------------------------------//

extern "C"{
#include "graphlib.h"
}
#include "window.h"
#include "encoding.h"
#include <X11/Xatom.h>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <sys/select.h>
#include <vector>

/*
  Graphlib exposes the X11 objects it uses. We rely on them for two things it
  cannot do: ask the window manager to preserve the aspect ratio, and resize
  the double-buffer pixmap where all of its drawing primitives render.
*/
extern "C" {
  extern Display  * mydisplay;
  extern Window     mywindow;
  extern GC         mygc;
  extern Pixmap     dblbuff;
  extern XSizeHints myhint;
  extern int        profondeur;
}

// Font size at a scale factor of 1.
static const int TEXT_SIZE = 12;

// Quiet period before repainting: a mouse drag produces far more ConfigureNotify
// events than drawing can keep up with.
static const int RESIZE_QUIET_MS = 150;

// Beyond this, stop enforcing the aspect ratio and just center the content.
static const int MAX_CORRECTIONS = 3;

// Share of the screen the window takes when it opens. The reference size is a
// logical coordinate system, not a comfortable size in pixels: 600 of them
// cover barely a quarter of the height of a 1080p screen.
static const double SCREEN_SHARE = 0.75;

static const long EVENT_MASK =
  KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask |
  EnterWindowMask | LeaveWindowMask | KeymapStateMask | ExposureMask |
  FocusChangeMask | StructureNotifyMask;

static int refWidth = 1, refHeight = 1;  // reference window size
static double factor = 1.0;                // screen pixels per logical unit
static int offsetX = 0, offsetY = 0;       // centering when aspect ratio is off
static XFontStruct * font = NULL;          // font at the current scale factor
static double textFactor = 1.0;            // text size asked for by the caller

double scale(){
  return factor;
}

int pixX(int x){
  return offsetX + (int) lround(x * factor);
}

int pixY(int y){
  return offsetY + (int) lround(y * factor);
}

static int logicalX(int x){
  return (int) floor((x - offsetX) / factor);
}

static int logicalY(int y){
  return (int) floor((y - offsetY) / factor);
}

/*
  Graphlib's modifierTailleTexte() is unusable: its XLFD pattern omits the
  average-width field, so it matches no font. We load the font ourselves.
  Vector font families on the X server accept any pixel size; patterns are
  tried in order to match fonts actually installed on the system.
*/
static void updateFont(){
  static const char * patterns[] = {
    "-*-times-medium-r-normal--%d-*-*-*-*-*-iso8859-1",
    "-*-helvetica-medium-r-normal--%d-*-*-*-*-*-iso8859-1",
    "-*-fixed-medium-r-normal--%d-*-*-*-*-*-iso8859-1"
  };

  int size = (int) lround(TEXT_SIZE * factor * textFactor);
  if(size < 1)
    size = 1;

  for(unsigned int i = 0; i < sizeof patterns / sizeof *patterns; i++){
    char name[128];
    snprintf(name, sizeof name, patterns[i], size);

    // XLoadQueryFont returns NULL instead of raising an X error.
    XFontStruct * loaded = XLoadQueryFont(mydisplay, name);
    if(loaded == NULL)
      continue;

    XSetFont(mydisplay, mygc, loaded->fid);
    if(font != NULL)
      XFreeFont(mydisplay, font);
    font = loaded;
    return;
  }
}

void setTextScale(double newFactor){
  if(newFactor <= 0.0 || newFactor == textFactor)
    return;
  textFactor = newFactor;
  updateFont();
}

int textWidth(const char * text){
  std::string latin1 = utf8ToLatin1(text);
  if(font == NULL)
    return latin1.size() * TEXT_SIZE / 2;
  return (int) lround(XTextWidth(font, latin1.c_str(), latin1.size()) / factor);
}

static void applySize(int width, int height){
  if(width <= 0 || height <= 0)
    return;

  double fx = width / (double) refWidth;
  double fy = height / (double) refHeight;
  factor = (fx < fy) ? fx : fy;
  offsetX = (width - (int) lround(refWidth * factor)) / 2;
  offsetY = (height - (int) lround(refHeight * factor)) / 2;

  // Without a new pixmap, areas redrawn after an Expose would be clipped to
  // the old window size.
  XFreePixmap(mydisplay, dblbuff);
  dblbuff = XCreatePixmap(mydisplay, mywindow, width, height, profondeur);
  myhint.width  = width;
  myhint.height = height;
  viderFenetre();

  updateFont();
}

/*
  The icon is drawn here rather than loaded from a file: the ARGB format
  expected by the window manager has nothing in common with the indexed
  bitmaps in assets/, and a grid is easy to describe with a few lines.

  Colors are opaque: the specification does not say whether alpha is
  pre-multiplied, and window managers disagree.
*/
static const long ICON_BACKGROUND = 0xFFFFFFFF;
static const long ICON_LINE       = 0xFFB4B4B4;
static const long ICON_BLOCK      = 0xFF23496E;

static void iconBar(long * pixels, int size,
		    int x, int y, int width, int height, long color){
  for(int j = 0; j < height; j++)
    for(int i = 0; i < width; i++)
      pixels[(y + j) * size + x + i] = color;
}

static void paintIcon(long * pixels, int size){
  for(int i = 0; i < size * size; i++)
    pixels[i] = ICON_BACKGROUND;

  // The two line kinds are distinguished first by color: at small sizes they
  // both collapse to one pixel.
  int blockThickness = size / 32;
  if(blockThickness < 1)
    blockThickness = 1;
  int fineThickness = size / 64;
  if(fineThickness < 1)
    fineThickness = 1;

  // Below 48 pixels, a cell is less than three pixels wide and all nine
  // columns turn gray: keep only the block separators.
  bool fineLines = (size >= 48);

  // Block lines are drawn second so they cover the fine lines.
  for(int pass = 0; pass < 2; pass++){
    bool blocks = (pass == 1);
    if(!blocks && !fineLines)
      continue;

    int thickness = blocks ? blockThickness : fineThickness;
    long color = blocks ? ICON_BLOCK : ICON_LINE;

    for(int k = 0; k <= 9; k++){
      if((k % 3 == 0) != blocks)
	continue;
      // Line k = 9 must stay inside the image, hence the subtracted thickness.
      int pos = k * (size - thickness) / 9;
      iconBar(pixels, size, pos, 0, thickness, size, color);
      iconBar(pixels, size, 0, pos, size, thickness, color);
    }
  }
}

/*
  _NET_WM_ICON is a sequence of images, each preceded by its width and height;
  the window manager keeps the one closest to the size it needs. CARD32 values
  in a format-32 property are stored in a long array regardless of long size.

  Mutter 18 (GNOME 50) no longer exposes these pixels to GNOME Shell, so the
  property is useless under GNOME but still read by other environments.
  setDockIcon() supplies the icon shown in the GNOME dock.
*/
static void setWindowIcon(){
  static const int sizes[] = { 16, 32, 48, 64, 128 };

  std::vector<long> data;
  for(unsigned int i = 0; i < sizeof sizes / sizeof *sizes; i++){
    int size = sizes[i];
    size_t start = data.size();
    data.resize(start + 2 + (size_t) size * size);
    data[start]     = size;
    data[start + 1] = size;
    paintIcon(&data[start + 2], size);
  }

  XChangeProperty(mydisplay, mywindow,
		  XInternAtom(mydisplay, "_NET_WM_ICON", False),
		  XA_CARDINAL, 32, PropModeReplace,
		  (const unsigned char *) data.data(), (int) data.size());
}

/*
  GNOME Shell associates a window with an application, and therefore a dock
  icon, only by comparing its WM_CLASS to the StartupWMClass of installed
  desktop entries. Without WM_CLASS the window stays anonymous and gets the
  generic application-x-executable icon.

  The class must stay identical to the StartupWMClass in
  packaging/sudoku.desktop.in.
*/
static void setDockIcon(){
  XClassHint * hint = XAllocClassHint();
  hint->res_name  = (char *) "sudoku";
  hint->res_class = (char *) "Sudoku";
  XSetClassHint(mydisplay, mywindow, hint);
  XFree(hint);
}

/*
  Graphlib connects to the X server only when the window opens, so the screen
  dimensions are read over a connection of our own. A failure here is not worth
  reporting: the window opens at its reference size instead.
*/
static void screenSize(int * width, int * height){
  *width = *height = 0;

  Display * display = XOpenDisplay(NULL);
  if(display == NULL)
    return;

  int screen = DefaultScreen(display);
  *width  = DisplayWidth(display, screen);
  *height = DisplayHeight(display, screen);
  XCloseDisplay(display);
}

static void initialSize(int * width, int * height){
  int screenWidth, screenHeight;
  screenSize(&screenWidth, &screenHeight);

  double f = 1.0;
  if(screenWidth > 0 && screenHeight > 0){
    // Both axes are considered: with several monitors merged into one screen,
    // the width alone would size the window for the whole desk.
    double fx = screenWidth * SCREEN_SHARE / refWidth;
    double fy = screenHeight * SCREEN_SHARE / refHeight;
    f = (fx < fy) ? fx : fy;
  }

  // Small screens keep the reference size, which the user can still shrink.
  if(f < 1.0)
    f = 1.0;

  *width  = (int) lround(refWidth * f);
  *height = (int) lround(refHeight * f);
}

void openScalableWindow(int logicalWidth, int logicalHeight, const char * title){
  refWidth = logicalWidth;
  refHeight = logicalHeight;

  int width, height;
  initialSize(&width, &height);

  ouvrirFenetreTailleTitre(width, height, (char *) title);

  // graphlib sets the title via XSetStandardProperties (Latin-1).
  Xutf8SetWMProperties(mydisplay, mywindow, title, title, NULL, 0, NULL, NULL, NULL);

  setDockIcon();
  setWindowIcon();

  XSizeHints * hints = XAllocSizeHints();
  hints->flags = PSize | PMinSize | PBaseSize | PAspect;
  hints->width  = width;
  hints->height = height;
  hints->min_width  = refWidth / 4;
  hints->min_height = refHeight / 4;
  // Aspect ratios apply to the size minus the base size.
  hints->base_width = hints->base_height = 0;
  hints->min_aspect.x = hints->max_aspect.x = refWidth;
  hints->min_aspect.y = hints->max_aspect.y = refHeight;
  XSetWMNormalHints(mydisplay, mywindow, hints);
  XFree(hints);

  // Graphlib does not listen for ConfigureNotify.
  XSelectInput(mydisplay, mywindow, EVENT_MASK);

  applySize(width, height);
}

// Wait for an event, at most timeoutMs milliseconds.
static bool waitForEvent(int timeoutMs){
  if(XPending(mydisplay))
    return true;

  int fd = ConnectionNumber(mydisplay);
  fd_set readSet;
  FD_ZERO(&readSet);
  FD_SET(fd, &readSet);

  struct timeval timeout;
  timeout.tv_sec  = timeoutMs / 1000;
  timeout.tv_usec = (timeoutMs % 1000) * 1000;

  return select(fd + 1, &readSet, NULL, NULL, &timeout) > 0;
}

/*
  Closest size that respects the reference aspect ratio. Follow the axis the
  user moved most; otherwise dragging a horizontal edge would cancel the
  gesture by snapping the window back to its original width.
*/
static void sizeForAspect(int width, int height, int * targetW, int * targetH){
  double f;
  if(abs(width - myhint.width) >= abs(height - myhint.height))
    f = width / (double) refWidth;
  else
    f = height / (double) refHeight;

  int w = (int) lround(refWidth * f);
  int h = (int) lround(refHeight * f);

  // Stay above the advertised minimum, otherwise the window manager would
  // correct again and we would loop.
  if(w < refWidth / 4 || h < refHeight / 4){
    w = refWidth / 4;
    h = refHeight / 4;
  }

  *targetW = w;
  *targetH = h;
}

bool waitForLogicalClick(int * x, int * y){
  bool needsRefresh = false;
  int corrections  = 0;
  int desiredWidth = myhint.width, desiredHeight = myhint.height;

  for(;;){
    bool needsResize = (desiredWidth != myhint.width ||
			desiredHeight != myhint.height);

    if(!XPending(mydisplay)){
      if(needsResize){
	// A mouse drag chains ConfigureNotify events: wait for a pause so we
	// repaint only once, at the final size.
	if(!waitForEvent(RESIZE_QUIET_MS)){
	  int targetW, targetH;
	  sizeForAspect(desiredWidth, desiredHeight, &targetW, &targetH);

	  // PAspect is not always applied during a mouse resize: correct once
	  // the gesture is finished.
	  if((targetW != desiredWidth || targetH != desiredHeight) &&
	     corrections < MAX_CORRECTIONS){
	    corrections++;
	    XResizeWindow(mydisplay, mywindow, targetW, targetH);
	    continue;
	  }

	  applySize(desiredWidth, desiredHeight);
	  return false;
	}
      }else if(needsRefresh){
	rafraichirFenetre();
	needsRefresh = false;
      }
    }

    XEvent event;
    XNextEvent(mydisplay, &event);

    switch(event.type){
    case ButtonPress:
      if(needsResize){
	XPutBackEvent(mydisplay, &event);
	applySize(desiredWidth, desiredHeight);
	return false;
      }
      *x = logicalX(event.xbutton.x);
      *y = logicalY(event.xbutton.y);
      return true;

    case ConfigureNotify:
      desiredWidth = event.xconfigure.width;
      desiredHeight = event.xconfigure.height;
      break;

    case Expose:
      needsRefresh = true;
      break;
    }
  }
}

void drawLine(int x1, int y1, int x2, int y2){
  tracerLigne(pixX(x1), pixY(y1), pixX(x2), pixY(y2));
}

void drawRect(int x1, int y1, int x2, int y2){
  tracerRectangle(pixX(x1), pixY(y1), pixX(x2), pixY(y2));
}

void fillRect(int x1, int y1, int x2, int y2){
  remplirRectangle(pixX(x1), pixY(y1), pixX(x2), pixY(y2));
}

void fillTriangle(int x1, int y1, int x2, int y2, int x3, int y3){
  remplirTriangle(pixX(x1), pixY(y1), pixX(x2), pixY(y2), pixX(x3), pixY(y3));
}

void drawText(int x, int y, const char * text){
  std::string latin1 = utf8ToLatin1(text);
  ecrireSurImpression(pixX(x), pixY(y), (char *) latin1.c_str());
}
