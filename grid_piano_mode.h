#ifndef GRID_PIANO_MODE_H
#define GRID_PIANO_MODE_H

#include "common_definitions.h"
#include "ui_elements.h"
#include "midi_utils.h"

// Grid Piano mode variables
#define GRID_COLS 8
#define GRID_ROWS 5
#define GRID_CELL_W 35
#define GRID_CELL_H 24
#define GRID_START_X 13
#define GRID_START_Y 58
#define GRID_SPACING 3
#define GRID_LAYOUTS 3

int gridOctave = 3;
int gridPressedNote = -1;
int gridPressedRow = -1;
int gridPressedCol = -1;
int gridLastVelocity = 100;
int gridLayoutMode = 0; // 0=4ths, 1=3rds, 2=scale sequence
bool gridScaleLock = false;
bool gridHold = false;
bool gridHeld[GRID_ROWS][GRID_COLS];
int gridLayout[GRID_ROWS][GRID_COLS];

const char* gridLayoutNames[GRID_LAYOUTS] = {"4TH", "3RD", "SEQ"};

// Function declarations
void initializeGridPianoMode();
void drawGridPianoMode();
void handleGridPianoMode();
void drawGridCell(int row, int col, bool pressed = false);
void drawGridControls();
void drawGridStatus();
void drawGridButton(int x, int y, int w, int h, String text, uint16_t color, bool pressed = false);
void calculateGridLayout();
void stopGridHeldNotes();
void stopGridPressedNote();
void toggleGridHeldCell(int row, int col, int velocity);
bool findGridCellAtTouch(int &row, int &col);
bool isGridNoteInScale(int note);
bool isGridCellPlayable(int row, int col);
int getGridNote(int row, int col);
int getGridVelocity(int row);
String getGridCellLabel(int note);

// Implementations
void initializeGridPianoMode() {
  gridOctave = 3;
  gridPressedNote = -1;
  gridPressedRow = -1;
  gridPressedCol = -1;
  gridLastVelocity = 100;
  gridLayoutMode = 0;
  gridScaleLock = false;
  gridHold = false;
  for (int row = 0; row < GRID_ROWS; row++) {
    for (int col = 0; col < GRID_COLS; col++) {
      gridHeld[row][col] = false;
    }
  }
  calculateGridLayout();
}

void calculateGridLayout() {
  int rowInterval = gridLayoutMode == 1 ? 4 : 5;
  int baseNote = 36 + performance.root + (gridOctave - 3) * 12; // Root at octave 3 by default

  for (int row = 0; row < GRID_ROWS; row++) {
    for (int col = 0; col < GRID_COLS; col++) {
      int invertedRow = GRID_ROWS - 1 - row;
      if (gridLayoutMode == 2) {
        int degree = col + invertedRow * GRID_COLS;
        gridLayout[row][col] = getNoteInScale(performance.scale, degree, gridOctave);
      } else {
        gridLayout[row][col] = baseNote + col + invertedRow * rowInterval;
      }
    }
  }
}

void drawGridPianoMode() {
  calculateGridLayout();
  tft.fillScreen(THEME_BG);
  drawHeader("GRID", getRootName() + " " + scales[performance.scale].name + "  " + gridLayoutNames[gridLayoutMode]);

  tft.fillRoundRect(6, 50, 308, 144, 6, THEME_PANEL);
  tft.drawRoundRect(6, 50, 308, 144, 6, THEME_BORDER);

  for (int row = 0; row < GRID_ROWS; row++) {
    for (int col = 0; col < GRID_COLS; col++) {
      drawGridCell(row, col);
    }
  }

  drawGridControls();
  drawGridStatus();
}

