#ifndef RANDOM_GENERATOR_MODE_H
#define RANDOM_GENERATOR_MODE_H

#include "common_definitions.h"
#include "ui_elements.h"
#include "midi_utils.h"

// Random Generator mode variables
struct RandomGen {
  int minOctave = 3;
  int maxOctave = 6;
  int probability = 50; // 0-100%
  int subdivision = 4; // 4=quarter, 8=eighth, 16=sixteenth
  bool isPlaying = false;
  unsigned long lastNoteTime = 0;
  unsigned long nextNoteTime = 0;
  int currentNote = -1;
  unsigned long noteInterval = 500; // Calculated from BPM
};

RandomGen randomGen;

// Function declarations
void initializeRandomGeneratorMode();
void drawRandomGeneratorMode();
void handleRandomGeneratorMode();
void drawRandomGenControls();
void updateRandomGenerator();
void playRandomNote();
void calculateNoteInterval();

// Implementations
void initializeRandomGeneratorMode() {
  randomGen.minOctave = 3;
  randomGen.maxOctave = 6;
  randomGen.probability = 50;
  randomGen.subdivision = 4;
  randomGen.isPlaying = false;
  randomGen.currentNote = -1;
  calculateNoteInterval();
  randomGen.nextNoteTime = millis() + randomGen.noteInterval;
}

void drawRandomGeneratorMode() {
  calculateNoteInterval();
  tft.fillScreen(THEME_BG);
  drawHeader("RNG JAMS", "Random Music");
  
  drawRandomGenControls();
}

void drawRandomGenControls() {
  int y = 56;
  int spacing = 34;

  tft.fillRoundRect(6, 50, 308, 178, 6, THEME_PANEL);
  tft.drawRoundRect(6, 50, 308, 178, 6, THEME_BORDER);
  
  // Play/Stop and Root note on same line
  drawRoundButton(14, y, 62, 30, randomGen.isPlaying ? "STOP" : "PLAY",
                 randomGen.isPlaying ? THEME_ERROR : THEME_SUCCESS);
  
  tft.setTextColor(THEME_TEXT_DIM, THEME_PANEL);
  tft.drawString("KEY", 88, y + 4, 1);
  drawRoundButton(116, y, 42, 30, getRootName(), THEME_PRIMARY);
  drawRoundButton(164, y, 30, 30, "+", THEME_SECONDARY);
  drawRoundButton(200, y, 30, 30, "-", THEME_SECONDARY);
  
  // Scale selector
  drawRoundButton(238, y, 66, 30, scales[performance.scale].name, THEME_ACCENT);
  
  y += spacing;
  
  // Octave range
  tft.setTextColor(THEME_TEXT_DIM, THEME_PANEL);
  tft.drawString("OCT", 16, y + 4, 1);
  tft.setTextColor(THEME_TEXT, THEME_PANEL);
  tft.drawString(String(randomGen.minOctave) + "-" + String(randomGen.maxOctave), 16, y + 17, 2);
  drawRoundButton(70, y, 42, 30, "MIN-", THEME_SECONDARY);
  drawRoundButton(118, y, 42, 30, "MIN+", THEME_SECONDARY);
  drawRoundButton(174, y, 42, 30, "MAX-", THEME_SECONDARY);
  drawRoundButton(222, y, 42, 30, "MAX+", THEME_SECONDARY);
  
  y += spacing;
  
  // Probability with visual bar
  tft.setTextColor(THEME_TEXT_DIM, THEME_PANEL);
  tft.drawString("CHANCE", 16, y + 4, 1);
  tft.setTextColor(THEME_TEXT, THEME_PANEL);
  tft.drawString(String(randomGen.probability) + "%", 16, y + 17, 2);
  drawRoundButton(80, y, 30, 30, "-", THEME_SECONDARY);
  drawRoundButton(116, y, 30, 30, "+", THEME_SECONDARY);
  
  // Compact probability bar - clear and redraw
  int barW = 144;
  int barX = 160;
  tft.fillRoundRect(barX, y + 9, barW, 14, 3, THEME_BG);
  tft.drawRoundRect(barX, y + 9, barW, 14, 3, THEME_BORDER);
  int fillW = (barW * randomGen.probability) / 100;
  if (fillW > 0) {
    tft.fillRoundRect(barX + 1, y + 10, max(1, fillW - 2), 12, 2, THEME_PRIMARY);
  }
  
  y += spacing;
  
  // BPM and subdivision controls
  tft.setTextColor(THEME_TEXT_DIM, THEME_PANEL);
  tft.drawString("BPM", 16, y + 4, 1);
  tft.setTextColor(THEME_TEXT, THEME_PANEL);
  tft.drawString(String(performance.bpm), 16, y + 17, 2);
  drawRoundButton(64, y, 30, 30, "-", THEME_SECONDARY);
  drawRoundButton(100, y, 30, 30, "+", THEME_SECONDARY);
  
  tft.setTextColor(THEME_TEXT_DIM, THEME_PANEL);
  tft.drawString("BEAT", 148, y + 4, 1);
  String subdivText;
  if (randomGen.subdivision == 4) subdivText = "1/4";
  else if (randomGen.subdivision == 8) subdivText = "1/8";
  else if (randomGen.subdivision == 16) subdivText = "1/16";
  tft.setTextColor(THEME_TEXT, THEME_PANEL);
  tft.drawString(subdivText, 148, y + 17, 2);
  drawRoundButton(206, y, 30, 30, "<", THEME_SECONDARY);
  drawRoundButton(242, y, 30, 30, ">", THEME_SECONDARY);
  
  y += spacing;
  
  // Current note indicator (compact)
  tft.fillRoundRect(14, y, 290, 28, 5, THEME_BG);
  if (randomGen.currentNote != -1) {
    String currentNoteName = getNoteNameFromMIDI(randomGen.currentNote);
    tft.setTextColor(THEME_ACCENT, THEME_BG);
    tft.drawCentreString("NOW  " + currentNoteName, 160, y + 7, 2);
  } else {
    tft.setTextColor(THEME_TEXT_DIM, THEME_BG);
    tft.drawCentreString(randomGen.isPlaying ? "Waiting for chance" : "Idle", 160, y + 7, 2);
  }
}

