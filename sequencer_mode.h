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
#define SEQ_STATE_OFF 0
#define SEQ_STATE_HIT 1
#define SEQ_STATE_ACCENT 2
#define SEQ_STATE_RATCHET 3

uint8_t sequenceSteps[SEQ_TRACKS][SEQ_STEPS]; // 0=off, 1=hit, 2=accent, 3=ratchet
bool trackMuted[SEQ_TRACKS] = {false};
int currentStep = 0;
int sequencerPlayhead = -1;
unsigned long lastStepTime = 0;
unsigned long noteOffTime[SEQ_TRACKS] = {0};
bool ratchetPending[SEQ_TRACKS] = {false};
unsigned long ratchetTime[SEQ_TRACKS] = {0};
int stepInterval;
int sequencerSwing = 50;
int sequencerPatternIndex = 0;
int sequencerProbability = 92;
int sequencerHumanize = 6;
int sequencerDensity = 58;
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
void generateSequencerGroove();
void varySequencerGroove();
void rotateSequencerPattern(int amount);
void updateSequencer();
void playSequencerStep();
void playSequencerHit(int track, int level, unsigned long now, int lengthOverride = 0);
unsigned long getSequencerStepInterval();
int getStepVelocity(int track, int level);
void stopSequencerTrack(int track);
void stopSequencerNotes();

// Implementations
void initializeSequencerMode() {
  stepInterval = 60000 / performance.bpm / 4; // 16th notes
  sequencerPlaying = false;
  currentStep = 0;
  sequencerPlayhead = -1;
  sequencerSwing = 50;
  sequencerPatternIndex = 0;
  sequencerProbability = 92;
  sequencerHumanize = 6;
  sequencerDensity = 58;
  clearSequencerPattern();
  applySequencerPreset(sequencerPatternIndex);
}

void drawSequencerMode() {
  stepInterval = 60000 / performance.bpm / 4;
  tft.fillScreen(THEME_BG);
  drawHeader("BEATS", seqPresetNames[sequencerPatternIndex] + String("  ") +
             String(performance.bpm) + " BPM  SW" + String(sequencerSwing));

  drawSequencerGrid();
  drawSequencerControls();
}

