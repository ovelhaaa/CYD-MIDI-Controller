#ifndef KEYBOARD_MODE_H
#define KEYBOARD_MODE_H

#include "common_definitions.h"
#include "ui_elements.h"
#include "midi_utils.h"

// Keyboard mode variables
#define NUM_KEYS 10
#define NUM_ROWS 2
#define KEYBOARD_KEY_Y 52
#define KEYBOARD_KEY_H 56
#define KEYBOARD_KEY_GAP 4
#define KEYBOARD_CTRL_Y 174
#define KEYBOARD_MAX_NOTES 4
#define KEYBOARD_ASSIST_MODES 4

int keyboardOctave = 4;
int keyboardAssistMode = 0; // 0=single, 1=fifth, 2=octave, 3=triad
bool keyboardHold = false;
bool keyboardHeld[NUM_ROWS][NUM_KEYS];
int keyboardActiveNotes[NUM_ROWS][NUM_KEYS][KEYBOARD_MAX_NOTES];
int keyboardLastVelocity = 100;
int lastKey = -1;
int lastRow = -1;

const char* keyboardAssistNames[KEYBOARD_ASSIST_MODES] = {"1", "5TH", "OCT", "TRI"};

// Function declarations
void initializeKeyboardMode();
void drawKeyboardMode();
void handleKeyboardMode();
void drawKeyboardKey(int row, int keyIndex, bool pressed = false);
void drawKeyboardControls();
void drawKeyboardStatus();
void drawKeyboardButton(int x, int y, int w, int h, String text, uint16_t color, bool pressed = false);
void playKeyboardPad(int row, int keyIndex, bool on, int velocity = 100);
void buildKeyboardNotes(int row, int keyIndex, int notes[], int &numNotes);
void stopKeyboardPad(int row, int keyIndex);
void stopKeyboardPressedPad();
void stopKeyboardHeldPads();
void toggleKeyboardHeldPad(int row, int keyIndex, int velocity);
bool findKeyboardTouchCell(int &row, int &key);
int getKeyboardVelocity(int row);
String getKeyboardPadLabel(int row, int keyIndex);

// Implementations
void initializeKeyboardMode() {
  keyboardOctave = 4;
  keyboardAssistMode = 0;
  keyboardHold = false;
  keyboardLastVelocity = 100;
  lastKey = -1;
  lastRow = -1;
  for (int row = 0; row < NUM_ROWS; row++) {
    for (int key = 0; key < NUM_KEYS; key++) {
      keyboardHeld[row][key] = false;
      for (int i = 0; i < KEYBOARD_MAX_NOTES; i++) {
        keyboardActiveNotes[row][key][i] = -1;
      }
    }
  }
}

void drawKeyboardMode() {
  tft.fillScreen(THEME_BG);
  drawHeader("KEYS", getRootName() + " " + scales[performance.scale].name + "  " + keyboardAssistNames[keyboardAssistMode]);

  for (int row = 0; row < NUM_ROWS; row++) {
    for (int i = 0; i < NUM_KEYS; i++) {
      drawKeyboardKey(row, i);
    }
  }

  drawKeyboardControls();
  drawKeyboardStatus();
}

void drawKeyboardButton(int x, int y, int w, int h, String text, uint16_t color, bool pressed) {
  uint16_t bgColor = pressed ? color : THEME_PANEL;
  uint16_t textColor = pressed ? THEME_BG : THEME_TEXT;

  if (!pressed) {
    tft.fillRoundRect(x + 1, y + 2, w, h, 4, THEME_BG);
  }
  tft.fillRoundRect(x, y + (pressed ? 1 : 0), w, h, 4, bgColor);
  tft.drawRoundRect(x, y + (pressed ? 1 : 0), w, h, 4, color);
  tft.setTextColor(textColor, bgColor);
  tft.drawCentreString(text, x + w / 2, y + h / 2 - 4 + (pressed ? 1 : 0), 1);
}

void drawKeyboardControls() {
  tft.fillRect(0, 170, 320, 70, THEME_BG);

  drawKeyboardButton(8, KEYBOARD_CTRL_Y, 30, 18, "O-", THEME_SECONDARY);
  drawKeyboardButton(44, KEYBOARD_CTRL_Y, 30, 18, "O+", THEME_SECONDARY);
  drawKeyboardButton(82, KEYBOARD_CTRL_Y, 44, 18, keyboardAssistNames[keyboardAssistMode], THEME_ACCENT);
  drawKeyboardButton(134, KEYBOARD_CTRL_Y, 44, 18, keyboardHold ? "HOLD" : "TAP",
                     keyboardHold ? THEME_PRIMARY : THEME_SECONDARY);
  drawKeyboardButton(186, KEYBOARD_CTRL_Y, 34, 18, "K-", THEME_WARNING);
  drawKeyboardButton(228, KEYBOARD_CTRL_Y, 34, 18, "K+", THEME_WARNING);
  drawKeyboardButton(270, KEYBOARD_CTRL_Y, 42, 18, "SCALE", THEME_ACCENT);
}