void handleRandomGeneratorMode() {
  // Back button
  if (touch.justPressed && isButtonPressed(10, 10, 50, 25)) {
    if (randomGen.currentNote != -1) {
      sendNote(performance.generativeChannel, randomGen.currentNote, 0, false);
      randomGen.currentNote = -1;
    }
    randomGen.isPlaying = false;
    exitToMenu();
    return;
  }
  
  if (touch.justPressed) {
    int y = 56;
    int spacing = 34;
    
    // Play/Stop and Root note controls
    if (isButtonPressed(14, y, 62, 30)) {
      randomGen.isPlaying = !randomGen.isPlaying;
      if (randomGen.isPlaying) {
        calculateNoteInterval();
        randomGen.nextNoteTime = millis() + randomGen.noteInterval;
      } else if (randomGen.currentNote != -1) {
        sendNote(performance.generativeChannel, randomGen.currentNote, 0, false);
        randomGen.currentNote = -1;
      }
      drawRandomGenControls();
      return;
    }
    
    if (isButtonPressed(164, y, 30, 30)) {
      nudgeGlobalRoot(1);
      drawRandomGenControls();
      return;
    }
    if (isButtonPressed(200, y, 30, 30)) {
      nudgeGlobalRoot(-1);
      drawRandomGenControls();
      return;
    }
    
    // Scale selector
    if (isButtonPressed(238, y, 66, 30)) {
      nudgeGlobalScale(1);
      drawRandomGenControls();
      return;
    }
    
    y += spacing;
    
    // Octave controls
    if (isButtonPressed(70, y, 42, 30)) {
      randomGen.minOctave = max(1, randomGen.minOctave - 1);
      if (randomGen.minOctave >= randomGen.maxOctave) {
        randomGen.maxOctave = randomGen.minOctave + 1;
      }
      drawRandomGenControls();
      return;
    }
    if (isButtonPressed(118, y, 42, 30)) {
      randomGen.minOctave = min(8, randomGen.minOctave + 1);
      if (randomGen.minOctave >= randomGen.maxOctave) {
        randomGen.maxOctave = randomGen.minOctave + 1;
      }
      drawRandomGenControls();
      return;
    }
    if (isButtonPressed(174, y, 42, 30)) {
      randomGen.maxOctave = max(randomGen.minOctave + 1, randomGen.maxOctave - 1);
      drawRandomGenControls();
      return;
    }
    if (isButtonPressed(222, y, 42, 30)) {
      randomGen.maxOctave = min(9, randomGen.maxOctave + 1);
      drawRandomGenControls();
      return;
    }
    
    y += spacing;
    
    // Probability controls
    if (isButtonPressed(80, y, 30, 30)) {
      randomGen.probability = max(0, randomGen.probability - 5);
      drawRandomGenControls();
      return;
    }
    if (isButtonPressed(116, y, 30, 30)) {
      randomGen.probability = min(100, randomGen.probability + 5);
      drawRandomGenControls();
      return;
    }
    
    y += spacing;
    
    // BPM controls
    if (isButtonPressed(64, y, 30, 30)) {
      nudgeGlobalBpm(-5);
      calculateNoteInterval();
      drawRandomGenControls();
      return;
    }
    if (isButtonPressed(100, y, 30, 30)) {
      nudgeGlobalBpm(5);
      calculateNoteInterval();
      drawRandomGenControls();
      return;
    }
    
    // Subdivision controls
    if (isButtonPressed(206, y, 30, 30)) {
      if (randomGen.subdivision == 16) randomGen.subdivision = 8;
      else if (randomGen.subdivision == 8) randomGen.subdivision = 4;
      calculateNoteInterval();
      drawRandomGenControls();
      return;
    }
    if (isButtonPressed(242, y, 30, 30)) {
      if (randomGen.subdivision == 4) randomGen.subdivision = 8;
      else if (randomGen.subdivision == 8) randomGen.subdivision = 16;
      calculateNoteInterval();
      drawRandomGenControls();
      return;
    }
  }
  
  // Update random generator
  updateRandomGenerator();
}

