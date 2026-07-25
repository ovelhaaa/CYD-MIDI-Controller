#ifndef AUTO_CHORD_MODE_H
#define AUTO_CHORD_MODE_H

#include "common_definitions.h"
#include "ui_elements.h"
#include "midi_utils.h"

#define CHORD_PADS 8
#define CHORD_MAX_NOTES 5
#define CHORD_MODES 4

int chordOctave = 4;
int chordMode = 0;      // 0=triad, 1=seventh, 2=sus, 3=fifth stack
int chordInversion = 0; // 0=root, 1=1st, 2=2nd, 3=3rd
bool chordHold = false;
bool chordBass = false;
int activeChordNotes[CHORD_PADS][CHORD_MAX_NOTES];
bool chordPressed[CHORD_PADS] = {false};

const char* chordModeNames[CHORD_MODES] = {"TRI", "7TH", "SUS", "5TH"};
const char* chordDegreeNames[CHORD_PADS] = {"I", "ii", "iii", "IV", "V", "vi", "vii", "I+"};

// Function declarations
void initializeAutoChordMode();
void drawAutoChordMode();
void handleAutoChordMode();
void drawChordKeys();
void drawChordControls();
void drawChordButton(int x, int y, int w, int h, String text, uint16_t color, bool pressed = false);
void playChord(int scaleDegree, bool on);
void toggleHeldChord(int scaleDegree);
void stopAllChords();
void buildChordNotes(int scaleDegree, int notes[], int &numNotes);
int getChordRootNote(int scaleDegree);
String getChordName(int scaleDegree);
String getChordStatus();

// Implementations
void initializeAutoChordMode() {
  chordOctave = 4;
  chordMode = 0;
  chordInversion = 0;
  chordHold = false;
  chordBass = false;
  for (int i = 0; i < CHORD_PADS; i++) {
    chordPressed[i] = false;
    for (int j = 0; j < CHORD_MAX_NOTES; j++) {
      activeChordNotes[i][j] = -1;
    }
  }
}

void drawAutoChordMode() {
  tft.fillScreen(THEME_BG);
  drawHeader("CHORD", getRootName() + " " + scales[performance.scale].name + "  " + chordModeNames[chordMode]);

  drawChordKeys();
  drawChordControls();
}

