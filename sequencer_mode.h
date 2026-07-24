#ifndef SEQUENCER_MODE_H
#define SEQUENCER_MODE_H

#include "common_definitions.h"
#include "ui_elements.h"
#include "midi_utils.h"

// Sequencer mode variables
#define SEQ_STEPS 16
#define SEQ_TRACKS 4
#define SEQ_GRID_X 48
#define SEQ_GRID_Y 52
#define SEQ_CELL_W 15
#define SEQ_CELL_H 24
#define SEQ_CELL_GAP 1
#define SEQ_ROW_GAP 4
#define SEQ_LABEL_X 10
#define SEQ_LABEL_W 34

uint8_t sequenceSteps[SEQ_TRACKS][SEQ_STEPS]; // 0=off, 1=normal, 2=accent
bool trackMuted[SEQ_TRACKS] = {false};
int currentStep = 0;
int sequencerPlayhead = -1;
unsigned long lastStepTime = 0;
unsigned long noteOffTime[SEQ_TRACKS] = {0};
int stepInterval;
int sequencerSwing = 50;
int sequencerPatternIndex = 0;
bool sequencerPlaying = false;

const int seqDrumNotes[SEQ_TRACKS] = {36, 38, 42, 46};
const int seqNoteLengths[SEQ_TRACKS] = {140, 110, 42, 180};
const char* seqTrackLabels[SEQ_TRACKS] = {"KCK", "SNR", "HAT", "OPN"};
const char* seqPresetNames[] = {"FOUR", "HOUSE", "BREAK", "DUB"};
const int NUM_SEQ_PRESETS = 4;

// Function declarations
void initializeSequencerMode();
void drawSequencerMode();
void handleSequencerMode();
void drawSequencerGrid();
void drawSequencerControls();
void drawSeqButton(int x, int y, int w, int h, String text, uint16_t color, bool pressed = false);
void toggleSequencerStep(int track, int step);
void clearSequencerPattern();
void applySequencerPreset(int presetIndex);
void applySequencerFill();
void updateSequencer();
void playSequencerStep();
unsigned long getSequencerStepInterval();
int getStepVelocity(int track, int level);
void stopSequencerTrack(int track);

// Implementations
void initializeSequencerMode() {
  stepInterval = 60000 / performance.bpm / 4; // 16th notes
  sequencerPlaying = false;
  currentStep = 0;
  sequencerPlayhead = -1;
  sequencerSwing = 50;
  sequencerPatternIndex = 0;
  clearSequencerPattern();
}

void drawSequencerMode() {
  stepInterval = 60000 / performance.bpm / 4;
  tft.fillScreen(THEME_BG);
  drawHeader("BEATS", seqPresetNames[sequencerPatternIndex] + String("  ") + String(performance.bpm) + " BPM");

  drawSequencerGrid();
  drawSequencerControls();
}

void drawSeqButton(int x, int y, int w, int h, String text, uint16_t color, bool pressed) {
  uint16_t bgColor = pressed ? color : THEME_PANEL;
  uint16_t textColor = pressed ? THEME_BG : THEME_TEXT;

  if (!pressed) {
    tft.fillRoundRect(x + 1, y + 2, w, h, 5, THEME_BG);
  }
  tft.fillRoundRect(x, y + (pressed ? 1 : 0), w, h, 5, bgColor);
  tft.drawRoundRect(x, y + (pressed ? 1 : 0), w, h, 5, color);
  tft.setTextColor(textColor, bgColor);
  tft.drawCentreString(text, x + w / 2, y + h / 2 - 4 + (pressed ? 1 : 0), 1);
}