void drawGridButton(int x, int y, int w, int h, String text, uint16_t color, bool pressed) {
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

void drawGridControls() {
  tft.fillRect(0, 198, 320, 22, THEME_BG);
  drawGridButton(8, 200, 34, 18, "O-", THEME_SECONDARY);
  drawGridButton(48, 200, 34, 18, "O+", THEME_SECONDARY);
  drawGridButton(90, 200, 42, 18, gridLayoutNames[gridLayoutMode], THEME_ACCENT);
  drawGridButton(140, 200, 46, 18, gridScaleLock ? "LOCK" : "FREE",
                 gridScaleLock ? THEME_SUCCESS : THEME_SECONDARY);
  drawGridButton(194, 200, 46, 18, gridHold ? "HOLD" : "TAP",
                 gridHold ? THEME_PRIMARY : THEME_SECONDARY);
  drawGridButton(248, 200, 54, 18, "SCALE", THEME_WARNING);
}

void drawGridStatus() {
  tft.fillRoundRect(8, 222, 304, 16, 4, THEME_PANEL);
  tft.drawRoundRect(8, 222, 304, 16, 4, THEME_BORDER);

  String status = "Oct " + String(gridOctave) + "  Vel " + String(gridLastVelocity) + "  ";
  if (gridPressedNote != -1) {
    status += getNoteNameFromMIDI(gridPressedNote);
  } else {
    status += gridHold ? "Latch ready" : "Ready";
  }

  tft.setTextColor(THEME_TEXT_DIM, THEME_PANEL);
  tft.drawCentreString(status, 160, 226, 1);
}

void drawGridCell(int row, int col, bool pressed) {
  int x = GRID_START_X + col * (GRID_CELL_W + GRID_SPACING);
  int y = GRID_START_Y + row * (GRID_CELL_H + GRID_SPACING);
  int note = gridLayout[row][col];
  int pitchClass = (note % 12 + 12) % 12;
  bool root = pitchClass == performance.root;
  bool inScale = isGridNoteInScale(note);
  bool playable = isGridCellPlayable(row, col);
  bool held = gridHeld[row][col];

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
  } else if (root) {
    bgColor = THEME_ACCENT;
    borderColor = THEME_TEXT;
    textColor = THEME_BG;
  } else if (inScale) {
    bgColor = THEME_SURFACE;
    borderColor = THEME_PRIMARY;
    textColor = THEME_TEXT;
  } else if (!playable) {
    bgColor = THEME_BG;
    borderColor = THEME_PANEL;
    textColor = THEME_TEXT_DIM;
  } else {
    bgColor = THEME_BG;
    borderColor = THEME_BORDER;
    textColor = THEME_TEXT_DIM;
  }

  tft.fillRoundRect(x, y, GRID_CELL_W, GRID_CELL_H, 4, bgColor);
  tft.drawRoundRect(x, y, GRID_CELL_W, GRID_CELL_H, 4, borderColor);

  tft.setTextColor(textColor, bgColor);
  tft.drawCentreString(getGridCellLabel(note), x + GRID_CELL_W / 2, y + 5, 1);

  if (gridScaleLock && !inScale && gridLayoutMode != 2) {
    tft.drawFastHLine(x + 9, y + GRID_CELL_H - 5, GRID_CELL_W - 18, THEME_TEXT_DIM);
  }
}

void handleGridPianoMode() {
  if (touch.justPressed && isButtonPressed(10, 10, 50, 25)) {
    stopGridPressedNote();
    stopGridHeldNotes();
    exitToMenu();
    return;
  }

  if (touch.justPressed) {
    if (isButtonPressed(8, 200, 34, 18)) {
      stopGridPressedNote();
      stopGridHeldNotes();
      gridOctave = max(1, gridOctave - 1);
      calculateGridLayout();
      drawGridPianoMode();
      return;
    }
    if (isButtonPressed(48, 200, 34, 18)) {
      stopGridPressedNote();
      stopGridHeldNotes();
      gridOctave = min(6, gridOctave + 1);
      calculateGridLayout();
      drawGridPianoMode();
      return;
    }
    if (isButtonPressed(90, 200, 42, 18)) {
      stopGridPressedNote();
      stopGridHeldNotes();
      gridLayoutMode = (gridLayoutMode + 1) % GRID_LAYOUTS;
      calculateGridLayout();
      drawGridPianoMode();
      return;
    }
    if (isButtonPressed(140, 200, 46, 18)) {
      stopGridPressedNote();
      stopGridHeldNotes();
      gridScaleLock = !gridScaleLock;
      drawGridPianoMode();
      return;
    }
    if (isButtonPressed(194, 200, 46, 18)) {
      stopGridPressedNote();
      stopGridHeldNotes();
      gridHold = !gridHold;
      drawGridPianoMode();
      return;
    }
    if (isButtonPressed(248, 200, 54, 18)) {
      stopGridPressedNote();
      stopGridHeldNotes();
      nudgeGlobalScale(1);
      calculateGridLayout();
      drawGridPianoMode();
      return;
    }
  }

  int row = -1;
  int col = -1;
  bool hasCell = findGridCellAtTouch(row, col);

  if (gridHold) {
    if (touch.justPressed && hasCell && isGridCellPlayable(row, col)) {
      toggleGridHeldCell(row, col, getGridVelocity(row));
      drawGridCell(row, col);
      drawGridStatus();
    }
    return;
  }

  int pressedNote = -1;
  int pressedRow = -1;
  int pressedCol = -1;
  if (touch.isPressed && hasCell && isGridCellPlayable(row, col)) {
    pressedNote = gridLayout[row][col];
    pressedRow = row;
    pressedCol = col;
  }

  if (pressedNote != gridPressedNote) {
    stopGridPressedNote();

    if (pressedNote != -1 && deviceConnected) {
      gridLastVelocity = getGridVelocity(pressedRow);
      sendNote(performance.keysChannel, pressedNote, gridLastVelocity, true);
      gridPressedNote = pressedNote;
      gridPressedRow = pressedRow;
      gridPressedCol = pressedCol;
      drawGridCell(pressedRow, pressedCol, true);
    }
    drawGridStatus();
  } else if (touch.justReleased && gridPressedNote != -1) {
    stopGridPressedNote();
    drawGridStatus();
  }
}

