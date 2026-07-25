#ifndef ARPEGGIATOR_MODE_H
#define ARPEGGIATOR_MODE_H

#include "common_definitions.h"
#include "ui_elements.h"
#include "midi_utils.h"

#define ARP_DEGREES 8
#define ARP_PATTERN_COUNT 6
#define ARP_CHORD_MODE_COUNT 4

// Arpeggiator mode variables
struct Arpeggiator {
  int chordMode = 0; // 0=Diatonic triad, 1=Diatonic 7th, 2=Sus, 3=Fifth
  int pattern = 0;   // 0=Up, 1=Down, 2=UpDown, 3=Random, 4=Chance, 5=Pedal
  int octaves = 2;
  int speed = 8;     // 4, 8, 16, 32
  int gate = 70;     // Percent of step length
  int arpOctave = 4;
  bool isPlaying = false;
  int currentStep = 0;
  int currentNote = -1;
  int triggeredDegree = -1;
  unsigned long lastStepTime = 0;
  unsigned long noteOffTime = 0;
  unsigned long stepInterval = 125;
};

Arpeggiator arp;
const char* arpPatternNames[ARP_PATTERN_COUNT] = {"UP", "DOWN", "UP/DN", "RAND", "CHNC", "PEDAL"};
const char* arpChordModeNames[ARP_CHORD_MODE_COUNT] = {"DIA", "7TH", "SUS", "5TH"};
const char* arpDegreeNames[ARP_DEGREES] = {"I", "ii", "iii", "IV", "V", "vi", "vii", "I+"};

// Function declarations
void initializeArpeggiatorMode();
void drawArpeggiatorMode();
void handleArpeggiatorMode();
void drawArpControls();
void drawArpDegreePads();
void drawArpButton(int x, int y, int w, int h, String text, uint16_t color, bool pressed = false);
void startArpDegree(int degree);
void stopArp();
void updateArpeggiator();
void playArpNote();
int getArpNote();
int getArpStepIndex(int totalSteps);
int buildArpChord(int notes[]);
int getArpDegreeNote(int degree, int octave);
int getArpVelocity(int chordStep);
String getArpSpeedText();
void calculateStepInterval();

// Implementations
void initializeArpeggiatorMode() {
  arp.chordMode = 0;
  arp.pattern = 0;
  arp.octaves = 2;
  arp.speed = 8;
  arp.gate = 70;
  arp.arpOctave = 4;
  arp.isPlaying = false;
  arp.currentStep = 0;
  arp.currentNote = -1;
  arp.triggeredDegree = -1;
  arp.noteOffTime = 0;
  calculateStepInterval();
}

void drawArpeggiatorMode() {
  calculateStepInterval();
  tft.fillScreen(THEME_BG);
  drawHeader("ARP", getRootName() + " " + scales[performance.scale].name + "  " + String(performance.bpm) + " BPM");

  drawArpControls();
  drawArpDegreePads();
}