void updateRandomGenerator() {
  if (!randomGen.isPlaying || !deviceConnected) return;
  
  unsigned long now = millis();
  
  if (now >= randomGen.nextNoteTime) {
    playRandomNote();
    randomGen.nextNoteTime = now + randomGen.noteInterval;
  }
}

void playRandomNote() {
  // Stop current note if playing
  if (randomGen.currentNote != -1) {
    sendNote(performance.generativeChannel, randomGen.currentNote, 0, false);
    randomGen.currentNote = -1;
  }
  
  // Check probability
  if (random(100) < randomGen.probability) {
    // Generate random note in scale and octave range
    Scale& scale = scales[performance.scale];
    int degree = random(scale.numNotes);
    int octave = random(randomGen.minOctave, randomGen.maxOctave + 1);
    int note = performance.root + scale.intervals[degree] + (octave * 12);
    
    if (note >= 0 && note <= 127) {
      sendNote(performance.generativeChannel, note, 100, true);
      randomGen.currentNote = note;
      
      Serial.printf("Random note: %s (prob: %d%%)\n", 
                   getNoteNameFromMIDI(note).c_str(), randomGen.probability);
      
      // Update display
      drawRandomGenControls();
    }
  }
}

void calculateNoteInterval() {
  // Calculate note interval from BPM and subdivision
  float beatsPerSecond = performance.bpm / 60.0;
  float notesPerSecond = beatsPerSecond * (randomGen.subdivision / 4.0);
  randomGen.noteInterval = 1000.0 / notesPerSecond;
}

#endif