void drawKeyboardStatus() {
  tft.fillRoundRect(8, 204, 304, 28, 5, THEME_PANEL);
  tft.drawRoundRect(8, 204, 304, 28, 5, THEME_BORDER);

  String status = "Oct " + String(keyboardOctave) + "  Vel " + String(keyboardLastVelocity) +
                  "  " + keyboardAssistNames[keyboardAssistMode] + "  " +
                  getRootName() + " " + scales[performance.scale].name;
  if (lastKey != -1 && lastRow != -1) {
    status = getKeyboardPadLabel(lastRow, lastKey) + "  Vel " + String(keyboardLastVelocity);
  }

  tft.setTextColor(THEME_TEXT_DIM, THEME_PANEL);
  tft.drawCentreString(status, 160, 214, 1);
}

void drawKeyboardKey(int row, int keyIndex, bool pressed) {
  int keyWidth = 320 / NUM_KEYS;
  int keyHeight = KEYBOARD_KEY_H;
  int keyY = KEYBOARD_KEY_Y + (row * (keyHeight + KEYBOARD_KEY_GAP));
  int x = keyIndex * keyWidth;
  bool held = keyboardHeld[row][keyIndex];

  uint16_t bgColor;
  uint16_t borderColor;
  uint16_t textColor;

  if (pressed) {
    bgColor = THEME_PRIMARY;
    borderColor = THEME_TEXT;
    textColor = THEME_BG;
  } else if (held) {
    bgColor = THEME_SUCCESS;
    borderColor = THEME_TEXT;
    textColor = THEME_BG;
  } else {
    bgColor = row == 0 ? THEME_SURFACE : THEME_PANEL;
    borderColor = THEME_BORDER;
    textColor = THEME_TEXT;
  }

  tft.fillRoundRect(x + 1, keyY + 1, keyWidth - 3, keyHeight - 2, 4, bgColor);
  tft.drawRoundRect(x + 1, keyY + 1, keyWidth - 3, keyHeight - 2, 4, borderColor);

  tft.setTextColor(textColor, bgColor);
  tft.drawCentreString(getKeyboardPadLabel(row, keyIndex), x + keyWidth / 2, keyY + 15, 2);
  tft.setTextColor(pressed || held ? THEME_BG : THEME_TEXT_DIM, bgColor);
  tft.drawCentreString(row == 0 ? "LOW" : "HIGH", x + keyWidth / 2, keyY + 38, 1);
}

void handleKeyboardMode() {
  if (touch.justPressed && isButtonPressed(10, 10, 50, 25)) {
    stopKeyboardPressedPad();
    stopKeyboardHeldPads();
    exitToMenu();
    return;
  }

  if (touch.justPressed) {
    if (isButtonPressed(8, KEYBOARD_CTRL_Y, 30, 18)) {
      stopKeyboardPressedPad();
      stopKeyboardHeldPads();
      keyboardOctave = max(1, keyboardOctave - 1);
      drawKeyboardMode();
      return;
    }
    if (isButtonPressed(44, KEYBOARD_CTRL_Y, 30, 18)) {
      stopKeyboardPressedPad();
      stopKeyboardHeldPads();
      keyboardOctave = min(8, keyboardOctave + 1);
      drawKeyboardMode();
      return;
    }
    if (isButtonPressed(82, KEYBOARD_CTRL_Y, 44, 18)) {
      stopKeyboardPressedPad();
      stopKeyboardHeldPads();
      keyboardAssistMode = (keyboardAssistMode + 1) % KEYBOARD_ASSIST_MODES;
      drawKeyboardMode();
      return;
    }
    if (isButtonPressed(134, KEYBOARD_CTRL_Y, 44, 18)) {
      stopKeyboardPressedPad();
      stopKeyboardHeldPads();
      keyboardHold = !keyboardHold;
      drawKeyboardMode();
      return;
    }
    if (isButtonPressed(186, KEYBOARD_CTRL_Y, 34, 18)) {
      stopKeyboardPressedPad();
      stopKeyboardHeldPads();
      nudgeGlobalRoot(-1);
      drawKeyboardMode();
      return;
    }
    if (isButtonPressed(228, KEYBOARD_CTRL_Y, 34, 18)) {
      stopKeyboardPressedPad();
      stopKeyboardHeldPads();
      nudgeGlobalRoot(1);
      drawKeyboardMode();
      return;
    }
    if (isButtonPressed(270, KEYBOARD_CTRL_Y, 42, 18)) {
      stopKeyboardPressedPad();
      stopKeyboardHeldPads();
      nudgeGlobalScale(1);
      drawKeyboardMode();
      return;
    }
  }

  int key = -1;
  int row = -1;
  bool hasKey = findKeyboardTouchCell(row, key);

  if (keyboardHold) {
    if (touch.justPressed && hasKey) {
      toggleKeyboardHeldPad(row, key, getKeyboardVelocity(row));
      drawKeyboardKey(row, key);
      drawKeyboardStatus();
    }
    return;
  }

  if (touch.isPressed && hasKey) {
    if (key != lastKey || row != lastRow) {
      stopKeyboardPressedPad();
      keyboardLastVelocity = getKeyboardVelocity(row);
      playKeyboardPad(row, key, true, keyboardLastVelocity);
      drawKeyboardKey(row, key, true);
      lastKey = key;
      lastRow = row;
      drawKeyboardStatus();
    }
  } else if (touch.justReleased && lastKey != -1 && lastRow != -1) {
    stopKeyboardPressedPad();
    drawKeyboardStatus();
  }
}