void stopGridPressedNote() {
  if (gridPressedNote != -1) {
    sendNote(performance.keysChannel, gridPressedNote, 0, false);
    if (gridPressedRow != -1 && gridPressedCol != -1) {
      drawGridCell(gridPressedRow, gridPressedCol, false);
    }
    gridPressedNote = -1;
    gridPressedRow = -1;
    gridPressedCol = -1;
  }
}

void stopGridHeldNotes() {
  for (int row = 0; row < GRID_ROWS; row++) {
    for (int col = 0; col < GRID_COLS; col++) {
      if (gridHeld[row][col]) {
        sendNote(performance.keysChannel, gridLayout[row][col], 0, false);
        gridHeld[row][col] = false;
      }
    }
  }
}

void toggleGridHeldCell(int row, int col, int velocity) {
  int note = gridLayout[row][col];
  if (gridHeld[row][col]) {
    sendNote(performance.keysChannel, note, 0, false);
    gridHeld[row][col] = false;
    gridPressedNote = -1;
  } else {
    gridLastVelocity = velocity;
    sendNote(performance.keysChannel, note, velocity, true);
    gridHeld[row][col] = true;
    gridPressedNote = note;
  }
  gridPressedRow = row;
  gridPressedCol = col;
}

bool findGridCellAtTouch(int &row, int &col) {
  if (!touch.isPressed) return false;

  for (int r = 0; r < GRID_ROWS; r++) {
    for (int c = 0; c < GRID_COLS; c++) {
      int x = GRID_START_X + c * (GRID_CELL_W + GRID_SPACING);
      int y = GRID_START_Y + r * (GRID_CELL_H + GRID_SPACING);
      if (isButtonPressed(x, y, GRID_CELL_W, GRID_CELL_H)) {
        row = r;
        col = c;
        return true;
      }
    }
  }
  return false;
}

bool isGridNoteInScale(int note) {
  int pitch = ((note % 12) - performance.root + 12) % 12;
  Scale& scale = scales[performance.scale];
  for (int i = 0; i < scale.numNotes; i++) {
    if (scale.intervals[i] == pitch) return true;
  }
  return false;
}

bool isGridCellPlayable(int row, int col) {
  if (gridLayoutMode == 2) return true;
  if (!gridScaleLock) return true;
  return isGridNoteInScale(gridLayout[row][col]);
}

int getGridNote(int row, int col) {
  if (row < 0 || row >= GRID_ROWS || col < 0 || col >= GRID_COLS) return -1;
  return gridLayout[row][col];
}

int getGridVelocity(int row) {
  int y = GRID_START_Y + row * (GRID_CELL_H + GRID_SPACING);
  int offset = constrain(touch.y - y, 0, GRID_CELL_H - 1);
  return map(offset, 0, GRID_CELL_H - 1, 62, 122);
}

String getGridCellLabel(int note) {
  String noteName = getNoteNameFromMIDI(note);
  int octaveStart = noteName.length() - 1;
  while (octaveStart > 0 && isDigit(noteName.charAt(octaveStart - 1))) {
    octaveStart--;
  }
  return noteName.substring(0, octaveStart);
}

#endif