void drawArpButton(int x, int y, int w, int h, String text, uint16_t color, bool pressed) {
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

void drawArpControls() {
  int y = 52;
  int rowH = 24;
  int rowGap = 2;

  tft.fillRoundRect(6, 48, 308, 108, 6, THEME_PANEL);
  tft.drawRoundRect(6, 48, 308, 108, 6, THEME_BORDER);

  tft.setTextColor(THEME_TEXT_DIM, THEME_PANEL);
  tft.drawString("PAT", 12, y + 8, 1);
  drawArpButton(40, y, 58, rowH, arpPatternNames[arp.pattern], THEME_WARNING);
  drawArpButton(104, y, 24, rowH, "<", THEME_SECONDARY);
  drawArpButton(132, y, 24, rowH, ">", THEME_SECONDARY);
  tft.setTextColor(THEME_TEXT_DIM, THEME_PANEL);
  tft.drawString("CHD", 178, y + 8, 1);
  drawArpButton(208, y, 48, rowH, arpChordModeNames[arp.chordMode], THEME_ACCENT);
  drawArpButton(264, y, 42, rowH, arp.isPlaying ? "STOP" : "HOLD",
                arp.isPlaying ? THEME_ERROR : THEME_SUCCESS);

  y += rowH + rowGap;

  tft.setTextColor(THEME_TEXT_DIM, THEME_PANEL);
  tft.drawString("OCT", 12, y + 8, 1);
  drawArpButton(42, y, 24, rowH, "-", THEME_SECONDARY);
  tft.setTextColor(THEME_TEXT, THEME_PANEL);
  tft.drawCentreString(String(arp.octaves), 82, y + 8, 1);
  drawArpButton(98, y, 24, rowH, "+", THEME_SECONDARY);

  tft.setTextColor(THEME_TEXT_DIM, THEME_PANEL);
  tft.drawString("RATE", 142, y + 8, 1);
  drawArpButton(180, y, 24, rowH, "-", THEME_SECONDARY);
  tft.setTextColor(THEME_TEXT, THEME_PANEL);
  tft.drawCentreString(getArpSpeedText(), 226, y + 8, 1);
  drawArpButton(264, y, 24, rowH, "+", THEME_SECONDARY);

  y += rowH + rowGap;

  tft.setTextColor(THEME_TEXT_DIM, THEME_PANEL);
  tft.drawString("BPM", 12, y + 8, 1);
  drawArpButton(42, y, 24, rowH, "-", THEME_SECONDARY);
  tft.setTextColor(THEME_TEXT, THEME_PANEL);
  tft.drawCentreString(String(performance.bpm), 86, y + 8, 1);
  drawArpButton(112, y, 24, rowH, "+", THEME_SECONDARY);

  tft.setTextColor(THEME_TEXT_DIM, THEME_PANEL);
  tft.drawString("GATE", 158, y + 8, 1);
  drawArpButton(198, y, 24, rowH, "-", THEME_SECONDARY);
  tft.setTextColor(THEME_TEXT, THEME_PANEL);
  tft.drawCentreString(String(arp.gate), 244, y + 8, 1);
  drawArpButton(276, y, 24, rowH, "+", THEME_SECONDARY);

  y += rowH + rowGap;

  tft.setTextColor(THEME_TEXT_DIM, THEME_PANEL);
  tft.drawString("ROOT", 12, y + 8, 1);
  drawArpButton(50, y, 28, rowH, "-", THEME_SECONDARY);
  tft.setTextColor(THEME_TEXT, THEME_PANEL);
  tft.drawCentreString(getRootName(), 100, y + 8, 1);
  drawArpButton(124, y, 28, rowH, "+", THEME_SECONDARY);

  tft.setTextColor(THEME_TEXT_DIM, THEME_PANEL);
  tft.drawString("PAD", 174, y + 8, 1);
  drawArpButton(206, y, 28, rowH, "-", THEME_SECONDARY);
  tft.setTextColor(THEME_TEXT, THEME_PANEL);
  tft.drawCentreString("O" + String(arp.arpOctave), 256, y + 8, 1);
  drawArpButton(284, y, 24, rowH, "+", THEME_SECONDARY);

  tft.fillRoundRect(170, 132, 136, 18, 4, THEME_BG);
  if (arp.currentNote != -1) {
    tft.setTextColor(THEME_ACCENT, THEME_BG);
    tft.drawCentreString("NOTE " + getNoteNameFromMIDI(arp.currentNote), 238, 135, 1);
  } else {
    tft.setTextColor(THEME_TEXT_DIM, THEME_BG);
    tft.drawCentreString(arp.isPlaying ? "REST" : "READY", 238, 135, 1);
  }
}

void drawArpDegreePads() {
  int keyY = 164;
  int keyWidth = 37;
  int keyHeight = 58;
  int startX = 10;
  int gap = 2;
  uint16_t degreeColors[ARP_DEGREES] = {
    THEME_PRIMARY, THEME_SECONDARY, THEME_ACCENT, THEME_SUCCESS,
    THEME_WARNING, THEME_ERROR, 0xF81F, THEME_PRIMARY
  };

  tft.fillRoundRect(6, 158, 308, 72, 6, THEME_PANEL);
  tft.drawRoundRect(6, 158, 308, 72, 6, THEME_BORDER);

  for (int i = 0; i < ARP_DEGREES; i++) {
    int x = startX + i * (keyWidth + gap);
    bool active = arp.isPlaying && arp.triggeredDegree == i;
    uint16_t bgColor = active ? degreeColors[i] : THEME_BG;
    uint16_t textColor = active ? THEME_BG : THEME_TEXT;
    int degreeNote = getArpDegreeNote(i, arp.arpOctave);

    tft.fillRoundRect(x, keyY, keyWidth, keyHeight, 5, bgColor);
    tft.drawRoundRect(x, keyY, keyWidth, keyHeight, 5, active ? THEME_TEXT : degreeColors[i]);
    tft.setTextColor(textColor, bgColor);
    tft.drawCentreString(arpDegreeNames[i], x + keyWidth / 2, keyY + 12, 2);
    tft.setTextColor(active ? THEME_BG : THEME_TEXT_DIM, bgColor);
    tft.drawCentreString(getNoteNameFromMIDI(degreeNote), x + keyWidth / 2, keyY + 38, 1);
  }
}

void handleArpeggiatorMode() {
  if (touch.justPressed && isButtonPressed(10, 10, 50, 25)) {
    stopArp();
    exitToMenu();
    return;
  }

  if (touch.justPressed) {
    int y = 52;
    int rowH = 24;
    int rowGap = 2;

    if (isButtonPressed(104, y, 24, rowH)) {
      arp.pattern = (arp.pattern - 1 + ARP_PATTERN_COUNT) % ARP_PATTERN_COUNT;
      drawArpControls();
      return;
    }
    if (isButtonPressed(132, y, 24, rowH)) {
      arp.pattern = (arp.pattern + 1) % ARP_PATTERN_COUNT;
      drawArpControls();
      return;
    }
    if (isButtonPressed(208, y, 48, rowH)) {
      arp.chordMode = (arp.chordMode + 1) % ARP_CHORD_MODE_COUNT;
      arp.currentStep = 0;
      drawArpeggiatorMode();
      return;
    }
    if (isButtonPressed(264, y, 42, rowH)) {
      if (arp.isPlaying) {
        stopArp();
      } else if (arp.triggeredDegree != -1) {
        startArpDegree(arp.triggeredDegree);
      }
      drawArpeggiatorMode();
      return;
    }

    y += rowH + rowGap;

    if (isButtonPressed(42, y, 24, rowH)) {
      arp.octaves = max(1, arp.octaves - 1);
      arp.currentStep = 0;
      drawArpControls();
      return;
    }
    if (isButtonPressed(98, y, 24, rowH)) {
      arp.octaves = min(4, arp.octaves + 1);
      arp.currentStep = 0;
      drawArpControls();
      return;
    }
    if (isButtonPressed(180, y, 24, rowH)) {
      if (arp.speed == 4) arp.speed = 8;
      else if (arp.speed == 8) arp.speed = 16;
      else if (arp.speed == 16) arp.speed = 32;
      calculateStepInterval();
      drawArpControls();
      return;
    }
    if (isButtonPressed(264, y, 24, rowH)) {
      if (arp.speed == 32) arp.speed = 16;
      else if (arp.speed == 16) arp.speed = 8;
      else if (arp.speed == 8) arp.speed = 4;
      calculateStepInterval();
      drawArpControls();
      return;
    }

    y += rowH + rowGap;

    if (isButtonPressed(42, y, 24, rowH)) {
      nudgeGlobalBpm(-5);
      calculateStepInterval();
      drawArpControls();
      return;
    }
    if (isButtonPressed(112, y, 24, rowH)) {
      nudgeGlobalBpm(5);
      calculateStepInterval();
      drawArpControls();
      return;
    }
    if (isButtonPressed(198, y, 24, rowH)) {
      arp.gate = max(20, arp.gate - 10);
      drawArpControls();
      return;
    }
    if (isButtonPressed(276, y, 24, rowH)) {
      arp.gate = min(100, arp.gate + 10);
      drawArpControls();
      return;
    }

    y += rowH + rowGap;

    if (isButtonPressed(50, y, 28, rowH)) {
      nudgeGlobalRoot(-1);
      arp.currentStep = 0;
      drawArpeggiatorMode();
      return;
    }
    if (isButtonPressed(124, y, 28, rowH)) {
      nudgeGlobalRoot(1);
      arp.currentStep = 0;
      drawArpeggiatorMode();
      return;
    }
    if (isButtonPressed(206, y, 28, rowH)) {
      arp.arpOctave = max(2, arp.arpOctave - 1);
      drawArpeggiatorMode();
      return;
    }
    if (isButtonPressed(284, y, 24, rowH)) {
      arp.arpOctave = min(6, arp.arpOctave + 1);
      drawArpeggiatorMode();
      return;
    }

    int keyY = 164;
    int keyWidth = 37;
    int keyHeight = 58;
    int startX = 10;
    int gap = 2;
    for (int i = 0; i < ARP_DEGREES; i++) {
      int x = startX + i * (keyWidth + gap);
      if (isButtonPressed(x, keyY, keyWidth, keyHeight)) {
        if (arp.isPlaying && arp.triggeredDegree == i) {
          stopArp();
        } else {
          startArpDegree(i);
        }
        drawArpeggiatorMode();
        return;
      }
    }
  }

  updateArpeggiator();
}

void startArpDegree(int degree) {
  if (arp.currentNote != -1) {
    sendNote(performance.arpChannel, arp.currentNote, 0, false);
  }
  arp.triggeredDegree = degree;
  arp.isPlaying = true;
  arp.currentStep = 0;
  arp.currentNote = -1;
  arp.noteOffTime = 0;
  calculateStepInterval();
  playArpNote();
  arp.lastStepTime = millis();
}

void stopArp() {
  if (arp.currentNote != -1) {
    sendNote(performance.arpChannel, arp.currentNote, 0, false);
  }
  arp.isPlaying = false;
  arp.currentNote = -1;
  arp.noteOffTime = 0;
}

void updateArpeggiator() {
  if (arp.currentNote != -1 && arp.noteOffTime > 0 && millis() >= arp.noteOffTime) {
    sendNote(performance.arpChannel, arp.currentNote, 0, false);
    arp.currentNote = -1;
    arp.noteOffTime = 0;
    drawArpControls();
  }

  if (!arp.isPlaying) return;

  unsigned long now = millis();
  if (now - arp.lastStepTime >= arp.stepInterval) {
    playArpNote();
    arp.lastStepTime = now;
  }
}

void playArpNote() {
  if (arp.triggeredDegree == -1) return;

  if (arp.currentNote != -1) {
    sendNote(performance.arpChannel, arp.currentNote, 0, false);
    arp.currentNote = -1;
  }

  if (arp.pattern == 4 && random(100) < 30) {
    arp.currentStep++;
    arp.noteOffTime = 0;
    drawArpControls();
    return;
  }

  int chordNotes[4];
  int chordLength = buildArpChord(chordNotes);
  if (chordLength <= 0) return;

  int step = getArpStepIndex(chordLength * arp.octaves);
  int octaveOffset = step / chordLength;
  int chordStep = step % chordLength;
  int velocity = getArpVelocity(chordStep);

  arp.currentNote = chordNotes[chordStep] + (octaveOffset * 12);
  sendNote(performance.arpChannel, arp.currentNote, velocity, true);
  arp.noteOffTime = millis() + ((arp.stepInterval * arp.gate) / 100);
  arp.currentStep++;
  drawArpControls();
}

int getArpNote() {
  int chordNotes[4];
  int chordLength = buildArpChord(chordNotes);
  if (chordLength <= 0) return getArpDegreeNote(0, arp.arpOctave);

  int totalSteps = chordLength * arp.octaves;
  int step = getArpStepIndex(totalSteps);
  int octaveOffset = step / chordLength;
  int chordStep = step % chordLength;

  return chordNotes[chordStep] + (octaveOffset * 12);
}

int getArpStepIndex(int totalSteps) {
  if (totalSteps <= 1) return 0;

  switch (arp.pattern) {
    case 0: // Up
    case 4: // Chance
      return arp.currentStep % totalSteps;
    case 1: // Down
      return (totalSteps - 1) - (arp.currentStep % totalSteps);
    case 2: // Up/Down
      {
        int cycle = (totalSteps - 1) * 2;
        int pos = arp.currentStep % cycle;
        return pos < totalSteps ? pos : cycle - pos;
      }
    case 3: // Random
      return random(totalSteps);
    case 5: // Pedal
      if (arp.currentStep % 2 == 0) return 0;
      return 1 + ((arp.currentStep / 2) % (totalSteps - 1));
  }
  return arp.currentStep % totalSteps;
}

int buildArpChord(int notes[]) {
  int degree = arp.triggeredDegree == -1 ? 0 : arp.triggeredDegree;
  int length = 3;

  switch (arp.chordMode) {
    case 0: // Diatonic triad
      notes[0] = getArpDegreeNote(degree, arp.arpOctave);
      notes[1] = getArpDegreeNote(degree + 2, arp.arpOctave);
      notes[2] = getArpDegreeNote(degree + 4, arp.arpOctave);
      length = 3;
      break;
    case 1: // Diatonic seventh
      notes[0] = getArpDegreeNote(degree, arp.arpOctave);
      notes[1] = getArpDegreeNote(degree + 2, arp.arpOctave);
      notes[2] = getArpDegreeNote(degree + 4, arp.arpOctave);
      notes[3] = getArpDegreeNote(degree + 6, arp.arpOctave);
      length = 4;
      break;
    case 2: // Sus
      notes[0] = getArpDegreeNote(degree, arp.arpOctave);
      notes[1] = getArpDegreeNote(degree + 3, arp.arpOctave);
      notes[2] = getArpDegreeNote(degree + 4, arp.arpOctave);
      length = 3;
      break;
    case 3: // Fifth
      notes[0] = getArpDegreeNote(degree, arp.arpOctave);
      notes[1] = getArpDegreeNote(degree + 4, arp.arpOctave);
      notes[2] = getArpDegreeNote(degree + 7, arp.arpOctave);
      length = 3;
      break;
  }

  return length;
}

int getArpDegreeNote(int degree, int octave) {
  return getNoteInScale(performance.scale, degree, octave);
}

int getArpVelocity(int chordStep) {
  if (chordStep == 0) return 112;
  if (arp.pattern == 5 && arp.currentStep % 2 == 0) return 118;
  return 92;
}

String getArpSpeedText() {
  if (arp.speed == 4) return "1/4";
  if (arp.speed == 8) return "1/8";
  if (arp.speed == 16) return "1/16";
  return "1/32";
}

void calculateStepInterval() {
  float beatsPerSecond = performance.bpm / 60.0;
  float notesPerSecond = beatsPerSecond * (arp.speed / 4.0);
  arp.stepInterval = 1000.0 / notesPerSecond;
}

#endif
