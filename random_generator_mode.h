#ifndef RANDOM_GENERATOR_MODE_H
#define RANDOM_GENERATOR_MODE_H

#include "common_definitions.h"
#include "ui_elements.h"
#include "midi_utils.h"

#define RNG_MAX_STEPS 16
#define RNG_REGIONS 4

// Random Generator mode variables
struct RandomGen {
  int density = 55;       // 0-100%
  int variation = 25;     // 0-100%
  int gate = 65;          // Percent of step length
  int subdivision = 8;    // 4=quarter, 8=eighth, 16=sixteenth
  int phraseLength = 8;   // 4, 8, 16
  int region = 1;         // 0=bass, 1=mid, 2=lead, 3=spark
  bool isPlaying = false;
  unsigned long nextStepTime = 0;
  unsigned long noteOffTime = 0;
  unsigned long noteInterval = 500;
  int currentStep = 0;
  int currentNote = -1;
  int phraseDegrees[RNG_MAX_STEPS];
  int phraseOctaves[RNG_MAX_STEPS];
};

RandomGen randomGen;
const char* rngRegionNames[RNG_REGIONS] = {"BASS", "MID", "LEAD", "SPRK"};

// Function declarations
void initializeRandomGeneratorMode();
void drawRandomGeneratorMode();
void handleRandomGeneratorMode();
void drawRandomGenControls();
void drawRngButton(int x, int y, int w, int h, String text, uint16_t color, bool pressed = false);
void updateRandomGenerator();
void playRandomStep();
void stopRandomNote();
void generateRandomPhrase();
void varyRandomPhrase();
void mutateRandomStep(int step);
void cycleRandomSubdivision(int amount);
void cycleRandomPhraseLength();
void cycleRandomRegion();
void calculateNoteInterval();
int getRandomWeightedDegree();
int getRandomRegionOctave();
int getRandomStepNote(int step);
int getRandomVelocity(int degree, int step);
String getRandomSubdivisionText();

// Implementations
void initializeRandomGeneratorMode() {
  randomGen.density = 55;
  randomGen.variation = 25;
  randomGen.gate = 65;
  randomGen.subdivision = 8;
  randomGen.phraseLength = 8;
  randomGen.region = 1;
  randomGen.isPlaying = false;
  randomGen.currentNote = -1;
  randomGen.currentStep = 0;
  randomGen.noteOffTime = 0;
  calculateNoteInterval();
  generateRandomPhrase();
  randomGen.nextStepTime = millis() + randomGen.noteInterval;
}

void drawRandomGeneratorMode() {
  calculateNoteInterval();
  tft.fillScreen(THEME_BG);
  drawHeader("RNG JAMS", getRootName() + " " + scales[performance.scale].name + "  " + String(performance.bpm) + " BPM");

  drawRandomGenControls();
}