void drawSeqButton(int x, int y, int w, int h, String text, uint16_t color, bool pressed) {
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

void drawSequencerControls() {
  tft.fillRect(0, 164, 320, 76, THEME_BG);

  drawSeqButton(8, 166, 46, 18, sequencerPlaying ? "STOP" : "PLAY",
                sequencerPlaying ? THEME_ERROR : THEME_SUCCESS);
  drawSeqButton(60, 166, 38, 18, "PAT", THEME_ACCENT);
  drawSeqButton(104, 166, 38, 18, "GEN", THEME_PRIMARY);
  drawSeqButton(148, 166, 38, 18, "VAR", THEME_WARNING);
  drawSeqButton(194, 166, 38, 18, "FILL", THEME_WARNING);
  drawSeqButton(240, 166, 34, 18, "CLR", THEME_ERROR);
  drawSeqButton(284, 166, 28, 18, "MIX", THEME_SECONDARY);

  drawSeqButton(8, 188, 38, 18, "B-", THEME_SECONDARY);
  tft.fillRoundRect(52, 188, 42, 18, 5, THEME_PANEL);
  tft.drawRoundRect(52, 188, 42, 18, 5, THEME_BORDER);
  tft.setTextColor(THEME_TEXT, THEME_PANEL);
  tft.drawCentreString(String(performance.bpm), 73, 192, 1);
  drawSeqButton(100, 188, 38, 18, "B+", THEME_SECONDARY);

  drawSeqButton(148, 188, 28, 18, "S-", THEME_SECONDARY);
  tft.fillRoundRect(182, 188, 28, 18, 5, THEME_PANEL);
  tft.drawRoundRect(182, 188, 28, 18, 5, THEME_BORDER);
  tft.setTextColor(sequencerSwing == 50 ? THEME_TEXT_DIM : THEME_PRIMARY, THEME_PANEL);
  tft.drawCentreString(String(sequencerSwing), 196, 192, 1);
  drawSeqButton(216, 188, 28, 18, "S+", THEME_SECONDARY);

  drawSeqButton(252, 188, 28, 18, "P-", THEME_SECONDARY);
  drawSeqButton(284, 188, 28, 18, "P+", THEME_SECONDARY);

  drawSeqButton(8, 210, 34, 18, "<", THEME_ACCENT);
  drawSeqButton(48, 210, 34, 18, ">", THEME_ACCENT);
  drawSeqButton(90, 210, 30, 18, "H-", THEME_SECONDARY);
  tft.fillRoundRect(126, 210, 28, 18, 5, THEME_PANEL);
  tft.drawRoundRect(126, 210, 28, 18, 5, THEME_BORDER);
  tft.setTextColor(sequencerHumanize == 0 ? THEME_TEXT_DIM : THEME_WARNING, THEME_PANEL);
  tft.drawCentreString(String(sequencerHumanize), 140, 214, 1);
  drawSeqButton(160, 210, 30, 18, "H+", THEME_SECONDARY);

  tft.fillRoundRect(198, 210, 114, 18, 5, THEME_PANEL);
  tft.drawRoundRect(198, 210, 114, 18, 5, THEME_BORDER);
  tft.setTextColor(THEME_TEXT_DIM, THEME_PANEL);
  tft.drawCentreString("P" + String(sequencerProbability) + " D" +
                       String(sequencerDensity) + " CH" +
                       String(performance.drumsChannel + 1), 255, 214, 1);
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
      bool accent = level == SEQ_STATE_ACCENT;
      bool ratchet = level == SEQ_STATE_RATCHET;

      uint16_t fillColor;
      uint16_t borderColor = current ? THEME_TEXT : THEME_BORDER;
      if (trackMuted[track]) {
        fillColor = level ? THEME_SURFACE : THEME_BG;
      } else if (level) {
        fillColor = accent ? THEME_TEXT : (ratchet ? THEME_SURFACE : trackColors[track]);
        borderColor = accent || ratchet ? trackColors[track] : borderColor;
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
      if (ratchet && !trackMuted[track]) {
        tft.fillRect(x + 4, y + 6, 2, SEQ_CELL_H - 12, trackColors[track]);
        tft.fillRect(x + 9, y + 6, 2, SEQ_CELL_H - 12, trackColors[track]);
      }
    }
  }
}

