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
#ifdef DEBUG_TOUCH
  static uint32_t lastTouchLogMs = 0;
  static uint32_t lastIdleDrawMs = 0;
#endif

  touch.wasPressed = touch.isPressed;
  touch.isPressed = cydTouchRead(p);
  touch.justPressed = touch.isPressed && !touch.wasPressed;
  touch.justReleased = !touch.isPressed && touch.wasPressed;
  
  if (touch.isPressed) {
    int mappedX = constrain(map(p.x, 240, 3800, 0, 320), 0, 319);
    int mappedY = constrain(map(p.y, 200, 3700, 0, 240), 0, 239);
    if (touch.wasPressed) {
      touch.x = (touch.x * 3 + mappedX) / 4;
      touch.y = (touch.y * 3 + mappedY) / 4;
    } else {
      touch.x = mappedX;
      touch.y = mappedY;
    }
    touch.rawX = p.x;
    touch.rawY = p.y;
    touch.rawZ = p.z;

#ifdef DEBUG_TOUCH
    if (currentMode == MENU && millis() - lastTouchLogMs > 120) {
      tft.fillRect(0, 224, 190, 16, THEME_BG);
      tft.setTextColor(THEME_SUCCESS, THEME_BG);
      tft.drawString("T:" + String(touch.x) + "," + String(touch.y) + " z" + String(touch.rawZ), 4, 226, 1);
      tft.fillCircle(touch.x, touch.y, 3, THEME_SUCCESS);
      Serial.printf("touch raw x=%d y=%d z=%d mapped x=%d y=%d\n", touch.rawX, touch.rawY, touch.rawZ, touch.x, touch.y);
      lastTouchLogMs = millis();
    }
#endif
#ifdef DEBUG_TOUCH
  } else if (currentMode == MENU && millis() - lastIdleDrawMs > 1000) {
    tft.fillRect(0, 224, 190, 16, THEME_BG);
    tft.setTextColor(THEME_TEXT_DIM, THEME_BG);
    tft.drawString("T:--", 4, 226, 1);
    lastIdleDrawMs = millis();
#endif
  }
}

bool isButtonPressed(int x, int y, int w, int h) {
  return touch.x >= x && touch.x <= x + w && touch.y >= y && touch.y <= y + h;
}

void drawRoundButton(int x, int y, int w, int h, String text, uint16_t color, bool pressed) {
  uint16_t bgColor = pressed ? color : THEME_PANEL;
  uint16_t borderColor = color;
  uint16_t textColor = pressed ? THEME_BG : THEME_TEXT;
  
  tft.fillRoundRect(x, y, w, h, 6, bgColor);
  tft.drawRoundRect(x, y, w, h, 6, borderColor);
  
  tft.setTextColor(textColor, bgColor);
  tft.drawCentreString(text, x + w/2, y + h/2 - 7, 2);
}

void drawHeader(String title, String subtitle) {
  tft.fillRect(0, 0, 320, 42, THEME_SURFACE);
  tft.drawFastHLine(0, 42, 320, THEME_BORDER);
  
  tft.setTextColor(THEME_TEXT, THEME_SURFACE);
  tft.drawCentreString(title, 160, 6, 4);
  
  if (subtitle.length() > 0) {
    tft.setTextColor(THEME_TEXT_DIM, THEME_SURFACE);
    tft.drawCentreString(subtitle, 160, 27, 2);
  }
  
  drawRoundButton(8, 8, 46, 26, "BACK", THEME_ERROR);
}

void updateStatus() {
  // Status bar removed - no more BLE connection alerts on every screen
}

#endif
