#ifndef SEQUENCER_MODE_H
#define SEQUENCER_MODE_H

#include "common_definitions.h"
#include "ui_elements.h"
#include "midi_utils.h"

// Sequencer mode variables
#define SEQ_STEPS 16
#define SEQ_TRACKS 4
#define SEQ_GRID_X 46
#define SEQ_GRID_Y 54
#define SEQ_CELL_W 15
#define SEQ_CELL_H 28
#define SEQ_CELL_GAP 1
#define SEQ_ROW_GAP 4
bool sequencePattern[SEQ_TRACKS][SEQ_STEPS];
int currentStep = 0;
unsigned long lastStepTime = 0;
unsigned long noteOffTime[SEQ_TRACKS] = {0};
int stepInterval;
bool sequencerPlaying = false;

// Function declarations
void initializeSequencerMode();
void drawSequencerMode();
void handleSequencerMode();
void drawSequencerGrid();
void toggleSequencerStep(int track, int step);
void updateSequencer();
void playSequencerStep();

// Implementations
void initializeSequencerMode() {
  stepInterval = 60000 / performance.bpm / 4; // 16th notes
  sequencerPlaying = false;
  currentStep = 0;
  
  // Clear all patterns
  for (int t = 0; t < SEQ_TRACKS; t++) {
    for (int s = 0; s < SEQ_STEPS; s++) {
      sequencePattern[t][s] = false;
    }
  }
}

void drawSequencerMode() {
  stepInterval = 60000 / performance.bpm / 4;
  tft.fillScreen(THEME_BG);
  drawHeader("BEATS", String(performance.bpm) + " BPM");
  
  drawSequencerGrid();
  
  // Transport controls - positioned to avoid overlap
  tft.fillRect(0, 194, 320, 46, THEME_BG);
  drawRoundButton(8, 202, 56, 30, sequencerPlaying ? "STOP" : "PLAY", 
                 sequencerPlaying ? THEME_ERROR : THEME_SUCCESS);
  drawRoundButton(72, 202, 60, 30, "CLEAR", THEME_WARNING);
  drawRoundButton(148, 202, 46, 30, "BPM-", THEME_SECONDARY);
  drawRoundButton(202, 202, 46, 30, "BPM+", THEME_SECONDARY);
  
  // BPM display
  tft.fillRoundRect(260, 202, 52, 30, 5, THEME_PANEL);
  tft.setTextColor(THEME_TEXT, THEME_PANEL);
  tft.drawCentreString(String(performance.bpm), 286, 209, 2);
}

void drawSequencerGrid() {
  // 808-style track labels and colors
  String trackLabels[] = {"KICK", "SNRE", "HHAT", "OPEN"};
  uint16_t trackColors[] = {THEME_ERROR, THEME_WARNING, THEME_PRIMARY, THEME_ACCENT};

  tft.fillRoundRect(6, 48, 308, 140, 6, THEME_PANEL);
  tft.drawRoundRect(6, 48, 308, 140, 6, THEME_BORDER);
  
  for (int track = 0; track < SEQ_TRACKS; track++) {
    int y = SEQ_GRID_Y + track * (SEQ_CELL_H + SEQ_ROW_GAP);
    
    // Track name with color coding
    tft.setTextColor(trackColors[track], THEME_PANEL);
    tft.drawString(trackLabels[track], 12, y + 10, 1);
    
    // Steps - 16 steps in 808 style
    for (int step = 0; step < SEQ_STEPS; step++) {
      int x = SEQ_GRID_X + step * (SEQ_CELL_W + SEQ_CELL_GAP);
      
      bool active = sequencePattern[track][step];
      bool current = (sequencerPlaying && step == currentStep);
      
      uint16_t color;
      if (current && active) color = THEME_TEXT;
      else if (current) color = trackColors[track];
      else if (active) color = trackColors[track];
      else color = (step % 4 == 0) ? THEME_SURFACE : THEME_BG;
      
      // Highlight every 4th step (like 808)
      if (step % 4 == 0) {
        tft.drawRect(x - 1, y - 1, SEQ_CELL_W + 2, SEQ_CELL_H + 2, THEME_TEXT_DIM);
      }
      
      tft.fillRoundRect(x, y, SEQ_CELL_W, SEQ_CELL_H, 3, color);
      tft.drawRoundRect(x, y, SEQ_CELL_W, SEQ_CELL_H, 3, active ? trackColors[track] : THEME_BORDER);
    }
  }
}

