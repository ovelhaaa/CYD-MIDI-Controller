#ifndef LFO_MODE_H
#define LFO_MODE_H

#include "common_definitions.h"
#include "ui_elements.h"
#include "midi_utils.h"

// LFO mode variables
struct LFOParams {
  float rate = 1.0;      // Hz (0.1 - 10.0)
  int amount = 64;       // 0-127
  int ccTarget = 1;      // CC number (0-127) or -1 for pitchwheel
  bool isRunning = false;
  float phase = 0.0;     // Current phase (0-2π)
  int waveform = 0;      // 0=Sine, 1=Triangle, 2=Square, 3=Sawtooth
  unsigned long lastUpdate = 0;
  int lastValue = 64;    // Last sent value
  bool pitchWheelMode = false; // Special mode for pitchwheel
};

LFOParams lfo;
String waveNames[] = {"SINE", "TRI", "SQR", "SAW"};

// Function declarations
void initializeLFOMode();
void drawLFOMode();
void handleLFOMode();
void drawLFOControls();
void updateLFO();
float calculateLFOValue();
void sendLFOValue(int value);
void drawWaveform();

// Implementations
void initializeLFOMode() {
  lfo.rate = 1.0;
  lfo.amount = 64;
  lfo.ccTarget = 1; // Modulation wheel by default
  lfo.isRunning = false;
  lfo.phase = 0.0;
  lfo.waveform = 0;
  lfo.lastUpdate = 0;
  lfo.lastValue = 64;
  lfo.pitchWheelMode = false;
}

void drawLFOMode() {
  tft.fillScreen(THEME_BG);
  drawHeader("LFO MOD", lfo.pitchWheelMode ? "Pitchwheel" : ("CC " + String(lfo.ccTarget)));
  
  drawLFOControls();
  drawWaveform();
}

void drawLFOControls() {
  int y = 54;
  int spacing = 36;

  tft.fillRoundRect(6, 50, 308, 118, 6, THEME_PANEL);
  tft.drawRoundRect(6, 50, 308, 118, 6, THEME_BORDER);
  
  // Play/Stop and Rate
  drawRoundButton(14, y + 2, 62, 30, lfo.isRunning ? "STOP" : "START", 
                 lfo.isRunning ? THEME_ERROR : THEME_SUCCESS);
  
  tft.setTextColor(THEME_TEXT_DIM, THEME_PANEL);
  tft.drawString("RATE", 88, y + 4, 1);
  tft.setTextColor(THEME_TEXT, THEME_PANEL);
  tft.drawString(String(lfo.rate, 1) + "Hz", 88, y + 16, 2);
  drawRoundButton(156, y + 2, 30, 30, "-", THEME_SECONDARY);
  drawRoundButton(192, y + 2, 30, 30, "+", THEME_SECONDARY);
  
  // Waveform selector
  drawRoundButton(238, y + 2, 66, 30, waveNames[lfo.waveform], THEME_ACCENT);
  
  y += spacing;
  
  // Amount
  tft.setTextColor(THEME_TEXT_DIM, THEME_PANEL);
  tft.drawString("AMT", 16, y + 4, 1);
  tft.setTextColor(THEME_TEXT, THEME_PANEL);
  tft.drawString(String(lfo.amount), 16, y + 16, 2);
  drawRoundButton(62, y + 2, 30, 30, "-", THEME_SECONDARY);
  drawRoundButton(98, y + 2, 30, 30, "+", THEME_SECONDARY);
  
  // Amount bar
  int barW = 160;
  int barX = 144;
  tft.drawRoundRect(barX, y + 11, barW, 12, 3, THEME_BORDER);
  int fillW = (barW * lfo.amount) / 127;
  tft.fillRoundRect(barX + 1, y + 12, max(1, fillW - 2), 10, 2, THEME_PRIMARY);
  
  y += spacing;
  
  // Target selection
  tft.setTextColor(THEME_TEXT_DIM, THEME_PANEL);
  tft.drawString("TARGET", 16, y + 4, 1);
  if (lfo.pitchWheelMode) {
    tft.setTextColor(THEME_TEXT, THEME_PANEL);
    tft.drawString("PITCH", 16, y + 16, 2);
  } else {
    tft.setTextColor(THEME_TEXT, THEME_PANEL);
    tft.drawString("CC" + String(lfo.ccTarget), 16, y + 16, 2);
  }
  
  drawRoundButton(90, y + 2, 30, 30, "-", THEME_SECONDARY);
  drawRoundButton(126, y + 2, 30, 30, "+", THEME_SECONDARY);
  drawRoundButton(174, y + 2, 76, 30, "PITCH", lfo.pitchWheelMode ? THEME_PRIMARY : THEME_WARNING);
  
  // Current value display
  tft.fillRoundRect(260, y + 2, 44, 30, 5, THEME_BG);
  tft.setTextColor(THEME_ACCENT, THEME_BG);
  tft.drawCentreString(String(lfo.lastValue), 282, y + 9, 2);
  
  // Status indicator
  if (lfo.isRunning) {
    tft.fillCircle(258, y + 17, 4, THEME_SUCCESS);
  } else {
    tft.drawCircle(258, y + 17, 4, THEME_TEXT_DIM);
  }
}

