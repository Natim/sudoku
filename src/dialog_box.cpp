#include "dialog_box.h"
#include "window.h"

extern "C"{
#include "graphlib.h"
}

#include <string>
#include <vector>

using namespace std;

static void (*repaintBackground)() = NULL;

struct ButtonArea {
  int x1, y1, x2, y2;
};

void setBackgroundRepaint(void (*paint)()){
  repaintBackground = paint;
}

static vector<ButtonArea> layoutButtons(int x, int y, int width, int height,
					  const vector<string> & labels){
  const int margin = 10;
  const int y1 = y + height - 30;
  const int y2 = y + height - 10;
  const int gap = 8;

  vector<int> widths;
  widths.reserve(labels.size());
  int total = 0;
  for(unsigned int i = 0; i < labels.size(); i++){
    int w = textWidth(labels[i].c_str()) + 16;
    if(w < 30)
      w = 30;
    widths.push_back(w);
    total += w;
  }
  if(!labels.empty())
    total += gap * ((int) labels.size() - 1);

  int start = x + margin;
  if(start + total < x + width - margin)
    start = x + (width - total) / 2;

  vector<ButtonArea> areas;
  int current = start;
  for(unsigned int i = 0; i < labels.size(); i++){
    ButtonArea area;
    area.x1 = current;
    area.y1 = y1;
    area.x2 = current + widths[i];
    area.y2 = y2;
    areas.push_back(area);
    current += widths[i] + gap;
  }
  return areas;
}

static void drawBox(int x, int y, int width, int height,
		    const string & message, const string & title,
		    const vector<string> & labels){
  initPalette(NULL, 0);

  modifierCouleur(20);
  fillRect(x, y, x + width, y + height);

  modifierCouleur(238);
  fillRect(x, y, x + width, y + 20);
  modifierCouleur(1);
  const int titleWidth = textWidth(title.c_str());
  const int titleStart = x + (width - titleWidth) / 2;
  drawText(titleStart, y + 15, title.c_str());
  drawLine(titleStart, y + 17, titleStart + titleWidth, y + 17);

  modifierCouleur(1);
  drawText(x + 20, y + 45, message.c_str());

  vector<ButtonArea> areas = layoutButtons(x, y, width, height, labels);
  for(unsigned int i = 0; i < labels.size(); i++){
    modifierCouleur(1);
    drawRect(areas[i].x1, areas[i].y1, areas[i].x2, areas[i].y2);
    const int labelWidth = textWidth(labels[i].c_str());
    const int textX = areas[i].x1 + (areas[i].x2 - areas[i].x1 - labelWidth) / 2;
    drawText(textX, areas[i].y2 - 5, labels[i].c_str());
  }

  modifierCouleur(0);
  drawRect(x, y, x + width, y + height);
  drawRect(x, y + 20, x + width, y + height);
}

static int clickedButton(int X, int Y, const vector<ButtonArea> & areas){
  for(unsigned int i = 0; i < areas.size(); i++){
    if(X > areas[i].x1 && X < areas[i].x2 &&
       Y > areas[i].y1 && Y < areas[i].y2)
      return (int) i;
  }
  return -1;
}

int choose(int x, int y, int width, int height,
	   string title, string message,
	   const vector<string> & choices){
  int X, Y;
  vector<ButtonArea> areas = layoutButtons(x, y, width, height, choices);
  drawBox(x, y, width, height, message, title, choices);

  while(true){
    if(!waitForLogicalClick(&X, &Y)){
      if(repaintBackground != NULL)
	repaintBackground();
      areas = layoutButtons(x, y, width, height, choices);
      drawBox(x, y, width, height, message, title, choices);
      continue;
    }

    int idx = clickedButton(X, Y, areas);
    if(idx >= 0)
      return idx;
  }
}

bool alert(int x, int y, string message){
  return alert(x, y, "/!\\ Attention /!\\", message);
}

bool alert(int x, int y, string title, string message){
  return alert(x, y, 300, 100, title, message);
}

bool alert(int x, int y, int width, int height, string title, string message){
  vector<string> buttons;
  buttons.push_back("Oui");
  buttons.push_back("Non");
  return choose(x, y, width, height, title, message, buttons) == 0;
}
