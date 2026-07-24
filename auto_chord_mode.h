#ifndef AUTO_CHORD_MODE_H
#define AUTO_CHORD_MODE_H

#include "common_definitions.h"
#include "ui_elements.h"
#include "midi_utils.h"

// Auto Chord mode variables - traditional piano chords
struct ChordType {
  String name;
  int intervals[4]; // Root, 3rd, 5th, optional 7th
  int numNotes;
};

// Traditional chord types - classic piano voicings
ChordType diatonicChords[] = {
  {"I", {0, 4, 7, -1}, 3},      // Major triad
  {"ii", {0, 3, 7, -1}, 3},     // Minor triad  
  {"iii", {0, 3, 7, -1}, 3},    // Minor triad
  {"IV", {0, 4, 7, -1}, 3},     // Major triad
  {"V", {0, 4, 7, -1}, 3},      // Major triad
  {"vi", {0, 3, 7, -1}, 3},     // Minor triad
  {"vii°", {0, 3, 6, -1}, 3},   // Diminished triad
  {"I+", {0, 4, 7, -1}, 3}      // Major triad (octave)
};

int chordOctave = 4;
int chordScale = 0;
int activeChordNotes[8][4]; // [chord][note index]
bool chordPressed[8] = {false}; // 8 diatonic chords

// Function declarations
void initializeAutoChordMode();
void drawAutoChordMode();
void handleAutoChordMode();
void drawChordKeys();
void playChord(int scaleDegree, bool on);
void stopAllChords();

// Implementations
void initializeAutoChordMode() {
  chordOctave = 4;
  chordScale = 0;
  stopAllChords();
  for (int i = 0; i < 8; i++) {
    chordPressed[i] = false;
    for (int j = 0; j < 4; j++) {
      activeChordNotes[i][j] = -1;
    }
  }
}

void drawAutoChordMode() {
  tft.fillScreen(THEME_BG);
  drawHeader("CHORD MODE", scales[chordScale].name + " Diatonic");
  
  drawChordKeys();
  
  // Controls
  tft.fillRect(0, 176, 320, 64, THEME_BG);
  drawRoundButton(8, 184, 46, 30, "OCT-", THEME_SECONDARY);
  drawRoundButton(60, 184, 46, 30, "OCT+", THEME_SECONDARY);
  drawRoundButton(122, 184, 68, 30, "SCALE", THEME_ACCENT);
  drawRoundButton(202, 184, 62, 30, "CLEAR", THEME_ERROR);
  
  // Status
  tft.fillRoundRect(8, 218, 304, 16, 4, THEME_PANEL);
  tft.setTextColor(THEME_TEXT_DIM, THEME_PANEL);
  tft.drawCentreString("Oct " + String(chordOctave) + "  Classic piano chords", 160, 221, 1);
}

void drawChordKeys() {
  int keyWidth = 37;
  int keyHeight = 96;
  int keyY = 66;
  int startX = 10;
  int gap = 2;
  
  uint16_t degreeColors[] = {
    THEME_PRIMARY, THEME_SECONDARY, THEME_ACCENT, THEME_SUCCESS,
    THEME_WARNING, THEME_ERROR, 0xF81F, 0x07E0
  };

  tft.fillRoundRect(6, 58, 308, 112, 6, THEME_PANEL);
  tft.drawRoundRect(6, 58, 308, 112, 6, THEME_BORDER);
  
  for (int i = 0; i < 8; i++) {
    int x = startX + i * (keyWidth + gap);
    
    uint16_t bgColor = chordPressed[i] ? degreeColors[i] : THEME_BG;
    uint16_t textColor = chordPressed[i] ? THEME_BG : THEME_TEXT;
    
    tft.fillRoundRect(x, keyY, keyWidth, keyHeight, 5, bgColor);
    tft.drawRoundRect(x, keyY, keyWidth, keyHeight, 5, degreeColors[i]);
    
    // Roman numeral
    tft.setTextColor(textColor, bgColor);
    tft.drawCentreString(diatonicChords[i].name, x + keyWidth/2, keyY + 18, 4);
    
    // Root note name
    int rootNote;
    if (i == 7) { // I+ octave
      rootNote = getNoteInScale(chordScale, 0, chordOctave + 1);
    } else {
      rootNote = getNoteInScale(chordScale, i, chordOctave);
    }
    String rootName = getNoteNameFromMIDI(rootNote);
    tft.setTextColor(chordPressed[i] ? THEME_BG : THEME_TEXT_DIM, bgColor);
    tft.drawCentreString(rootName, x + keyWidth/2, keyY + 58, 2);
  }
}

