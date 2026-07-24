#include <Arduino.h>
#include <FS.h>
#include <SPIFFS.h>
#include "../common_definitions.h"

void drawMenu();
void drawAppGraphics(AppMode mode, int x, int y, int iconSize);
void handleMenuTouch();
void enterMode(AppMode mode);
void exitToMenu();

#include "../CYD-MIDI-Controller.ino"