void playKeyboardPad(int row, int keyIndex, bool on, int velocity) {
  if (!on) {
    stopKeyboardPad(row, keyIndex);
    return;
  }

  int notes[KEYBOARD_MAX_NOTES];
  int numNotes = 0;
  buildKeyboardNotes(row, keyIndex, notes, numNotes);

  for (int i = 0; i < numNotes; i++) {
    if (notes[i] >= 0 && notes[i] <= 127) {
      sendNote(performance.keysChannel, notes[i], velocity, true);
      keyboardActiveNotes[row][keyIndex][i] = notes[i];
    }
  }

  Serial.printf("Key R%d:%d: %s vel %d\n", row, keyIndex, getNoteNameFromMIDI(notes[0]).c_str(), velocity);
}

void buildKeyboardNotes(int row, int keyIndex, int notes[], int &numNotes) {
  int octave = keyboardOctave + row;
  int rootNote = getNoteInScale(performance.scale, keyIndex, octave);
  numNotes = 0;
  notes[numNotes++] = rootNote;

  switch (keyboardAssistMode) {
    case 1: // Fifth
      notes[numNotes++] = getNoteInScale(performance.scale, keyIndex + 4, octave);
      break;
    case 2: // Octave
      notes[numNotes++] = rootNote + 12;
      break;
    case 3: // Diatonic triad
      notes[numNotes++] = getNoteInScale(performance.scale, keyIndex + 2, octave);
      notes[numNotes++] = getNoteInScale(performance.scale, keyIndex + 4, octave);
      break;
  }
}

void stopKeyboardPad(int row, int keyIndex) {
  for (int i = 0; i < KEYBOARD_MAX_NOTES; i++) {
    if (keyboardActiveNotes[row][keyIndex][i] != -1) {
      sendNote(performance.keysChannel, keyboardActiveNotes[row][keyIndex][i], 0, false);
      keyboardActiveNotes[row][keyIndex][i] = -1;
    }
  }
}

void stopKeyboardPressedPad() {
  if (lastKey != -1 && lastRow != -1) {
    stopKeyboardPad(lastRow, lastKey);
    drawKeyboardKey(lastRow, lastKey, false);
    lastKey = -1;
    lastRow = -1;
  }
}

void stopKeyboardHeldPads() {
  for (int row = 0; row < NUM_ROWS; row++) {
    for (int key = 0; key < NUM_KEYS; key++) {
      if (keyboardHeld[row][key]) {
        stopKeyboardPad(row, key);
        keyboardHeld[row][key] = false;
      }
    }
  }
}

void toggleKeyboardHeldPad(int row, int keyIndex, int velocity) {
  if (keyboardHeld[row][keyIndex]) {
    stopKeyboardPad(row, keyIndex);
    keyboardHeld[row][keyIndex] = false;
    lastKey = -1;
    lastRow = -1;
  } else {
    keyboardLastVelocity = velocity;
    playKeyboardPad(row, keyIndex, true, velocity);
    keyboardHeld[row][keyIndex] = true;
    lastKey = keyIndex;
    lastRow = row;
  }
}

bool findKeyboardTouchCell(int &row, int &key) {
  if (!touch.isPressed) return false;

  int keyWidth = 320 / NUM_KEYS;
  for (int r = 0; r < NUM_ROWS; r++) {
    int keyY = KEYBOARD_KEY_Y + (r * (KEYBOARD_KEY_H + KEYBOARD_KEY_GAP));
    if (touch.y >= keyY && touch.y < keyY + KEYBOARD_KEY_H) {
      row = r;
      key = touch.x / keyWidth;
      if (key >= NUM_KEYS) key = NUM_KEYS - 1;
      return true;
    }
  }
  return false;
}

int getKeyboardVelocity(int row) {
  int keyY = KEYBOARD_KEY_Y + (row * (KEYBOARD_KEY_H + KEYBOARD_KEY_GAP));
  int offset = constrain(touch.y - keyY, 0, KEYBOARD_KEY_H - 1);
  return map(offset, 0, KEYBOARD_KEY_H - 1, 62, 122);
}

String getKeyboardPadLabel(int row, int keyIndex) {
  int note = getNoteInScale(performance.scale, keyIndex, keyboardOctave + row);
  return getNoteNameFromMIDI(note);
}

#endif
