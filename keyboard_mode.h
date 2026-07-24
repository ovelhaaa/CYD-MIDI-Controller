#ifndef KEYBOARD_MODE_H
#define KEYBOARD_MODE_H

#include "common_definitions.h"
#include "ui_elements.h"
#include "midi_utils.h"

// Keyboard mode variables
#define NUM_KEYS 10  // More keys per row
#define NUM_ROWS 2   // Two rows
#define KEYBOARD_KEY_Y 52
#define KEYBOARD_KEY_H 56
#define KEYBOARD_KEY_GAP 4
#define KEYBOARD_CTRL_Y 178
#define KEYBOARD_CTRL_H 30
int keyboardOctave = 4;
int keyboardScale = 0;
int keyboardKey = 0;  // Key signature (C=0, C#=1, D=2, etc.)
int lastKey = -1;
int lastRow = -1;

// Function declarations
void initializeKeyboardMode();
void drawKeyboardMode();
void handleKeyboardMode();
void drawKeyboardKey(int row, int keyIndex, bool pressed);
void playKeyboardNote(int row, int keyIndex, bool on);

// Implementations
void initializeKeyboardMode() {
  keyboardOctave = 4;
  keyboardScale = 0;
  keyboardKey = 0;
  lastKey = -1;
  lastRow = -1;
}

void drawKeyboardMode() {
  tft.fillScreen(THEME_BG);
  drawHeader("KEYS", scales[keyboardScale].name + " Key " + getNoteNameFromMIDI(keyboardKey));
  
  // Draw keys - two rows
  for (int row = 0; row < NUM_ROWS; row++) {
    for (int i = 0; i < NUM_KEYS; i++) {
      drawKeyboardKey(row, i, false);
    }
  }
  
  // Control layout
  tft.fillRect(0, 170, 320, 70, THEME_BG);
  drawRoundButton(8, KEYBOARD_CTRL_Y, 46, KEYBOARD_CTRL_H, "OCT-", THEME_SECONDARY);
  drawRoundButton(60, KEYBOARD_CTRL_Y, 46, KEYBOARD_CTRL_H, "OCT+", THEME_SECONDARY);
  drawRoundButton(116, KEYBOARD_CTRL_Y, 66, KEYBOARD_CTRL_H, "SCALE", THEME_ACCENT);
  drawRoundButton(190, KEYBOARD_CTRL_Y, 46, KEYBOARD_CTRL_H, "KEY-", THEME_WARNING);
  drawRoundButton(242, KEYBOARD_CTRL_Y, 46, KEYBOARD_CTRL_H, "KEY+", THEME_WARNING);
  
  // Status display
  tft.fillRoundRect(8, 214, 304, 18, 4, THEME_PANEL);
  tft.setTextColor(THEME_TEXT_DIM, THEME_PANEL);
  tft.drawCentreString("Oct " + String(keyboardOctave) + "  " +
               scales[keyboardScale].name + "  Root " + getNoteNameFromMIDI(keyboardKey), 160, 217, 1);
}

void drawKeyboardKey(int row, int keyIndex, bool pressed) {
  int keyWidth = 320 / NUM_KEYS;
  int keyHeight = KEYBOARD_KEY_H;
  int keyY = KEYBOARD_KEY_Y + (row * (keyHeight + KEYBOARD_KEY_GAP));
  int x = keyIndex * keyWidth;
  
  uint16_t bgColor = pressed ? THEME_PRIMARY : (row == 0 ? THEME_SURFACE : THEME_PANEL);
  uint16_t borderColor = pressed ? THEME_TEXT : THEME_BORDER;
  uint16_t textColor = pressed ? THEME_BG : THEME_TEXT;
  
  tft.fillRoundRect(x + 1, keyY + 1, keyWidth - 3, keyHeight - 2, 4, bgColor);
  tft.drawRoundRect(x + 1, keyY + 1, keyWidth - 3, keyHeight - 2, 4, borderColor);
  
  // Row 0 = base octave, Row 1 = octave higher
  // Apply key signature transpose
  int note = getNoteInScale(keyboardScale, keyIndex, keyboardOctave + row) + keyboardKey;
  String noteName = getNoteNameFromMIDI(note);
  
  tft.setTextColor(textColor, bgColor);
  tft.drawCentreString(noteName, x + keyWidth/2, keyY + 19, 2);
  tft.setTextColor(pressed ? THEME_BG : THEME_TEXT_DIM, bgColor);
  tft.drawCentreString(row == 0 ? "LOW" : "HIGH", x + keyWidth/2, keyY + 39, 1);
}

