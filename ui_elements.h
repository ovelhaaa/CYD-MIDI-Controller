#ifndef UI_ELEMENTS_H
#define UI_ELEMENTS_H

#include "common_definitions.h"
#include "cyd_touch.h"

// UI function declarations
void updateTouch();
void updateStatus();
bool isButtonPressed(int x, int y, int w, int h);
void drawRoundButton(int x, int y, int w, int h, String text, uint16_t color, bool pressed = false);
void drawHeader(String title, String subtitle = "");
void exitToMenu();

// UI implementations
void updateTouch() {
  CYDTouchPoint p;
  static uint32_t lastTouchLogMs = 0;
  static uint32_t lastIdleDrawMs = 0;

  touch.wasPressed = touch.isPressed;
  touch.isPressed = cydTouchRead(p);
  touch.justPressed = touch.isPressed && !touch.wasPressed;
  touch.justReleased = !touch.isPressed && touch.wasPressed;
  
  if (touch.isPressed) {
    touch.x = constrain(map(p.x, 240, 3800, 0, 320), 0, 319);
    touch.y = constrain(map(p.y, 200, 3700, 0, 240), 0, 239);
    touch.rawX = p.x;
    touch.rawY = p.y;
    touch.rawZ = p.z;

    if (currentMode == MENU && millis() - lastTouchLogMs > 120) {
      tft.fillRect(0, 224, 190, 16, THEME_BG);
      tft.setTextColor(THEME_SUCCESS, THEME_BG);
      tft.drawString("T:" + String(touch.x) + "," + String(touch.y) + " z" + String(touch.rawZ), 4, 226, 1);
      tft.fillCircle(touch.x, touch.y, 3, THEME_SUCCESS);
      Serial.printf("touch raw x=%d y=%d z=%d mapped x=%d y=%d\n", touch.rawX, touch.rawY, touch.rawZ, touch.x, touch.y);
      lastTouchLogMs = millis();
    }
  } else if (currentMode == MENU && millis() - lastIdleDrawMs > 1000) {
    tft.fillRect(0, 224, 190, 16, THEME_BG);
    tft.setTextColor(THEME_TEXT_DIM, THEME_BG);
    tft.drawString("T:--", 4, 226, 1);
    lastIdleDrawMs = millis();
  }
}

bool isButtonPressed(int x, int y, int w, int h) {
  return touch.x >= x && touch.x <= x + w && touch.y >= y && touch.y <= y + h;
}

void drawRoundButton(int x, int y, int w, int h, String text, uint16_t color, bool pressed) {
  uint16_t bgColor = pressed ? color : THEME_SURFACE;
  uint16_t borderColor = color;
  uint16_t textColor = pressed ? THEME_BG : color;
  
  tft.fillRoundRect(x, y, w, h, 8, bgColor);
  tft.drawRoundRect(x, y, w, h, 8, borderColor);
  tft.drawRoundRect(x+1, y+1, w-2, h-2, 7, borderColor);
  
  tft.setTextColor(textColor, bgColor);
  tft.drawCentreString(text, x + w/2, y + h/2 - 8, 2);
}

void drawHeader(String title, String subtitle) {
  tft.fillRect(0, 0, 320, 45, THEME_SURFACE);
  tft.drawFastHLine(0, 45, 320, THEME_PRIMARY);
  
  tft.setTextColor(THEME_TEXT, THEME_SURFACE);
  tft.drawCentreString(title, 160, 8, 4);
  
  if (subtitle.length() > 0) {
    tft.setTextColor(THEME_TEXT_DIM, THEME_SURFACE);
    tft.drawCentreString(subtitle, 160, 28, 2);
  }
  
  drawRoundButton(10, 10, 50, 25, "BACK", THEME_ERROR);
}

void updateStatus() {
  // Status bar removed - no more BLE connection alerts on every screen
}

#endif
