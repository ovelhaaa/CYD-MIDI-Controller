#ifndef XY_PAD_MODE_H
#define XY_PAD_MODE_H

#include "common_definitions.h"
#include "ui_elements.h"
#include "midi_utils.h"

// XY Pad mode variables
int xCC = 1;  // CC number for X axis (Modulation Wheel by default)
int yCC = 7;  // CC number for Y axis (Volume by default)
int xValue = 64;  // Current X value (0-127)
int yValue = 64;  // Current Y value (0-127)
bool padPressed = false;
int padX = 0, padY = 0;  // Touch position on pad
bool xyPadNeedsReset = false;  // Flag to reset static variables

// Pad area dimensions
#define PAD_X 10
#define PAD_Y 52
#define PAD_WIDTH 210
#define PAD_HEIGHT 142
#define PAD_CENTER_X (PAD_X + PAD_WIDTH/2)
#define PAD_CENTER_Y (PAD_Y + PAD_HEIGHT/2)

// Function declarations
void initializeXYPadMode();
void drawXYPadMode();
void handleXYPadMode();
void drawXYPad();
void drawCCControls();
void updateXYValues(int touchX, int touchY);
void sendXYValues();

// Implementations
void initializeXYPadMode() {
  xCC = 74;  // Cutoff/Filter Frequency
  yCC = 71;  // Resonance/Filter Q
  xValue = 64;
  yValue = 64;
  padPressed = false;
}

void drawXYPadMode() {
  tft.fillScreen(THEME_BG);
  drawHeader("XY PAD", "Touch Control");
  
  // Signal that static variables should be reset
  xyPadNeedsReset = true;
  
  drawXYPad();
  drawCCControls();
}

void drawXYPad() {
  static int lastIndicatorX = -1, lastIndicatorY = -1;
  static bool lastPadPressed = false;
  static int lastXValue = -1, lastYValue = -1;
  static bool backgroundDrawn = false;
  
  // Reset static variables if requested
  if (xyPadNeedsReset) {
    lastIndicatorX = -1;
    lastIndicatorY = -1;
    lastPadPressed = false;
    lastXValue = -1;
    lastYValue = -1;
    backgroundDrawn = false;
    xyPadNeedsReset = false;
  }
  
  // Always ensure background is drawn properly
  if (!backgroundDrawn || lastIndicatorX == -1) {
    // Draw pad background
    tft.fillRoundRect(PAD_X, PAD_Y, PAD_WIDTH, PAD_HEIGHT, 6, THEME_PANEL);
    tft.drawRoundRect(PAD_X, PAD_Y, PAD_WIDTH, PAD_HEIGHT, 6, THEME_BORDER);
    
    // Draw crosshairs
    tft.drawFastHLine(PAD_X + 6, PAD_CENTER_Y, PAD_WIDTH - 12, THEME_TEXT_DIM);
    tft.drawFastVLine(PAD_CENTER_X, PAD_Y + 6, PAD_HEIGHT - 12, THEME_TEXT_DIM);
    tft.drawCircle(PAD_CENTER_X, PAD_CENTER_Y, 18, THEME_SURFACE);
    
    backgroundDrawn = true;
  }
  
  // Calculate position indicator location
  int indicatorX = map(xValue, 0, 127, PAD_X + 5, PAD_X + PAD_WIDTH - 5);
  int indicatorY = map(yValue, 0, 127, PAD_Y + PAD_HEIGHT - 5, PAD_Y + 5);
  
  // Erase previous indicator if position changed
  if (lastIndicatorX != indicatorX || lastIndicatorY != indicatorY || lastPadPressed != padPressed) {
    if (lastIndicatorX != -1) {
      // Erase old indicator
      tft.fillCircle(lastIndicatorX, lastIndicatorY, 10, THEME_PANEL);
      // Always redraw the full crosshairs after erasing
      tft.drawFastHLine(PAD_X + 6, PAD_CENTER_Y, PAD_WIDTH - 12, THEME_TEXT_DIM);
      tft.drawFastVLine(PAD_CENTER_X, PAD_Y + 6, PAD_HEIGHT - 12, THEME_TEXT_DIM);
      tft.drawCircle(PAD_CENTER_X, PAD_CENTER_Y, 18, THEME_SURFACE);
      // Always redraw the border to prevent edge disappearing
      tft.drawRoundRect(PAD_X, PAD_Y, PAD_WIDTH, PAD_HEIGHT, 6, THEME_BORDER);
    }
    
    // Draw new indicator
    tft.fillCircle(indicatorX, indicatorY, 9, THEME_PRIMARY);
    tft.fillCircle(indicatorX, indicatorY, 5, padPressed ? THEME_ACCENT : THEME_TEXT);
    
    lastIndicatorX = indicatorX;
    lastIndicatorY = indicatorY;
    lastPadPressed = padPressed;
  }
  
  // Update value display only if values changed
  if (lastXValue != xValue || lastYValue != yValue) {
    // Clear previous text
    tft.fillRoundRect(PAD_X, PAD_Y + PAD_HEIGHT + 8, PAD_WIDTH, 26, 4, THEME_PANEL);
    
    // Draw new values
    tft.setTextColor(THEME_PRIMARY, THEME_PANEL);
    tft.drawString("X " + String(xValue), PAD_X + 12, PAD_Y + PAD_HEIGHT + 14, 2);
    tft.setTextColor(THEME_ACCENT, THEME_PANEL);
    tft.drawString("Y " + String(yValue), PAD_X + 118, PAD_Y + PAD_HEIGHT + 14, 2);
    
    lastXValue = xValue;
    lastYValue = yValue;
  }
}