void drawSequencerControls() {
  tft.fillRect(0, 166, 320, 74, THEME_BG);

  drawSeqButton(8, 172, 50, 24, sequencerPlaying ? "STOP" : "PLAY",
                sequencerPlaying ? THEME_ERROR : THEME_SUCCESS);
  drawSeqButton(64, 172, 44, 24, "PAT", THEME_ACCENT);
  drawSeqButton(114, 172, 44, 24, "FILL", THEME_WARNING);
  drawSeqButton(164, 172, 44, 24, "CLR", THEME_ERROR);
  drawSeqButton(218, 172, 28, 24, "SW-", THEME_SECONDARY);

  tft.fillRoundRect(252, 172, 30, 24, 5, THEME_PANEL);
  tft.drawRoundRect(252, 172, 30, 24, 5, THEME_BORDER);
  tft.setTextColor(sequencerSwing == 50 ? THEME_TEXT_DIM : THEME_PRIMARY, THEME_PANEL);
  tft.drawCentreString(String(sequencerSwing), 267, 179, 1);
  drawSeqButton(288, 172, 24, 24, "+", THEME_SECONDARY);

  drawSeqButton(8, 206, 46, 26, "BPM-", THEME_SECONDARY);
  tft.fillRoundRect(62, 206, 58, 26, 5, THEME_PANEL);
  tft.drawRoundRect(62, 206, 58, 26, 5, THEME_BORDER);
  tft.setTextColor(THEME_TEXT, THEME_PANEL);
  tft.drawCentreString(String(performance.bpm), 91, 212, 2);
  drawSeqButton(128, 206, 46, 26, "BPM+", THEME_SECONDARY);

  tft.fillRoundRect(188, 206, 124, 26, 5, THEME_PANEL);
  tft.drawRoundRect(188, 206, 124, 26, 5, THEME_BORDER);
  tft.setTextColor(THEME_TEXT_DIM, THEME_PANEL);
  tft.drawCentreString("CH " + String(performance.drumsChannel + 1) + "  " +
                       seqPresetNames[sequencerPatternIndex], 250, 212, 2);
}

void drawSequencerGrid() {
  uint16_t trackColors[] = {THEME_ERROR, THEME_WARNING, THEME_PRIMARY, THEME_ACCENT};

  tft.fillRoundRect(6, 46, 308, 114, 6, THEME_PANEL);
  tft.drawRoundRect(6, 46, 308, 114, 6, THEME_BORDER);

  for (int beat = 0; beat < 4; beat++) {
    int x = SEQ_GRID_X + beat * 4 * (SEQ_CELL_W + SEQ_CELL_GAP) - 1;
    tft.drawFastVLine(x, SEQ_GRID_Y - 2, 104, THEME_BORDER);
  }

  for (int track = 0; track < SEQ_TRACKS; track++) {
    int y = SEQ_GRID_Y + track * (SEQ_CELL_H + SEQ_ROW_GAP);
    uint16_t labelBg = trackMuted[track] ? THEME_BG : THEME_PANEL;
    uint16_t labelColor = trackMuted[track] ? THEME_TEXT_DIM : trackColors[track];

    tft.fillRoundRect(SEQ_LABEL_X, y + 1, SEQ_LABEL_W, SEQ_CELL_H - 2, 4, labelBg);
    tft.drawRoundRect(SEQ_LABEL_X, y + 1, SEQ_LABEL_W, SEQ_CELL_H - 2, 4, labelColor);
    tft.setTextColor(labelColor, labelBg);
    tft.drawCentreString(seqTrackLabels[track], SEQ_LABEL_X + SEQ_LABEL_W / 2, y + 8, 1);

    for (int step = 0; step < SEQ_STEPS; step++) {
      int x = SEQ_GRID_X + step * (SEQ_CELL_W + SEQ_CELL_GAP);
      bool current = (sequencerPlaying && step == sequencerPlayhead);
      int level = sequenceSteps[track][step];
      bool accent = level == 2;

      uint16_t fillColor;
      uint16_t borderColor = current ? THEME_TEXT : THEME_BORDER;
      if (trackMuted[track]) {
        fillColor = level ? THEME_SURFACE : THEME_BG;
      } else if (level) {
        fillColor = accent ? THEME_TEXT : trackColors[track];
        borderColor = accent ? trackColors[track] : borderColor;
      } else {
        fillColor = (step % 4 == 0) ? THEME_SURFACE : THEME_BG;
      }

      tft.fillRoundRect(x, y, SEQ_CELL_W, SEQ_CELL_H, 3, fillColor);
      tft.drawRoundRect(x, y, SEQ_CELL_W, SEQ_CELL_H, 3, borderColor);

      if (current) {
        tft.drawFastHLine(x + 2, y + 2, SEQ_CELL_W - 4, THEME_TEXT);
      }
      if (accent && !trackMuted[track]) {
        tft.fillCircle(x + SEQ_CELL_W / 2, y + SEQ_CELL_H / 2, 3, trackColors[track]);
      }
    }
  }
}