void drawChordButton(int x, int y, int w, int h, String text, uint16_t color, bool pressed) {
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

void drawChordControls() {
  tft.fillRect(0, 176, 320, 64, THEME_BG);

  drawChordButton(8, 182, 28, 18, "O-", THEME_SECONDARY);
  drawChordButton(42, 182, 28, 18, "O+", THEME_SECONDARY);
  drawChordButton(78, 182, 44, 18, chordModeNames[chordMode], THEME_ACCENT);
  drawChordButton(130, 182, 34, 18, "INV", THEME_WARNING);
  drawChordButton(172, 182, 42, 18, chordHold ? "HOLD" : "TAP",
                  chordHold ? THEME_PRIMARY : THEME_SECONDARY);
  drawChordButton(222, 182, 42, 18, chordBass ? "BASS" : "ROOT",
                  chordBass ? THEME_SUCCESS : THEME_SECONDARY);
  drawChordButton(272, 182, 38, 18, "CLR", THEME_ERROR);

  drawChordButton(8, 204, 34, 18, "K-", THEME_SECONDARY);
  drawChordButton(50, 204, 34, 18, "K+", THEME_SECONDARY);
  drawChordButton(94, 204, 52, 18, "SCALE", THEME_ACCENT);

  tft.fillRoundRect(154, 204, 156, 18, 4, THEME_PANEL);
  tft.drawRoundRect(154, 204, 156, 18, 4, THEME_BORDER);
  tft.setTextColor(THEME_TEXT_DIM, THEME_PANEL);
  tft.drawCentreString(getChordStatus(), 232, 208, 1);
}

void drawChordKeys() {
  int keyWidth = 37;
  int keyHeight = 94;
  int keyY = 66;
  int startX = 10;
  int gap = 2;

  uint16_t degreeColors[CHORD_PADS] = {
    THEME_PRIMARY, THEME_SECONDARY, THEME_ACCENT, THEME_SUCCESS,
    THEME_WARNING, THEME_ERROR, 0xF81F, THEME_PRIMARY
  };

  tft.fillRoundRect(6, 58, 308, 108, 6, THEME_PANEL);
  tft.drawRoundRect(6, 58, 308, 108, 6, THEME_BORDER);

  for (int i = 0; i < CHORD_PADS; i++) {
    int x = startX + i * (keyWidth + gap);
    bool active = chordPressed[i];
    uint16_t bgColor = active ? degreeColors[i] : THEME_BG;
    uint16_t textColor = active ? THEME_BG : THEME_TEXT;
    int rootNote = getChordRootNote(i);

    tft.fillRoundRect(x, keyY, keyWidth, keyHeight, 5, bgColor);
    tft.drawRoundRect(x, keyY, keyWidth, keyHeight, 5, active ? THEME_TEXT : degreeColors[i]);

    tft.setTextColor(textColor, bgColor);
    tft.drawCentreString(chordDegreeNames[i], x + keyWidth / 2, keyY + 14, 2);
    tft.setTextColor(active ? THEME_BG : THEME_TEXT_DIM, bgColor);
    tft.drawCentreString(getNoteNameFromMIDI(rootNote), x + keyWidth / 2, keyY + 38, 1);
    tft.drawCentreString(getChordName(i), x + keyWidth / 2, keyY + 58, 1);

    if (chordBass) {
      tft.fillCircle(x + keyWidth / 2, keyY + 78, 2, active ? THEME_BG : degreeColors[i]);
    }
  }
}

void handleAutoChordMode() {
  if (touch.justPressed && isButtonPressed(10, 10, 50, 25)) {
    stopAllChords();
    exitToMenu();
    return;
  }

  if (touch.justPressed) {
    if (isButtonPressed(8, 182, 28, 18)) {
      stopAllChords();
      chordOctave = max(2, chordOctave - 1);
      drawAutoChordMode();
      return;
    }
    if (isButtonPressed(42, 182, 28, 18)) {
      stopAllChords();
      chordOctave = min(6, chordOctave + 1);
      drawAutoChordMode();
      return;
    }
    if (isButtonPressed(78, 182, 44, 18)) {
      stopAllChords();
      chordMode = (chordMode + 1) % CHORD_MODES;
      chordInversion = min(chordInversion, chordMode == 1 ? 3 : 2);
      drawAutoChordMode();
      return;
    }
    if (isButtonPressed(130, 182, 34, 18)) {
      stopAllChords();
      int maxInversion = chordMode == 1 ? 3 : 2;
      chordInversion = (chordInversion + 1) % (maxInversion + 1);
      drawAutoChordMode();
      return;
    }
    if (isButtonPressed(172, 182, 42, 18)) {
      stopAllChords();
      chordHold = !chordHold;
      drawAutoChordMode();
      return;
    }
    if (isButtonPressed(222, 182, 42, 18)) {
      stopAllChords();
      chordBass = !chordBass;
      drawAutoChordMode();
      return;
    }
    if (isButtonPressed(272, 182, 38, 18)) {
      stopAllChords();
      drawAutoChordMode();
      return;
    }
    if (isButtonPressed(8, 204, 34, 18)) {
      stopAllChords();
      nudgeGlobalRoot(-1);
      drawAutoChordMode();
      return;
    }
    if (isButtonPressed(50, 204, 34, 18)) {
      stopAllChords();
      nudgeGlobalRoot(1);
      drawAutoChordMode();
      return;
    }
    if (isButtonPressed(94, 204, 52, 18)) {
      stopAllChords();
      nudgeGlobalScale(1);
      drawAutoChordMode();
      return;
    }

    int keyWidth = 37;
    int keyHeight = 94;
    int keyY = 66;
    int startX = 10;
    int gap = 2;

    for (int i = 0; i < CHORD_PADS; i++) {
      int x = startX + i * (keyWidth + gap);
      if (isButtonPressed(x, keyY, keyWidth, keyHeight)) {
        if (chordHold) {
          toggleHeldChord(i);
        } else if (!chordPressed[i]) {
          stopAllChords();
          playChord(i, true);
          chordPressed[i] = true;
        }
        drawChordKeys();
        drawChordControls();
        return;
      }
    }
  }

  if (!chordHold) {
    int currentKey = -1;
    int keyWidth = 37;
    int keyHeight = 94;
    int keyY = 66;
    int startX = 10;
    int gap = 2;

    if (touch.isPressed) {
      for (int i = 0; i < CHORD_PADS; i++) {
        int x = startX + i * (keyWidth + gap);
        if (touch.x >= x && touch.x < x + keyWidth &&
            touch.y >= keyY && touch.y < keyY + keyHeight) {
          currentKey = i;
          break;
        }
      }
    }

    if (currentKey != -1 && !chordPressed[currentKey]) {
      stopAllChords();
      playChord(currentKey, true);
      chordPressed[currentKey] = true;
      drawChordKeys();
      drawChordControls();
    } else if (!touch.isPressed) {
      bool anyChanged = false;
      for (int i = 0; i < CHORD_PADS; i++) {
        if (chordPressed[i]) anyChanged = true;
      }
      stopAllChords();
      if (anyChanged) {
        drawChordKeys();
        drawChordControls();
      }
    }
  }
}

void toggleHeldChord(int scaleDegree) {
  if (chordPressed[scaleDegree]) {
    playChord(scaleDegree, false);
    chordPressed[scaleDegree] = false;
  } else {
    playChord(scaleDegree, true);
    chordPressed[scaleDegree] = true;
  }
}

void playChord(int scaleDegree, bool on) {
  if (!on) {
    for (int i = 0; i < CHORD_MAX_NOTES; i++) {
      if (activeChordNotes[scaleDegree][i] != -1) {
        sendNote(performance.chordsChannel, activeChordNotes[scaleDegree][i], 0, false);
        activeChordNotes[scaleDegree][i] = -1;
      }
    }
    return;
  }

  int notes[CHORD_MAX_NOTES];
  int numNotes = 0;
  buildChordNotes(scaleDegree, notes, numNotes);

  for (int i = 0; i < numNotes; i++) {
    if (notes[i] >= 24 && notes[i] <= 108) {
      int velocity = (i == 0 || (chordBass && i == 1)) ? 110 : 94;
      sendNote(performance.chordsChannel, notes[i], velocity, true);
      activeChordNotes[scaleDegree][i] = notes[i];
    }
  }
}

void buildChordNotes(int scaleDegree, int notes[], int &numNotes) {
  int degree = scaleDegree == 7 ? scales[performance.scale].numNotes : scaleDegree;
  numNotes = 0;

  if (chordBass) {
    notes[numNotes++] = getNoteInScale(performance.scale, degree, chordOctave - 1);
  }

  if (chordMode == 3) {
    notes[numNotes++] = getNoteInScale(performance.scale, degree, chordOctave);
    notes[numNotes++] = getNoteInScale(performance.scale, degree + 4, chordOctave);
    notes[numNotes++] = getNoteInScale(performance.scale, degree + 7, chordOctave);
  } else {
    notes[numNotes++] = getNoteInScale(performance.scale, degree, chordOctave);
    if (chordMode == 2) {
      notes[numNotes++] = getNoteInScale(performance.scale, degree + 3, chordOctave);
      notes[numNotes++] = getNoteInScale(performance.scale, degree + 4, chordOctave);
    } else {
      notes[numNotes++] = getNoteInScale(performance.scale, degree + 2, chordOctave);
      notes[numNotes++] = getNoteInScale(performance.scale, degree + 4, chordOctave);
      if (chordMode == 1) {
        notes[numNotes++] = getNoteInScale(performance.scale, degree + 6, chordOctave);
      }
    }
  }

  int firstChordIndex = chordBass ? 1 : 0;
  int movableNotes = numNotes - firstChordIndex;
  int moves = min(chordInversion, max(0, movableNotes - 1));
  for (int i = 0; i < moves; i++) {
    notes[firstChordIndex + i] += 12;
  }

  for (int i = firstChordIndex; i < numNotes - 1; i++) {
    for (int j = i + 1; j < numNotes; j++) {
      if (notes[j] < notes[i]) {
        int tmp = notes[i];
        notes[i] = notes[j];
        notes[j] = tmp;
      }
    }
  }
}

int getChordRootNote(int scaleDegree) {
  int degree = scaleDegree == 7 ? scales[performance.scale].numNotes : scaleDegree;
  return getNoteInScale(performance.scale, degree, chordOctave);
}

String getChordName(int scaleDegree) {
  int notes[CHORD_MAX_NOTES];
  int numNotes = 0;
  buildChordNotes(scaleDegree, notes, numNotes);
  int rootIndex = chordBass ? 1 : 0;
  if (numNotes <= rootIndex + 2) return chordModeNames[chordMode];

  int third = (notes[rootIndex + 1] - notes[rootIndex] + 120) % 12;
  int fifth = (notes[rootIndex + 2] - notes[rootIndex] + 120) % 12;
  if (chordMode == 2) return "sus";
  if (chordMode == 3) return "5";
  if (chordMode == 1 && numNotes > rootIndex + 3) {
    int seventh = (notes[rootIndex + 3] - notes[rootIndex] + 120) % 12;
    if (third == 3 && fifth == 6 && seventh == 10) return "m7b5";
    if (third == 3 && seventh == 10) return "m7";
    if (third == 4 && seventh == 10) return "7";
    if (third == 4 && seventh == 11) return "maj7";
  }
  if (third == 3 && fifth == 6) return "dim";
  if (third == 3) return "min";
  if (third == 4) return "maj";
  return chordModeNames[chordMode];
}

String getChordStatus() {
  String status = "Oct " + String(chordOctave) + " Inv " + String(chordInversion);
  status += chordBass ? " Bass" : "";
  status += chordHold ? " Hold" : "";
  return status;
}

void stopAllChords() {
  for (int i = 0; i < CHORD_PADS; i++) {
    if (chordPressed[i]) {
      playChord(i, false);
      chordPressed[i] = false;
    }
  }
}

#endif