void handleSequencerMode() {
  if (touch.justPressed && isButtonPressed(10, 10, 50, 25)) {
    sequencerPlaying = false;
    stopSequencerNotes();
    exitToMenu();
    return;
  }

  if (touch.justPressed) {
    if (isButtonPressed(8, 166, 46, 18)) {
      sequencerPlaying = !sequencerPlaying;
      if (sequencerPlaying) {
        currentStep = 0;
        playSequencerStep();
        currentStep = 1;
        lastStepTime = millis();
      } else {
        sequencerPlayhead = -1;
        stopSequencerNotes();
      }
      drawSequencerMode();
      return;
    }

    if (isButtonPressed(60, 166, 38, 18)) {
      stopSequencerNotes();
      sequencerPatternIndex = (sequencerPatternIndex + 1) % NUM_SEQ_PRESETS;
      applySequencerPreset(sequencerPatternIndex);
      drawSequencerMode();
      return;
    }

    if (isButtonPressed(104, 166, 38, 18)) {
      generateSequencerGroove();
      drawSequencerMode();
      return;
    }

    if (isButtonPressed(148, 166, 38, 18)) {
      varySequencerGroove();
      drawSequencerMode();
      return;
    }

    if (isButtonPressed(194, 166, 38, 18)) {
      applySequencerFill();
      drawSequencerMode();
      return;
    }

    if (isButtonPressed(240, 166, 34, 18)) {
      stopSequencerNotes();
      clearSequencerPattern();
      drawSequencerGrid();
      return;
    }

    if (isButtonPressed(284, 166, 28, 18)) {
      sequencerDensity = constrain(sequencerDensity + 14, 30, 86);
      if (sequencerDensity >= 86) sequencerDensity = 44;
      generateSequencerGroove();
      drawSequencerMode();
      return;
    }

    if (isButtonPressed(8, 188, 38, 18)) {
      nudgeGlobalBpm(-1);
      stepInterval = 60000 / performance.bpm / 4;
      drawSequencerMode();
      return;
    }

    if (isButtonPressed(100, 188, 38, 18)) {
      nudgeGlobalBpm(1);
      stepInterval = 60000 / performance.bpm / 4;
      drawSequencerMode();
      return;
    }

    if (isButtonPressed(148, 188, 28, 18)) {
      sequencerSwing = max(50, sequencerSwing - 4);
      drawSequencerMode();
      return;
    }

    if (isButtonPressed(216, 188, 28, 18)) {
      sequencerSwing = min(66, sequencerSwing + 4);
      drawSequencerMode();
      return;
    }

    if (isButtonPressed(252, 188, 28, 18)) {
      sequencerProbability = max(45, sequencerProbability - 5);
      drawSequencerControls();
      return;
    }

    if (isButtonPressed(284, 188, 28, 18)) {
      sequencerProbability = min(100, sequencerProbability + 5);
      drawSequencerControls();
      return;
    }

    if (isButtonPressed(8, 210, 34, 18)) {
      rotateSequencerPattern(-1);
      drawSequencerGrid();
      return;
    }

    if (isButtonPressed(48, 210, 34, 18)) {
      rotateSequencerPattern(1);
      drawSequencerGrid();
      return;
    }

    if (isButtonPressed(90, 210, 30, 18)) {
      sequencerHumanize = max(0, sequencerHumanize - 2);
      drawSequencerControls();
      return;
    }

    if (isButtonPressed(160, 210, 30, 18)) {
      sequencerHumanize = min(18, sequencerHumanize + 2);
      drawSequencerControls();
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
  sequenceSteps[track][step] = (sequenceSteps[track][step] + 1) % 4;
}

void clearSequencerPattern() {
  for (int track = 0; track < SEQ_TRACKS; track++) {
    stopSequencerTrack(track);
    trackMuted[track] = false;
    for (int step = 0; step < SEQ_STEPS; step++) {
      sequenceSteps[track][step] = SEQ_STATE_OFF;
    }
  }
  sequencerPlayhead = -1;
}

void applySequencerPreset(int presetIndex) {
  clearSequencerPattern();

  switch (presetIndex) {
    case 0: // Four-on-the-floor
      for (int step = 0; step < SEQ_STEPS; step += 4) sequenceSteps[0][step] = SEQ_STATE_ACCENT;
      sequenceSteps[1][4] = SEQ_STATE_ACCENT; sequenceSteps[1][12] = SEQ_STATE_ACCENT;
      for (int step = 2; step < SEQ_STEPS; step += 4) sequenceSteps[2][step] = SEQ_STATE_HIT;
      sequenceSteps[3][15] = SEQ_STATE_HIT;
      break;
    case 1: // House
      for (int step = 0; step < SEQ_STEPS; step += 4) sequenceSteps[0][step] = SEQ_STATE_ACCENT;
      sequenceSteps[1][4] = SEQ_STATE_ACCENT; sequenceSteps[1][12] = SEQ_STATE_ACCENT;
      for (int step = 0; step < SEQ_STEPS; step += 2) {
        sequenceSteps[2][step] = (step % 4 == 0) ? SEQ_STATE_HIT : SEQ_STATE_ACCENT;
      }
      sequenceSteps[3][2] = SEQ_STATE_HIT; sequenceSteps[3][6] = SEQ_STATE_HIT;
      sequenceSteps[3][10] = SEQ_STATE_HIT; sequenceSteps[3][14] = SEQ_STATE_ACCENT;
      break;
    case 2: // Breakbeat
      sequenceSteps[0][0] = SEQ_STATE_ACCENT; sequenceSteps[0][3] = SEQ_STATE_HIT;
      sequenceSteps[0][10] = SEQ_STATE_ACCENT;
      sequenceSteps[1][4] = SEQ_STATE_ACCENT; sequenceSteps[1][7] = SEQ_STATE_HIT;
      sequenceSteps[1][12] = SEQ_STATE_ACCENT; sequenceSteps[1][15] = SEQ_STATE_HIT;
      for (int step = 0; step < SEQ_STEPS; step += 2) sequenceSteps[2][step] = SEQ_STATE_HIT;
      sequenceSteps[2][11] = SEQ_STATE_RATCHET; sequenceSteps[3][14] = SEQ_STATE_HIT;
      break;
    case 3: // Dub-ish sparse groove
      sequenceSteps[0][0] = SEQ_STATE_ACCENT; sequenceSteps[0][6] = SEQ_STATE_HIT;
      sequenceSteps[0][11] = SEQ_STATE_HIT;
      sequenceSteps[1][4] = SEQ_STATE_ACCENT; sequenceSteps[1][13] = SEQ_STATE_HIT;
      sequenceSteps[2][2] = SEQ_STATE_HIT; sequenceSteps[2][5] = SEQ_STATE_HIT;
      sequenceSteps[2][10] = SEQ_STATE_HIT; sequenceSteps[2][15] = SEQ_STATE_RATCHET;
      sequenceSteps[3][7] = SEQ_STATE_HIT;
      sequencerSwing = max(sequencerSwing, 58);
      break;
  }
}

void applySequencerFill() {
  sequenceSteps[1][14] = SEQ_STATE_HIT;
  sequenceSteps[1][15] = SEQ_STATE_ACCENT;
  sequenceSteps[2][12] = SEQ_STATE_HIT;
  sequenceSteps[2][13] = SEQ_STATE_RATCHET;
  sequenceSteps[2][14] = SEQ_STATE_ACCENT;
  sequenceSteps[2][15] = SEQ_STATE_HIT;
  sequenceSteps[3][15] = SEQ_STATE_ACCENT;
}

void generateSequencerGroove() {
  stopSequencerNotes();
  clearSequencerPattern();

  sequenceSteps[0][0] = SEQ_STATE_ACCENT;
  if (sequencerDensity > 38) sequenceSteps[0][8] = random(100) < 70 ? SEQ_STATE_ACCENT : SEQ_STATE_HIT;
  if (sequencerDensity > 48 && random(100) < 45) sequenceSteps[0][6] = SEQ_STATE_HIT;
  if (sequencerDensity > 58 && random(100) < 65) sequenceSteps[0][10] = SEQ_STATE_HIT;
  if (sequencerDensity > 68) sequenceSteps[0][14] = SEQ_STATE_HIT;

  sequenceSteps[1][4] = SEQ_STATE_ACCENT;
  sequenceSteps[1][12] = SEQ_STATE_ACCENT;
  if (sequencerDensity > 54 && random(100) < 65) sequenceSteps[1][7] = SEQ_STATE_HIT;
  if (sequencerDensity > 62 && random(100) < 70) sequenceSteps[1][15] = SEQ_STATE_RATCHET;

  int hatStep = sequencerDensity > 64 ? 1 : 2;
  for (int step = 0; step < SEQ_STEPS; step += hatStep) {
    if (random(100) < sequencerDensity + 16) {
      sequenceSteps[2][step] = (step % 4 == 0) ? SEQ_STATE_HIT : SEQ_STATE_ACCENT;
    }
  }
  if (sequencerDensity > 72) {
    sequenceSteps[2][11] = SEQ_STATE_RATCHET;
    sequenceSteps[2][15] = SEQ_STATE_RATCHET;
  }

  for (int step = 2; step < SEQ_STEPS; step += 4) {
    if (random(100) < sequencerDensity - 8) {
      sequenceSteps[3][step] = random(100) < 20 ? SEQ_STATE_ACCENT : SEQ_STATE_HIT;
    }
  }
}

void varySequencerGroove() {
  for (int track = 0; track < SEQ_TRACKS; track++) {
    for (int step = 0; step < SEQ_STEPS; step++) {
      bool protectedStep = (track == 0 && step == 0) || (track == 1 && (step == 4 || step == 12));
      if (protectedStep || random(100) >= 18) continue;

      if (sequenceSteps[track][step] == SEQ_STATE_OFF) {
        int chance = sequencerDensity - (track == 2 ? 6 : 22);
        if (random(100) < chance) {
          sequenceSteps[track][step] = random(100) < 18 ? SEQ_STATE_RATCHET : SEQ_STATE_HIT;
        }
      } else if (random(100) < 45) {
        sequenceSteps[track][step] = SEQ_STATE_OFF;
      } else {
        sequenceSteps[track][step] = min(SEQ_STATE_RATCHET, sequenceSteps[track][step] + 1);
      }
    }
  }
}

void rotateSequencerPattern(int amount) {
  stopSequencerNotes();
  for (int track = 0; track < SEQ_TRACKS; track++) {
    uint8_t shifted[SEQ_STEPS];
    for (int step = 0; step < SEQ_STEPS; step++) {
      int source = (step - amount + SEQ_STEPS) % SEQ_STEPS;
      shifted[step] = sequenceSteps[track][source];
    }
    for (int step = 0; step < SEQ_STEPS; step++) {
      sequenceSteps[track][step] = shifted[step];
    }
  }
}

void updateSequencer() {
  unsigned long now = millis();

  for (int track = 0; track < SEQ_TRACKS; track++) {
    if (noteOffTime[track] > 0 && now >= noteOffTime[track]) {
      sendNote(performance.drumsChannel, seqDrumNotes[track], 0, false);
      noteOffTime[track] = 0;
    }
    if (sequencerPlaying && ratchetPending[track] && now >= ratchetTime[track]) {
      ratchetPending[track] = false;
      playSequencerHit(track, SEQ_STATE_HIT, now, max(24, seqNoteLengths[track] / 2));
    }
  }

  if (!sequencerPlaying) return;

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

  for (int track = 0; track < SEQ_TRACKS; track++) {
    int level = sequenceSteps[track][currentStep];
    if (level > SEQ_STATE_OFF && !trackMuted[track]) {
      if (level != SEQ_STATE_ACCENT && random(100) >= sequencerProbability) continue;
      playSequencerHit(track, level, now);
      if (level == SEQ_STATE_RATCHET) {
        ratchetPending[track] = true;
        ratchetTime[track] = now + max(32, (int)getSequencerStepInterval() / 2);
      }
    }
  }
}

void playSequencerHit(int track, int level, unsigned long now, int lengthOverride) {
  if (!deviceConnected) return;

  if (noteOffTime[track] > 0) {
    sendNote(performance.drumsChannel, seqDrumNotes[track], 0, false);
  }

  sendNote(performance.drumsChannel, seqDrumNotes[track], getStepVelocity(track, level), true);
  int length = lengthOverride > 0 ? lengthOverride : seqNoteLengths[track];
  noteOffTime[track] = now + length;
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
  int velocity = level == SEQ_STATE_ACCENT ? accentVelocities[track] : normalVelocities[track];
  if (level == SEQ_STATE_RATCHET) velocity = min(122, velocity + 10);
  if (sequencerHumanize > 0 && level != SEQ_STATE_ACCENT) {
    velocity += random(-sequencerHumanize, sequencerHumanize + 1);
  }
  return constrain(velocity, 24, 127);
}

void stopSequencerTrack(int track) {
  if (noteOffTime[track] > 0) {
    sendNote(performance.drumsChannel, seqDrumNotes[track], 0, false);
    noteOffTime[track] = 0;
  }
  ratchetPending[track] = false;
}

void stopSequencerNotes() {
  for (int track = 0; track < SEQ_TRACKS; track++) {
    stopSequencerTrack(track);
  }
}

#endif