void handleSequencerMode() {
  if (touch.justPressed && isButtonPressed(10, 10, 50, 25)) {
    sequencerPlaying = false;
    for (int track = 0; track < SEQ_TRACKS; track++) {
      stopSequencerTrack(track);
    }
    exitToMenu();
    return;
  }

  if (touch.justPressed) {
    if (isButtonPressed(8, 172, 50, 24)) {
      sequencerPlaying = !sequencerPlaying;
      if (sequencerPlaying) {
        currentStep = 0;
        playSequencerStep();
        currentStep = 1;
        lastStepTime = millis();
      } else {
        sequencerPlayhead = -1;
        for (int track = 0; track < SEQ_TRACKS; track++) {
          stopSequencerTrack(track);
        }
      }
      drawSequencerMode();
      return;
    }

    if (isButtonPressed(64, 172, 44, 24)) {
      sequencerPatternIndex = (sequencerPatternIndex + 1) % NUM_SEQ_PRESETS;
      applySequencerPreset(sequencerPatternIndex);
      drawSequencerMode();
      return;
    }

    if (isButtonPressed(114, 172, 44, 24)) {
      applySequencerFill();
      drawSequencerMode();
      return;
    }

    if (isButtonPressed(164, 172, 44, 24)) {
      clearSequencerPattern();
      drawSequencerGrid();
      return;
    }

    if (isButtonPressed(218, 172, 28, 24)) {
      sequencerSwing = max(50, sequencerSwing - 4);
      drawSequencerControls();
      return;
    }

    if (isButtonPressed(288, 172, 24, 24)) {
      sequencerSwing = min(66, sequencerSwing + 4);
      drawSequencerControls();
      return;
    }

    if (isButtonPressed(8, 206, 46, 26)) {
      nudgeGlobalBpm(-1);
      stepInterval = 60000 / performance.bpm / 4;
      drawSequencerMode();
      return;
    }

    if (isButtonPressed(128, 206, 46, 26)) {
      nudgeGlobalBpm(1);
      stepInterval = 60000 / performance.bpm / 4;
      drawSequencerMode();
      return;
    }

    for (int track = 0; track < SEQ_TRACKS; track++) {
      int y = SEQ_GRID_Y + track * (SEQ_CELL_H + SEQ_ROW_GAP);
      if (isButtonPressed(SEQ_LABEL_X, y, SEQ_LABEL_W, SEQ_CELL_H)) {
        trackMuted[track] = !trackMuted[track];
        stopSequencerTrack(track);
        drawSequencerGrid();
        return;
      }
    }

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

  updateSequencer();
}

void toggleSequencerStep(int track, int step) {
  sequenceSteps[track][step] = (sequenceSteps[track][step] + 1) % 3;
}

void clearSequencerPattern() {
  for (int track = 0; track < SEQ_TRACKS; track++) {
    stopSequencerTrack(track);
    trackMuted[track] = false;
    for (int step = 0; step < SEQ_STEPS; step++) {
      sequenceSteps[track][step] = 0;
    }
  }
  sequencerPlayhead = -1;
}

void applySequencerPreset(int presetIndex) {
  clearSequencerPattern();

  switch (presetIndex) {
    case 0: // Four-on-the-floor
      for (int step = 0; step < SEQ_STEPS; step += 4) sequenceSteps[0][step] = 2;
      sequenceSteps[1][4] = 2; sequenceSteps[1][12] = 2;
      for (int step = 2; step < SEQ_STEPS; step += 4) sequenceSteps[2][step] = 1;
      sequenceSteps[3][15] = 1;
      break;
    case 1: // House
      for (int step = 0; step < SEQ_STEPS; step += 4) sequenceSteps[0][step] = 2;
      sequenceSteps[1][4] = 2; sequenceSteps[1][12] = 2;
      for (int step = 0; step < SEQ_STEPS; step += 2) sequenceSteps[2][step] = (step % 4 == 0) ? 1 : 2;
      sequenceSteps[3][2] = 1; sequenceSteps[3][6] = 1; sequenceSteps[3][10] = 1; sequenceSteps[3][14] = 2;
      break;
    case 2: // Breakbeat
      sequenceSteps[0][0] = 2; sequenceSteps[0][3] = 1; sequenceSteps[0][10] = 2;
      sequenceSteps[1][4] = 2; sequenceSteps[1][7] = 1; sequenceSteps[1][12] = 2; sequenceSteps[1][15] = 1;
      for (int step = 0; step < SEQ_STEPS; step += 2) sequenceSteps[2][step] = 1;
      sequenceSteps[2][11] = 2; sequenceSteps[3][14] = 1;
      break;
    case 3: // Dub-ish sparse groove
      sequenceSteps[0][0] = 2; sequenceSteps[0][6] = 1; sequenceSteps[0][11] = 1;
      sequenceSteps[1][4] = 2; sequenceSteps[1][13] = 1;
      sequenceSteps[2][2] = 1; sequenceSteps[2][5] = 1; sequenceSteps[2][10] = 1; sequenceSteps[2][15] = 2;
      sequenceSteps[3][7] = 1;
      sequencerSwing = max(sequencerSwing, 58);
      break;
  }
}

void applySequencerFill() {
  sequenceSteps[1][14] = 1;
  sequenceSteps[1][15] = 2;
  sequenceSteps[2][12] = 1;
  sequenceSteps[2][13] = 1;
  sequenceSteps[2][14] = 2;
  sequenceSteps[2][15] = 1;
  sequenceSteps[3][15] = 2;
}

void updateSequencer() {
  if (!sequencerPlaying) return;

  unsigned long now = millis();

  for (int track = 0; track < SEQ_TRACKS; track++) {
    if (noteOffTime[track] > 0 && now >= noteOffTime[track]) {
      sendNote(performance.drumsChannel, seqDrumNotes[track], 0, false);
      noteOffTime[track] = 0;
    }
  }

  if (now - lastStepTime >= getSequencerStepInterval()) {
    playSequencerStep();
    currentStep = (currentStep + 1) % SEQ_STEPS;
    lastStepTime = now;
    drawSequencerGrid();
  }
}

void playSequencerStep() {
  sequencerPlayhead = currentStep;
  unsigned long now = millis();

  if (!deviceConnected) return;

  for (int track = 0; track < SEQ_TRACKS; track++) {
    int level = sequenceSteps[track][currentStep];
    if (level > 0 && !trackMuted[track]) {
      sendNote(performance.drumsChannel, seqDrumNotes[track], getStepVelocity(track, level), true);
      noteOffTime[track] = now + seqNoteLengths[track];
    }
  }
}

unsigned long getSequencerStepInterval() {
  int baseInterval = 60000 / performance.bpm / 4;
  if (sequencerSwing == 50) return baseInterval;

  int previousStep = (currentStep + SEQ_STEPS - 1) % SEQ_STEPS;
  int pairInterval = baseInterval * 2;
  if (previousStep % 2 == 0) {
    return (pairInterval * sequencerSwing) / 100;
  }
  return (pairInterval * (100 - sequencerSwing)) / 100;
}

int getStepVelocity(int track, int level) {
  int normalVelocities[SEQ_TRACKS] = {108, 96, 72, 82};
  int accentVelocities[SEQ_TRACKS] = {127, 118, 104, 112};
  return level == 2 ? accentVelocities[track] : normalVelocities[track];
}

void stopSequencerTrack(int track) {
  if (noteOffTime[track] > 0) {
    sendNote(performance.drumsChannel, seqDrumNotes[track], 0, false);
    noteOffTime[track] = 0;
  }
}

#endif
