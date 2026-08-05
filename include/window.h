#pragma once

// The size given is the logical coordinate system drawing uses; the window
// itself opens larger or smaller depending on the screen.
void openScalableWindow(int logicalWidth, int logicalHeight, const char * title);

double scale();
int pixX(int x);
int pixY(int y);

bool waitForLogicalClick(int * x, int * y);

void drawLine(int x1, int y1, int x2, int y2);
void drawRect(int x1, int y1, int x2, int y2);
void fillRect(int x1, int y1, int x2, int y2);
void fillTriangle(int x1, int y1, int x2, int y2, int x3, int y3);
void drawText(int x, int y, const char * text);
int  textWidth(const char * text);

// Size of the text drawn from now on, 1.0 being the usual size. Callers that
// enlarge the text are expected to set it back.
void setTextScale(double factor);