void drawRngButton(int x, int y, int w, int h, String text, uint16_t color, bool pressed) {
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

void drawRandomGenControls() {
  tft.fillRoundRect(6, 50, 308, 178, 6, THEME_PANEL);
  tft.drawRoundRect(6, 50, 308, 178, 6, THEME_BORDER);

  int y = 56;
  drawRngButton(14, y, 50, 24, randomGen.isPlaying ? "STOP" : "PLAY",
                randomGen.isPlaying ? THEME_ERROR : THEME_SUCCESS);
  drawRngButton(70, y, 42, 24, "NEW", THEME_ACCENT);
  drawRngButton(118, y, 42, 24, "VAR", THEME_WARNING);
  drawRngButton(168, y, 56, 24, rngRegionNames[randomGen.region], THEME_PRIMARY);
  drawRngButton(232, y, 32, 24, "LEN", THEME_SECONDARY);
  tft.fillRoundRect(272, y, 34, 24, 5, THEME_BG);
  tft.drawRoundRect(272, y, 34, 24, 5, THEME_BORDER);
  tft.setTextColor(THEME_TEXT, THEME_BG);
  tft.drawCentreString(String(randomGen.phraseLength), 289, y + 8, 1);

  y += 30;
  tft.setTextColor(THEME_TEXT_DIM, THEME_PANEL);
  tft.drawString("ROOT", 16, y + 8, 1);
  drawRngButton(52, y, 24, 24, "-", THEME_SECONDARY);
  tft.fillRoundRect(82, y, 34, 24, 5, THEME_BG);
  tft.setTextColor(THEME_TEXT, THEME_BG);
  tft.drawCentreString(getRootName(), 99, y + 8, 1);
  drawRngButton(122, y, 24, 24, "+", THEME_SECONDARY);
  drawRngButton(156, y, 66, 24, scales[performance.scale].name, THEME_ACCENT);
  drawRngButton(232, y, 28, 24, "B-", THEME_SECONDARY);
  drawRngButton(264, y, 34, 24, "B+", THEME_SECONDARY);

  y += 30;
  tft.setTextColor(THEME_TEXT_DIM, THEME_PANEL);
  tft.drawString("DENS", 16, y + 8, 1);
  drawRngButton(54, y, 24, 24, "-", THEME_SECONDARY);
  tft.setTextColor(THEME_TEXT, THEME_PANEL);
  tft.drawCentreString(String(randomGen.density), 98, y + 8, 1);
  drawRngButton(120, y, 24, 24, "+", THEME_SECONDARY);
  tft.setTextColor(THEME_TEXT_DIM, THEME_PANEL);
  tft.drawString("VAR", 160, y + 8, 1);
  drawRngButton(190, y, 24, 24, "-", THEME_SECONDARY);
  tft.setTextColor(THEME_TEXT, THEME_PANEL);
  tft.drawCentreString(String(randomGen.variation), 232, y + 8, 1);
  drawRngButton(256, y, 24, 24, "+", THEME_SECONDARY);

  y += 30;
  tft.setTextColor(THEME_TEXT_DIM, THEME_PANEL);
  tft.drawString("GATE", 16, y + 8, 1);
  drawRngButton(54, y, 24, 24, "-", THEME_SECONDARY);
  tft.setTextColor(THEME_TEXT, THEME_PANEL);
  tft.drawCentreString(String(randomGen.gate), 98, y + 8, 1);
  drawRngButton(120, y, 24, 24, "+", THEME_SECONDARY);
  tft.setTextColor(THEME_TEXT_DIM, THEME_PANEL);
  tft.drawString("BEAT", 160, y + 8, 1);
  drawRngButton(198, y, 24, 24, "<", THEME_SECONDARY);
  tft.setTextColor(THEME_TEXT, THEME_PANEL);
  tft.drawCentreString(getRandomSubdivisionText(), 244, y + 8, 1);
  drawRngButton(278, y, 24, 24, ">", THEME_SECONDARY);

  y += 31;
  tft.fillRoundRect(14, y, 292, 34, 5, THEME_BG);
  int stepW = 16;
  int gap = 2;
  int startX = 20;
  for (int i = 0; i < RNG_MAX_STEPS; i++) {
    int x = startX + i * (stepW + gap);
    bool inPhrase = i < randomGen.phraseLength;
    bool current = randomGen.isPlaying && i == randomGen.currentStep;
    bool rest = randomGen.phraseDegrees[i] < 0;
    uint16_t fill = !inPhrase ? THEME_PANEL : (rest ? THEME_BG : THEME_PRIMARY);
    uint16_t border = current ? THEME_TEXT : (inPhrase ? THEME_BORDER : THEME_PANEL);
    tft.fillRoundRect(x, y + 5, stepW, 20, 3, fill);
    tft.drawRoundRect(x, y + 5, stepW, 20, 3, border);
    if (inPhrase && !rest) {
      tft.setTextColor(fill == THEME_PRIMARY ? THEME_BG : THEME_TEXT_DIM, fill);
      tft.drawCentreString(String((randomGen.phraseDegrees[i] % 7) + 1), x + stepW / 2, y + 11, 1);
    }
  }

  if (randomGen.currentNote != -1) {
    tft.setTextColor(THEME_ACCENT, THEME_BG);
    tft.drawCentreString(getNoteNameFromMIDI(randomGen.currentNote), 278, y + 24, 1);
  }
}

void handleRandomGeneratorMode() {
  if (touch.justPressed && isButtonPressed(10, 10, 50, 25)) {
    stopRandomNote();
    randomGen.isPlaying = false;
    exitToMenu();
    return;
  }

  if (touch.justPressed) {
    int y = 56;
    if (isButtonPressed(14, y, 50, 24)) {
      randomGen.isPlaying = !randomGen.isPlaying;
      if (randomGen.isPlaying) {
        calculateNoteInterval();
        randomGen.nextStepTime = millis();
      } else {
        stopRandomNote();
      }
      drawRandomGenControls();
      return;
    }
    if (isButtonPressed(70, y, 42, 24)) {
      stopRandomNote();
      generateRandomPhrase();
      drawRandomGenControls();
      return;
    }
    if (isButtonPressed(118, y, 42, 24)) {
      varyRandomPhrase();
      drawRandomGenControls();
      return;
    }
    if (isButtonPressed(168, y, 56, 24)) {
      cycleRandomRegion();
      drawRandomGenControls();
      return;
    }
    if (isButtonPressed(232, y, 32, 24)) {
      cycleRandomPhraseLength();
      drawRandomGenControls();
      return;
    }

    y += 30;
    if (isButtonPressed(52, y, 24, 24)) {
      stopRandomNote();
      nudgeGlobalRoot(-1);
      drawRandomGeneratorMode();
      return;
    }
    if (isButtonPressed(122, y, 24, 24)) {
      stopRandomNote();
      nudgeGlobalRoot(1);
      drawRandomGeneratorMode();
      return;
    }
    if (isButtonPressed(156, y, 66, 24)) {
      stopRandomNote();
      nudgeGlobalScale(1);
      generateRandomPhrase();
      drawRandomGeneratorMode();
      return;
    }
    if (isButtonPressed(232, y, 28, 24)) {
      nudgeGlobalBpm(-5);
      calculateNoteInterval();
      drawRandomGenControls();
      return;
    }
    if (isButtonPressed(264, y, 34, 24)) {
      nudgeGlobalBpm(5);
      calculateNoteInterval();
      drawRandomGenControls();
      return;
    }

    y += 30;
    if (isButtonPressed(54, y, 24, 24)) {
      randomGen.density = max(0, randomGen.density - 5);
      drawRandomGenControls();
      return;
    }
    if (isButtonPressed(120, y, 24, 24)) {
      randomGen.density = min(100, randomGen.density + 5);
      drawRandomGenControls();
      return;
    }
    if (isButtonPressed(190, y, 24, 24)) {
      randomGen.variation = max(0, randomGen.variation - 5);
      drawRandomGenControls();
      return;
    }
    if (isButtonPressed(256, y, 24, 24)) {
      randomGen.variation = min(100, randomGen.variation + 5);
      drawRandomGenControls();
      return;
    }

    y += 30;
    if (isButtonPressed(54, y, 24, 24)) {
      randomGen.gate = max(20, randomGen.gate - 10);
      drawRandomGenControls();
      return;
    }
    if (isButtonPressed(120, y, 24, 24)) {
      randomGen.gate = min(100, randomGen.gate + 10);
      drawRandomGenControls();
      return;
    }
    if (isButtonPressed(198, y, 24, 24)) {
      cycleRandomSubdivision(-1);
      calculateNoteInterval();
      drawRandomGenControls();
      return;
    }
    if (isButtonPressed(278, y, 24, 24)) {
      cycleRandomSubdivision(1);
      calculateNoteInterval();
      drawRandomGenControls();
      return;
    }
  }

  updateRandomGenerator();
}

void updateRandomGenerator() {
  if (randomGen.currentNote != -1 && randomGen.noteOffTime > 0 && millis() >= randomGen.noteOffTime) {
    stopRandomNote();
    drawRandomGenControls();
  }

  if (!randomGen.isPlaying || !deviceConnected) return;

  unsigned long now = millis();
  if (now >= randomGen.nextStepTime) {
    playRandomStep();
    randomGen.nextStepTime = now + randomGen.noteInterval;
  }
}

void playRandomStep() {
  stopRandomNote();

  int step = randomGen.currentStep;
  if (random(100) < randomGen.variation) {
    mutateRandomStep(step);
  }

  if (random(100) < randomGen.density && randomGen.phraseDegrees[step] >= 0) {
    int note = getRandomStepNote(step);
    if (note >= 0 && note <= 127) {
      int velocity = getRandomVelocity(randomGen.phraseDegrees[step], step);
      sendNote(performance.generativeChannel, note, velocity, true);
      randomGen.currentNote = note;
      randomGen.noteOffTime = millis() + ((randomGen.noteInterval * randomGen.gate) / 100);
      Serial.printf("RNG phrase note: %s\n", getNoteNameFromMIDI(note).c_str());
    }
  }

  randomGen.currentStep = (randomGen.currentStep + 1) % randomGen.phraseLength;
  drawRandomGenControls();
}

void stopRandomNote() {
  if (randomGen.currentNote != -1) {
    sendNote(performance.generativeChannel, randomGen.currentNote, 0, false);
    randomGen.currentNote = -1;
  }
  randomGen.noteOffTime = 0;
}

void generateRandomPhrase() {
  for (int i = 0; i < RNG_MAX_STEPS; i++) {
    mutateRandomStep(i);
  }
  randomGen.currentStep = 0;
}

void varyRandomPhrase() {
  int changes = max(1, randomGen.phraseLength / 4);
  for (int i = 0; i < changes; i++) {
    mutateRandomStep(random(randomGen.phraseLength));
  }
}

void mutateRandomStep(int step) {
  if (step < 0 || step >= RNG_MAX_STEPS) return;

  int restChance = 28;
  if (step == 0) restChance = 4;
  else if (step % 4 == 0) restChance = 14;

  if (random(100) < restChance) {
    randomGen.phraseDegrees[step] = -1;
  } else {
    randomGen.phraseDegrees[step] = getRandomWeightedDegree();
  }
  randomGen.phraseOctaves[step] = getRandomRegionOctave();
}

void cycleRandomSubdivision(int amount) {
  if (amount > 0) {
    if (randomGen.subdivision == 4) randomGen.subdivision = 8;
    else if (randomGen.subdivision == 8) randomGen.subdivision = 16;
  } else {
    if (randomGen.subdivision == 16) randomGen.subdivision = 8;
    else if (randomGen.subdivision == 8) randomGen.subdivision = 4;
  }
}

void cycleRandomPhraseLength() {
  if (randomGen.phraseLength == 4) randomGen.phraseLength = 8;
  else if (randomGen.phraseLength == 8) randomGen.phraseLength = 16;
  else randomGen.phraseLength = 4;
  randomGen.currentStep = 0;
}

void cycleRandomRegion() {
  randomGen.region = (randomGen.region + 1) % RNG_REGIONS;
  for (int i = 0; i < RNG_MAX_STEPS; i++) {
    randomGen.phraseOctaves[i] = getRandomRegionOctave();
  }
}

void calculateNoteInterval() {
  float beatsPerSecond = performance.bpm / 60.0;
  float notesPerSecond = beatsPerSecond * (randomGen.subdivision / 4.0);
  randomGen.noteInterval = 1000.0 / notesPerSecond;
}

int getRandomWeightedDegree() {
  int roll = random(100);
  if (roll < 24) return 0;  // root
  if (roll < 42) return 4;  // fifth
  if (roll < 57) return 2;  // third
  if (roll < 70) return 5;  // sixth
  if (roll < 81) return 3;  // fourth
  if (roll < 91) return 1;  // second
  return 6;                 // seventh
}

int getRandomRegionOctave() {
  switch (randomGen.region) {
    case 0: return random(2, 4); // bass
    case 1: return random(3, 6); // mid
    case 2: return random(4, 7); // lead
    case 3: return random(5, 8); // sparkle
  }
  return 4;
}

int getRandomStepNote(int step) {
  return getNoteInScale(performance.scale, randomGen.phraseDegrees[step], randomGen.phraseOctaves[step]);
}

int getRandomVelocity(int degree, int step) {
  int velocity = (degree == 0 || step == 0) ? 108 : 86;
  velocity += random(-10, 14);
  return constrain(velocity, 54, 120);
}

String getRandomSubdivisionText() {
  if (randomGen.subdivision == 4) return "1/4";
  if (randomGen.subdivision == 8) return "1/8";
  return "1/16";
}

#endif