void handleAutoChordMode() {
  // Back button
  if (touch.justPressed && isButtonPressed(10, 10, 50, 25)) {
    exitToMenu();
    return;
  }
  
  if (touch.justPressed) {
    // Octave controls
    if (isButtonPressed(8, 184, 46, 30)) {
      chordOctave = max(2, chordOctave - 1);
      drawAutoChordMode();
      return;
    }
    if (isButtonPressed(60, 184, 46, 30)) {
      chordOctave = min(6, chordOctave + 1);
      drawAutoChordMode();
      return;
    }
    
    // Scale selector
    if (isButtonPressed(122, 184, 68, 30)) {
      chordScale = (chordScale + 1) % NUM_SCALES;
      drawAutoChordMode();
      return;
    }
    
    // Clear all
    if (isButtonPressed(202, 184, 62, 30)) {
      stopAllChords();
      drawChordKeys();
      return;
    }
    
    // Chord keys - only handle on initial press
    int keyWidth = 37;
    int keyHeight = 96;
    int keyY = 66;
    int startX = 10;
    int gap = 2;
    
    for (int i = 0; i < 8; i++) {
      int x = startX + i * (keyWidth + gap);
      if (isButtonPressed(x, keyY, keyWidth, keyHeight)) {
        if (!chordPressed[i]) {
          // Turn on chord
          playChord(i, true);
          chordPressed[i] = true;
          drawChordKeys();
        }
        return;
      }
    }
  }
  
  // Handle single key press/hold functionality
  if (touch.isPressed) {
    int keyWidth = 37;
    int keyHeight = 96;
    int keyY = 66;
    int startX = 10;
    int gap = 2;
    
    int currentKey = -1;
    
    // Find which key is being pressed
    for (int i = 0; i < 8; i++) {
      int x = startX + i * (keyWidth + gap);
      if (touch.x >= x && touch.x < x + keyWidth && 
          touch.y >= keyY && touch.y < keyY + keyHeight) {
        currentKey = i;
        break;
      }
    }
    
    // Only allow one chord at a time
    if (currentKey != -1) {
      // Turn off all other chords first
      for (int i = 0; i < 8; i++) {
        if (i != currentKey && chordPressed[i]) {
          playChord(i, false);
          chordPressed[i] = false;
        }
      }
      
      // Turn on the current chord if not already on
      if (!chordPressed[currentKey]) {
        playChord(currentKey, true);
        chordPressed[currentKey] = true;
        drawChordKeys();
      }
    }
  } else {
    // Touch released - turn off all chords
    bool anyChanged = false;
    for (int i = 0; i < 8; i++) {
      if (chordPressed[i]) {
        playChord(i, false);
        chordPressed[i] = false;
        anyChanged = true;
      }
    }
    if (anyChanged) {
      drawChordKeys();
    }
  }
}

void playChord(int scaleDegree, bool on) {
  if (!deviceConnected) return;
  
  // Get root note for this scale degree
  int rootNote;
  if (scaleDegree == 7) { // I+ octave
    rootNote = getNoteInScale(chordScale, 0, chordOctave + 1);
  } else {
    rootNote = getNoteInScale(chordScale, scaleDegree, chordOctave);
  }
  
  if (on) {
    // Play traditional diatonic chord (root, 3rd, 5th)
    ChordType chord = diatonicChords[scaleDegree];
    
    for (int i = 0; i < chord.numNotes; i++) {
      if (chord.intervals[i] >= 0) {
        int chordNote = rootNote + chord.intervals[i];
        if (chordNote >= 24 && chordNote <= 108) {
          sendMIDI(0x90, chordNote, 100);
          activeChordNotes[scaleDegree][i] = chordNote;
        }
      }
    }
  } else {
    // Stop chord for this specific scale degree
    for (int i = 0; i < 4; i++) {
      if (activeChordNotes[scaleDegree][i] != -1) {
        sendMIDI(0x80, activeChordNotes[scaleDegree][i], 0);
        activeChordNotes[scaleDegree][i] = -1;
      }
    }
  }
}

void stopAllChords() {
  for (int i = 0; i < 8; i++) {
    if (chordPressed[i]) {
      playChord(i, false);
      chordPressed[i] = false;
    }
  }
}

#endif