void drawWaveform() {
  // Draw a mini waveform visualization
  int waveX = 10;
  int waveY = 178;
  int waveW = 300;
  int waveH = 48;
  
  tft.fillRoundRect(waveX, waveY, waveW, waveH, 6, THEME_PANEL);
  tft.drawRoundRect(waveX, waveY, waveW, waveH, 6, THEME_BORDER);
  tft.drawFastHLine(waveX + 8, waveY + waveH / 2, waveW - 16, THEME_TEXT_DIM);
  
  // Draw waveform based on type
  for (int x = 0; x < waveW - 2; x++) {
    float phase = (x / (float)(waveW - 2)) * 2 * PI;
    float value = 0;
    
    switch (lfo.waveform) {
      case 0: // Sine
        value = sin(phase);
        break;
      case 1: // Triangle
        value = (phase <= PI) ? (2 * phase / PI - 1) : (3 - 2 * phase / PI);
        break;
      case 2: // Square
        value = (phase <= PI) ? 1 : -1;
        break;
      case 3: // Sawtooth
        value = 2 * phase / (2 * PI) - 1;
        break;
    }
    
    int y = waveY + waveH/2 - (value * waveH/3);
    tft.drawPixel(waveX + 1 + x, y, THEME_PRIMARY);
  }
  
  // Phase indicator removed per user request
}

void handleLFOMode() {
  // Back button
  if (touch.justPressed && isButtonPressed(10, 10, 50, 25)) {
    lfo.isRunning = false;
    exitToMenu();
    return;
  }
  
  if (touch.justPressed) {
    int y = 55;
    int spacing = 36;
    
    // Start/Stop
    if (isButtonPressed(14, y + 2, 62, 30)) {
      lfo.isRunning = !lfo.isRunning;
      if (lfo.isRunning) {
        lfo.phase = 0.0;
        lfo.lastUpdate = millis();
      }
      drawLFOMode();
      return;
    }
    
    // Rate controls
    if (isButtonPressed(156, y + 2, 30, 30)) {
      lfo.rate = max(0.1, lfo.rate - 0.1);
      drawLFOControls();
      return;
    }
    if (isButtonPressed(192, y + 2, 30, 30)) {
      lfo.rate = min(10.0, lfo.rate + 0.1);
      drawLFOControls();
      return;
    }
    
    // Waveform selector
    if (isButtonPressed(238, y + 2, 66, 30)) {
      lfo.waveform = (lfo.waveform + 1) % 4;
      drawLFOMode();
      return;
    }
    
    y += spacing;
    
    // Amount controls
    if (isButtonPressed(62, y + 2, 30, 30)) {
      lfo.amount = max(0, lfo.amount - 5);
      drawLFOControls();
      return;
    }
    if (isButtonPressed(98, y + 2, 30, 30)) {
      lfo.amount = min(127, lfo.amount + 5);
      drawLFOControls();
      return;
    }
    
    y += spacing;
    
    // Target controls
    if (isButtonPressed(90, y + 2, 30, 30)) {
      if (lfo.pitchWheelMode) {
        lfo.pitchWheelMode = false;
        lfo.ccTarget = 1; // Back to modulation wheel
      } else {
        lfo.ccTarget = max(0, lfo.ccTarget - 1);
      }
      drawLFOMode();
      return;
    }
    if (isButtonPressed(126, y + 2, 30, 30)) {
      if (!lfo.pitchWheelMode) {
        lfo.ccTarget = min(127, lfo.ccTarget + 1);
      }
      drawLFOMode();
      return;
    }
    
    // Pitchwheel mode toggle
    if (isButtonPressed(174, y + 2, 76, 30)) {
      lfo.pitchWheelMode = !lfo.pitchWheelMode;
      drawLFOMode();
      return;
    }
  }
  
  // Update LFO
  updateLFO();
}

void updateLFO() {
  if (!lfo.isRunning) return;
  
  unsigned long now = millis();
  float deltaTime = (now - lfo.lastUpdate) / 1000.0; // Convert to seconds
  lfo.lastUpdate = now;
  
  // Update phase
  lfo.phase += 2 * PI * lfo.rate * deltaTime;
  while (lfo.phase >= 2 * PI) {
    lfo.phase -= 2 * PI;
  }
  
  // Calculate LFO value
  float lfoValue = calculateLFOValue();
  
  // Calculate output value based on target type
  int outputValue;
  if (lfo.pitchWheelMode) {
    // For pitch bend: 8192 center, scale by amount
    outputValue = 8192 + (lfoValue * lfo.amount * 64); // Scale for pitch bend range
    outputValue = constrain(outputValue, 0, 16383);
  } else {
    // For CC: 64 center, scale by amount  
    outputValue = 64 + (lfoValue * lfo.amount / 2);
    outputValue = constrain(outputValue, 0, 127);
  }
  
  // Send if value changed significantly (reduce MIDI spam)
  if (abs(outputValue - lfo.lastValue) >= 1) {
    sendLFOValue(outputValue);
    lfo.lastValue = outputValue;
    
    // Update display every few cycles
    static int displayUpdateCounter = 0;
    if (++displayUpdateCounter >= 10) {
      drawLFOControls();
      drawWaveform();
      displayUpdateCounter = 0;
    }
  }
}

float calculateLFOValue() {
  switch (lfo.waveform) {
    case 0: // Sine
      return sin(lfo.phase);
    case 1: // Triangle
      return (lfo.phase <= PI) ? (2 * lfo.phase / PI - 1) : (3 - 2 * lfo.phase / PI);
    case 2: // Square
      return (lfo.phase <= PI) ? 1 : -1;
    case 3: // Sawtooth
      return 2 * lfo.phase / (2 * PI) - 1;
    default:
      return 0;
  }
}

void sendLFOValue(int value) {
  if (!deviceConnected) return;
  
  if (lfo.pitchWheelMode) {
    sendPitchBend(performance.modChannel, value);
  } else {
    // Send regular CC
    sendCC(performance.modChannel, lfo.ccTarget, value);
  }
}

#endif