void handleSequencerMode() {
  // Back button
  if (touch.justPressed && isButtonPressed(10, 10, 50, 25)) {
    sequencerPlaying = false;
    int drumNotes[] = {36, 38, 42, 46};
    for (int track = 0; track < SEQ_TRACKS; track++) {
      if (noteOffTime[track] > 0) {
        sendNote(performance.drumsChannel, drumNotes[track], 0, false);
        noteOffTime[track] = 0;
      }
    }
    exitToMenu();
    return;
  }
  
  // Handle touch input
  if (touch.justPressed) {
    // Transport controls
    if (isButtonPressed(8, 202, 56, 30)) {
      sequencerPlaying = !sequencerPlaying;
      if (sequencerPlaying) {
        currentStep = 0;
        lastStepTime = millis();
      }
      drawSequencerMode();
      return;
    }
    
    if (isButtonPressed(72, 202, 60, 30)) {
      // Clear all patterns
      for (int t = 0; t < SEQ_TRACKS; t++) {
        for (int s = 0; s < SEQ_STEPS; s++) {
          sequencePattern[t][s] = false;
        }
      }
      drawSequencerGrid();
      return;
    }
    
    if (isButtonPressed(148, 202, 46, 30)) {
      nudgeGlobalBpm(-1);
      stepInterval = 60000 / performance.bpm / 4;
      drawSequencerMode();
      return;
    }
    
    if (isButtonPressed(202, 202, 46, 30)) {
      nudgeGlobalBpm(1);
      stepInterval = 60000 / performance.bpm / 4;
      drawSequencerMode();
      return;
    }
    
    // Grid interaction
    for (int track = 0; track < SEQ_TRACKS; track++) {
      for (int step = 0; step < SEQ_STEPS; step++) {
        int x = SEQ_GRID_X + step * (SEQ_CELL_W + SEQ_CELL_GAP);
        int y = SEQ_GRID_Y + track * (SEQ_CELL_H + SEQ_ROW_GAP);
        
        if (isButtonPressed(x, y, SEQ_CELL_W, SEQ_CELL_H)) {
          toggleSequencerStep(track, step);
          drawSequencerGrid();
          return;
        }
      }
    }
  }
  
  // Update sequencer timing
  updateSequencer();
}

void toggleSequencerStep(int track, int step) {
  sequencePattern[track][step] = !sequencePattern[track][step];
}

void updateSequencer() {
  if (!sequencerPlaying) return;
  
  unsigned long now = millis();
  
  // Check for notes to turn off
  int drumNotes[] = {36, 38, 42, 46};
  for (int track = 0; track < SEQ_TRACKS; track++) {
    if (noteOffTime[track] > 0 && now >= noteOffTime[track]) {
      sendNote(performance.drumsChannel, drumNotes[track], 0, false);
      noteOffTime[track] = 0;
    }
  }
  
  if (now - lastStepTime >= stepInterval) {
    playSequencerStep();
    currentStep = (currentStep + 1) % SEQ_STEPS;
    lastStepTime = now;
    drawSequencerGrid();
  }
}

void playSequencerStep() {
  if (!deviceConnected) return;
  
  int drumNotes[] = {36, 38, 42, 46}; // Kick, Snare, Hi-hat, Open Hi-hat
  int noteLengths[] = {200, 150, 50, 300}; // Note lengths in ms
  
  unsigned long now = millis();
  
  for (int track = 0; track < SEQ_TRACKS; track++) {
    if (sequencePattern[track][currentStep]) {
      // Turn on note
      sendNote(performance.drumsChannel, drumNotes[track], 100, true);
      // Schedule note off
      noteOffTime[track] = now + noteLengths[track];
    }
  }
}

#endif
