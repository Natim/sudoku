#pragma once

void openScalableWindow(int width, int height, const char * title);

double scale();
int pixX(int x);
int pixY(int y);

bool waitForLogicalClick(int * x, int * y);

void drawLine(int x1, int y1, int x2, int y2);
void drawRect(int x1, int y1, int x2, int y2);
void fillRect(int x1, int y1, int x2, int y2);
void drawText(int x, int y, const char * text);
int  textWidth(const char * text);