void drawCCControls() {
  // CC assignment controls
  int controlsX = PAD_X + PAD_WIDTH + 12;
  int panelW = 88;

  tft.fillRoundRect(controlsX, PAD_Y, panelW, 180, 6, THEME_PANEL);
  tft.drawRoundRect(controlsX, PAD_Y, panelW, 180, 6, THEME_BORDER);
  
  // X CC controls
  tft.setTextColor(THEME_PRIMARY, THEME_PANEL);
  tft.drawCentreString("X CC", controlsX + panelW / 2, PAD_Y + 10, 2);
  
  drawRoundButton(controlsX + 8, PAD_Y + 34, 32, 28, "-", THEME_SECONDARY);
  drawRoundButton(controlsX + 48, PAD_Y + 34, 32, 28, "+", THEME_SECONDARY);
  
  tft.fillRoundRect(controlsX + 22, PAD_Y + 68, 44, 24, 4, THEME_BG);
  tft.setTextColor(THEME_TEXT, THEME_BG);
  tft.drawCentreString(String(xCC), controlsX + panelW / 2, PAD_Y + 73, 2);
  
  // Y CC controls
  tft.setTextColor(THEME_ACCENT, THEME_PANEL);
  tft.drawCentreString("Y CC", controlsX + panelW / 2, PAD_Y + 98, 2);
  
  drawRoundButton(controlsX + 8, PAD_Y + 122, 32, 28, "-", THEME_SECONDARY);
  drawRoundButton(controlsX + 48, PAD_Y + 122, 32, 28, "+", THEME_SECONDARY);
  
  tft.fillRoundRect(controlsX + 22, PAD_Y + 156, 44, 24, 4, THEME_BG);
  tft.setTextColor(THEME_TEXT, THEME_BG);
  tft.drawCentreString(String(yCC), controlsX + panelW / 2, PAD_Y + 161, 2);
  
  // Reset button removed per user request
}

void handleXYPadMode() {
  // Back button
  if (touch.justPressed && isButtonPressed(10, 10, 50, 25)) {
    exitToMenu();
    return;
  }
  
  if (touch.isPressed) {
    // Check if touching the pad
    if (touch.x >= PAD_X && touch.x <= PAD_X + PAD_WIDTH &&
        touch.y >= PAD_Y && touch.y <= PAD_Y + PAD_HEIGHT) {
      padPressed = true;
      updateXYValues(touch.x, touch.y);
      sendXYValues();
      drawXYPad();  // Update position indicator
      return;
    }
  } else {
    if (padPressed) {
      padPressed = false;
      drawXYPad();  // Update indicator appearance
    }
  }
  
  if (touch.justPressed) {
    int controlsX = PAD_X + PAD_WIDTH + 12;
    
    // X CC controls
    if (isButtonPressed(controlsX + 8, PAD_Y + 34, 32, 28)) {
      xCC = max(0, xCC - 1);
      drawCCControls();
      return;
    }
    if (isButtonPressed(controlsX + 48, PAD_Y + 34, 32, 28)) {
      xCC = min(127, xCC + 1);
      drawCCControls();
      return;
    }
    
    // Y CC controls
    if (isButtonPressed(controlsX + 8, PAD_Y + 122, 32, 28)) {
      yCC = max(0, yCC - 1);
      drawCCControls();
      return;
    }
    if (isButtonPressed(controlsX + 48, PAD_Y + 122, 32, 28)) {
      yCC = min(127, yCC + 1);
      drawCCControls();
      return;
    }
    
    // Reset button removed
  }
}

void updateXYValues(int touchX, int touchY) {
  // Constrain touch coordinates to pad area first
  touchX = constrain(touchX, PAD_X, PAD_X + PAD_WIDTH);
  touchY = constrain(touchY, PAD_Y, PAD_Y + PAD_HEIGHT);
  
  // Map touch coordinates to CC values
  xValue = map(touchX, PAD_X, PAD_X + PAD_WIDTH, 0, 127);
  yValue = map(touchY, PAD_Y + PAD_HEIGHT, PAD_Y, 0, 127);  // Invert Y axis
  
  // Constrain values
  xValue = constrain(xValue, 0, 127);
  yValue = constrain(yValue, 0, 127);
}

void sendXYValues() {
  if (deviceConnected) {
    // Send X CC
    sendCC(performance.modChannel, xCC, xValue);
    // Send Y CC
    sendCC(performance.modChannel, yCC, yValue);
  }
}

// Reset function removed - using global flag instead

#endif