void handleKeyboardMode() {
  if (touch.justPressed && isButtonPressed(10, 10, 50, 25)) {
    exitToMenu();
    return;
  }
  
  if (touch.justPressed) {
    if (isButtonPressed(8, KEYBOARD_CTRL_Y, 46, KEYBOARD_CTRL_H)) {
      keyboardOctave = max(1, keyboardOctave - 1);
      drawKeyboardMode();
      return;
    }
    if (isButtonPressed(60, KEYBOARD_CTRL_Y, 46, KEYBOARD_CTRL_H)) {
      keyboardOctave = min(8, keyboardOctave + 1);
      drawKeyboardMode();
      return;
    }
    if (isButtonPressed(116, KEYBOARD_CTRL_Y, 66, KEYBOARD_CTRL_H)) {
      keyboardScale = (keyboardScale + 1) % NUM_SCALES;
      drawKeyboardMode();
      return;
    }
    if (isButtonPressed(190, KEYBOARD_CTRL_Y, 46, KEYBOARD_CTRL_H)) {
      keyboardKey = (keyboardKey - 1 + 12) % 12;
      drawKeyboardMode();
      return;
    }
    if (isButtonPressed(242, KEYBOARD_CTRL_Y, 46, KEYBOARD_CTRL_H)) {
      keyboardKey = (keyboardKey + 1) % 12;
      drawKeyboardMode();
      return;
    }
  }
  
  // Key sliding support - handle two rows
  int key = -1;
  int row = -1;
  
  // Check which key and row is being touched
  if (touch.isPressed) {
    int keyWidth = 320 / NUM_KEYS;
    int keyHeight = KEYBOARD_KEY_H;
    
    for (int r = 0; r < NUM_ROWS; r++) {
      int keyY = KEYBOARD_KEY_Y + (r * (keyHeight + KEYBOARD_KEY_GAP));
      if (touch.y >= keyY && touch.y < keyY + keyHeight) {
        row = r;
        key = touch.x / keyWidth;
        if (key >= NUM_KEYS) key = NUM_KEYS - 1;
        break;
      }
    }
  }
  
  if (touch.isPressed && key != -1 && row != -1) {
    if (key != lastKey || row != lastRow) {
      if (lastKey != -1 && lastRow != -1) {
        playKeyboardNote(lastRow, lastKey, false);
        drawKeyboardKey(lastRow, lastKey, false);
      }
      playKeyboardNote(row, key, true);
      drawKeyboardKey(row, key, true);
      lastKey = key;
      lastRow = row;
    }
  } else if (touch.justReleased && lastKey != -1 && lastRow != -1) {
    playKeyboardNote(lastRow, lastKey, false);
    drawKeyboardKey(lastRow, lastKey, false);
    lastKey = -1;
    lastRow = -1;
  }
}

void playKeyboardNote(int row, int keyIndex, bool on) {
  if (!deviceConnected) return;
  
  int note = getNoteInScale(keyboardScale, keyIndex, keyboardOctave + row) + keyboardKey;
  sendMIDI(on ? 0x90 : 0x80, note, on ? 100 : 0);
  
  Serial.printf("Key R%d:%d: %s %s\n", row, keyIndex, getNoteNameFromMIDI(note).c_str(), on ? "ON" : "OFF");
}

#endif
