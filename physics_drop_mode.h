#ifndef PHYSICS_DROP_MODE_H
#define PHYSICS_DROP_MODE_H

#include "common_definitions.h"
#include "ui_elements.h"
#include "midi_utils.h"

// Physics Drop mode variables
struct DropBall {
  float x, y;
  float vx, vy;
  float gravity = 0.15;
  float bounce = 0.6;
  float friction = 0.98;
  uint16_t color;
  int size;
  bool active;
  unsigned long spawnTime;
  int note;
  String noteName;
  unsigned long noteOffTime;
};

struct Platform {
  float x, y;
  float w, h;
  float angle; // In radians
  uint16_t color;
  bool active;
  int note;
  String noteName;
  unsigned long activeTime;
  unsigned long noteOffTime;
  int flashVelocity;
};

#define MAX_DROP_BALLS 8
#define MAX_PLATFORMS 6
#define DROP_PRESETS 3
DropBall dropBalls[MAX_DROP_BALLS];
Platform platforms[MAX_PLATFORMS];
int numActiveDropBalls = 0;
int numPlatforms = 0;
int dropOctave = 4;
int dropGravity = 15;      // 10-30, displayed as percentage-ish control
int dropNoteLength = 180;  // ms base length
int dropPresetIndex = 0;
bool platformMode = false; // false = drop mode, true = platform edit mode
const char* dropPresetNames[DROP_PRESETS] = {"RISE", "STAIR", "BOWL"};

// Function declarations
void initializePhysicsDropMode();
void drawPhysicsDropMode();
void handlePhysicsDropMode();
void drawDropBalls();
void drawPlatforms();
void updatePhysics();
void spawnDropBall(int x, int y);
void addPlatform(int x, int y);
void checkPlatformCollisions();
void refreshPlatformNotes();
void drawDropButton(int x, int y, int w, int h, String text, uint16_t color, bool pressed = false);
void drawDropControls();
void applyDropPreset(int presetIndex);
void clearDropBalls();
void clearDropPlatforms();
void updateDropNotes();
void triggerDropNote(int note, int velocity, unsigned long &noteOffTime);
void stopDropNotes();
int getDropVelocity(float speed);
int getDropBallNote(int x);

// Implementations
void initializePhysicsDropMode() {
  numActiveDropBalls = 0;
  numPlatforms = 0;
  dropOctave = 4;
  dropGravity = 15;
  dropNoteLength = 180;
  dropPresetIndex = 0;
  platformMode = false;
  
  // Initialize all balls as inactive
  for (int i = 0; i < MAX_DROP_BALLS; i++) {
    dropBalls[i].active = false;
    dropBalls[i].noteOffTime = 0;
  }

  applyDropPreset(dropPresetIndex);
}

void drawPhysicsDropMode() {
  tft.fillScreen(THEME_BG);
  drawHeader("DROP", platformMode ? "Edit Platforms" : "Tap to Drop");

  tft.fillRoundRect(6, 50, 308, 134, 6, THEME_PANEL);
  tft.drawRoundRect(6, 50, 308, 134, 6, THEME_BORDER);

  tft.fillRoundRect(8, 187, 304, 12, 3, THEME_PANEL);
  tft.setTextColor(THEME_TEXT_DIM, THEME_PANEL);
  tft.drawCentreString(getRootName() + " " + scales[performance.scale].name + "  Oct " + String(dropOctave) +
                       "  G" + String(dropGravity) + " L" + String(dropNoteLength), 160, 189, 1);
  
  drawPlatforms();
  drawDropBalls();
  drawDropControls();
}

void drawDropButton(int x, int y, int w, int h, String text, uint16_t color, bool pressed) {
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

void drawDropControls() {
  tft.fillRect(0, 200, 320, 40, THEME_BG);

  drawDropButton(8, 202, 40, 16, platformMode ? "DROP" : "EDIT", THEME_WARNING);
  drawDropButton(54, 202, 42, 16, "PRE", THEME_ACCENT);
  drawDropButton(104, 202, 34, 16, "CLR", THEME_ERROR);
  drawDropButton(146, 202, 26, 16, "G-", THEME_SECONDARY);
  drawDropButton(178, 202, 26, 16, "G+", THEME_SECONDARY);
  drawDropButton(212, 202, 34, 16, "L-", THEME_SECONDARY);
  drawDropButton(254, 202, 34, 16, "L+", THEME_SECONDARY);

  drawDropButton(8, 222, 46, 16, "SCALE", THEME_ACCENT);
  drawDropButton(62, 222, 34, 16, "K-", THEME_SECONDARY);
  drawDropButton(104, 222, 34, 16, "K+", THEME_SECONDARY);
  drawDropButton(146, 222, 28, 16, "O", THEME_PRIMARY);

  tft.fillRoundRect(184, 222, 128, 16, 4, THEME_PANEL);
  tft.drawRoundRect(184, 222, 128, 16, 4, THEME_BORDER);
  tft.setTextColor(THEME_TEXT_DIM, THEME_PANEL);
  tft.drawCentreString(String(dropPresetNames[dropPresetIndex]) + "  B" + String(numActiveDropBalls) +
                       " P" + String(numPlatforms), 248, 226, 1);
}

void drawDropBalls() {
  for (int i = 0; i < MAX_DROP_BALLS; i++) {
    if (!dropBalls[i].active) continue;
    
    // Fade out old balls
    unsigned long age = millis() - dropBalls[i].spawnTime;
    if (age > 5000) {
      if (dropBalls[i].noteOffTime > 0) {
        sendNote(performance.generativeChannel, dropBalls[i].note, 0, false);
        dropBalls[i].noteOffTime = 0;
      }
      dropBalls[i].active = false;
      numActiveDropBalls--;
      continue;
    }
    
    tft.fillCircle(dropBalls[i].x, dropBalls[i].y, dropBalls[i].size, dropBalls[i].color);
    tft.drawCircle(dropBalls[i].x, dropBalls[i].y, dropBalls[i].size, THEME_TEXT);
  }
}

void drawPlatforms() {
  for (int i = 0; i < numPlatforms; i++) {
    uint16_t color = platforms[i].color;
    
    // Flash when hit
    if (platforms[i].active) {
      unsigned long elapsed = millis() - platforms[i].activeTime;
      if (elapsed < 200) {
        color = platforms[i].flashVelocity > 100 ? THEME_TEXT : THEME_ACCENT;
      } else {
        platforms[i].active = false;
      }
    }
    
    // Draw angled rectangle (simplified as normal rectangle for now)
    tft.fillRect(platforms[i].x, platforms[i].y, platforms[i].w, platforms[i].h, color);
    tft.drawRect(platforms[i].x, platforms[i].y, platforms[i].w, platforms[i].h, THEME_BORDER);
    
    // Show note name
    tft.setTextColor(THEME_BG, color);
    tft.drawCentreString(platforms[i].noteName, 
                        platforms[i].x + platforms[i].w/2, 
                        platforms[i].y + platforms[i].h/2 - 4, 1);
  }
}

void handlePhysicsDropMode() {
  // Back button
  if (touch.justPressed && isButtonPressed(10, 10, 50, 25)) {
    stopDropNotes();
    exitToMenu();
    return;
  }
  
  if (touch.justPressed) {
    // Mode toggle
    if (isButtonPressed(8, 202, 40, 16)) {
      platformMode = !platformMode;
      drawPhysicsDropMode();
      return;
    }

    if (isButtonPressed(54, 202, 42, 16)) {
      stopDropNotes();
      dropPresetIndex = (dropPresetIndex + 1) % DROP_PRESETS;
      applyDropPreset(dropPresetIndex);
      drawPhysicsDropMode();
      return;
    }
    
    // Clear button
    if (isButtonPressed(104, 202, 34, 16)) {
      stopDropNotes();
      clearDropBalls();
      if (platformMode) clearDropPlatforms();
      drawPhysicsDropMode();
      return;
    }

    if (isButtonPressed(146, 202, 26, 16)) {
      dropGravity = max(8, dropGravity - 2);
      drawPhysicsDropMode();
      return;
    }

    if (isButtonPressed(178, 202, 26, 16)) {
      dropGravity = min(30, dropGravity + 2);
      drawPhysicsDropMode();
      return;
    }

    if (isButtonPressed(212, 202, 34, 16)) {
      dropNoteLength = max(60, dropNoteLength - 60);
      drawPhysicsDropMode();
      return;
    }

    if (isButtonPressed(254, 202, 34, 16)) {
      dropNoteLength = min(720, dropNoteLength + 60);
      drawPhysicsDropMode();
      return;
    }
    
    // Scale button
    if (isButtonPressed(8, 222, 46, 16)) {
      stopDropNotes();
      nudgeGlobalScale(1);
      refreshPlatformNotes();
      drawPhysicsDropMode();
      return;
    }
    
    // Key controls
    if (isButtonPressed(62, 222, 34, 16)) {
      stopDropNotes();
      nudgeGlobalRoot(-1);
      refreshPlatformNotes();
      drawPhysicsDropMode();
      return;
    }
    
    if (isButtonPressed(104, 222, 34, 16)) {
      stopDropNotes();
      nudgeGlobalRoot(1);
      refreshPlatformNotes();
      drawPhysicsDropMode();
      return;
    }
    
    // Octave button
    if (isButtonPressed(146, 222, 28, 16)) {
      stopDropNotes();
      dropOctave = (dropOctave == 7) ? 2 : dropOctave + 1;
      refreshPlatformNotes();
      drawPhysicsDropMode();
      return;
    }
    
    // Touch in play area
    if (touch.y >= 54 && touch.y <= 180) {
      if (platformMode) {
        // Add platform
        addPlatform(touch.x, touch.y);
      } else {
        // Drop ball
        spawnDropBall(touch.x, touch.y);
      }
      return;
    }
  }
  
  // Update physics
  updatePhysics();
}

void spawnDropBall(int x, int y) {
  if (numActiveDropBalls >= MAX_DROP_BALLS) return;
  
  // Find inactive ball slot
  for (int i = 0; i < MAX_DROP_BALLS; i++) {
    if (!dropBalls[i].active) {
      dropBalls[i].x = x;
      dropBalls[i].y = y;
      dropBalls[i].vx = random(-10, 11) / 10.0; // -1 to 1
      dropBalls[i].vy = 0;
      dropBalls[i].gravity = dropGravity / 100.0;
      dropBalls[i].color = random(0x2000, 0x8FFF);
      dropBalls[i].size = random(3, 6);
      dropBalls[i].active = true;
      dropBalls[i].spawnTime = millis();
      dropBalls[i].note = getDropBallNote(x);
      dropBalls[i].noteName = getNoteNameFromMIDI(dropBalls[i].note);
      dropBalls[i].noteOffTime = 0;
      numActiveDropBalls++;
      break;
    }
  }
}

void addPlatform(int x, int y) {
  if (numPlatforms >= MAX_PLATFORMS) return;
  
  platforms[numPlatforms].x = x - 25;
  platforms[numPlatforms].y = y - 4;
  platforms[numPlatforms].w = 50;
  platforms[numPlatforms].h = 8;
  platforms[numPlatforms].angle = random(-5, 6) / 10.0; // -0.5 to 0.5 radians
  platforms[numPlatforms].color = random(0x2000, 0xFFFF);
  platforms[numPlatforms].active = false;
  platforms[numPlatforms].note = getNoteInScale(performance.scale, numPlatforms % 8, dropOctave);
  platforms[numPlatforms].noteName = getNoteNameFromMIDI(platforms[numPlatforms].note);
  platforms[numPlatforms].activeTime = 0;
  platforms[numPlatforms].noteOffTime = 0;
  platforms[numPlatforms].flashVelocity = 0;
  numPlatforms++;
  
  drawPhysicsDropMode();
}

void updatePhysics() {
  updateDropNotes();

  static unsigned long lastUpdate = 0;
  static float lastX[MAX_DROP_BALLS], lastY[MAX_DROP_BALLS];
  static bool initialized = false;
  
  if (millis() - lastUpdate < 50) return; // 20 FPS to reduce flickering
  
  // Initialize last positions
  if (!initialized) {
    for (int i = 0; i < MAX_DROP_BALLS; i++) {
      lastX[i] = dropBalls[i].x;
      lastY[i] = dropBalls[i].y;
    }
    initialized = true;
  }
  
  // Clear previous ball positions only
  for (int i = 0; i < MAX_DROP_BALLS; i++) {
    if (dropBalls[i].active) {
      tft.fillCircle(lastX[i], lastY[i], dropBalls[i].size + 1, THEME_PANEL);
    }
  }
  
  for (int i = 0; i < MAX_DROP_BALLS; i++) {
    if (!dropBalls[i].active) continue;
    
    // Store last position
    lastX[i] = dropBalls[i].x;
    lastY[i] = dropBalls[i].y;
    
    // Apply gravity
    dropBalls[i].gravity = dropGravity / 100.0;
    dropBalls[i].vy += dropBalls[i].gravity;
    
    // Apply friction
    dropBalls[i].vx *= dropBalls[i].friction;
    
    // Update position
    dropBalls[i].x += dropBalls[i].vx;
    dropBalls[i].y += dropBalls[i].vy;
    
    // Boundary collisions
    if (dropBalls[i].x - dropBalls[i].size <= 10) {
      dropBalls[i].x = 10 + dropBalls[i].size;
      dropBalls[i].vx = -dropBalls[i].vx * dropBalls[i].bounce;
    }
    if (dropBalls[i].x + dropBalls[i].size >= 310) {
      dropBalls[i].x = 310 - dropBalls[i].size;
      dropBalls[i].vx = -dropBalls[i].vx * dropBalls[i].bounce;
    }
    if (dropBalls[i].y + dropBalls[i].size >= 175) {
      dropBalls[i].y = 175 - dropBalls[i].size;
      dropBalls[i].vy = -dropBalls[i].vy * dropBalls[i].bounce;
      
      // Ground hit - play note
      if (deviceConnected && abs(dropBalls[i].vy) > 1) {
        triggerDropNote(dropBalls[i].note, getDropVelocity(abs(dropBalls[i].vy)), dropBalls[i].noteOffTime);
      }
    }
  }
  
  checkPlatformCollisions();
  
  // Redraw platforms (they don't move so less flickering)
  drawPlatforms();
  // Draw balls at new positions
  drawDropBalls();
  
  lastUpdate = millis();
}

void checkPlatformCollisions() {
  for (int b = 0; b < MAX_DROP_BALLS; b++) {
    if (!dropBalls[b].active) continue;
    
    for (int p = 0; p < numPlatforms; p++) {
      // Simple rectangle collision
      if (dropBalls[b].x + dropBalls[b].size >= platforms[p].x &&
          dropBalls[b].x - dropBalls[b].size <= platforms[p].x + platforms[p].w &&
          dropBalls[b].y + dropBalls[b].size >= platforms[p].y &&
          dropBalls[b].y - dropBalls[b].size <= platforms[p].y + platforms[p].h &&
          dropBalls[b].vy > 0) { // Only if falling down
        
        // Bounce off platform
        dropBalls[b].y = platforms[p].y - dropBalls[b].size;
        dropBalls[b].vy = -abs(dropBalls[b].vy) * dropBalls[b].bounce;
        dropBalls[b].vx += platforms[p].angle * 1.5; // Platform angle affects bounce
        
        // Play platform note
        if (!platforms[p].active) {
          int velocity = getDropVelocity(abs(dropBalls[b].vy));
          triggerDropNote(platforms[p].note, velocity, platforms[p].noteOffTime);
          platforms[p].active = true;
          platforms[p].activeTime = millis();
          platforms[p].flashVelocity = velocity;
        }
        
        break;
      }
    }
  }
}

void refreshPlatformNotes() {
  for (int i = 0; i < numPlatforms; i++) {
    platforms[i].note = getNoteInScale(performance.scale, i % 8, dropOctave);
    platforms[i].noteName = getNoteNameFromMIDI(platforms[i].note);
  }
}

void applyDropPreset(int presetIndex) {
  clearDropBalls();
  clearDropPlatforms();

  switch (presetIndex) {
    case 0:
      platforms[0] = {72, 160, 68, 8, 0.2, THEME_PRIMARY, false, 60, "C4", 0, 0, 0};
      platforms[1] = {178, 140, 58, 8, -0.3, THEME_SECONDARY, false, 64, "E4", 0, 0, 0};
      platforms[2] = {118, 118, 48, 8, 0.1, THEME_ACCENT, false, 67, "G4", 0, 0, 0};
      numPlatforms = 3;
      break;
    case 1:
      platforms[0] = {52, 166, 46, 8, 0.0, THEME_PRIMARY, false, 60, "C4", 0, 0, 0};
      platforms[1] = {104, 146, 46, 8, 0.0, THEME_SECONDARY, false, 64, "E4", 0, 0, 0};
      platforms[2] = {156, 126, 46, 8, 0.0, THEME_ACCENT, false, 67, "G4", 0, 0, 0};
      platforms[3] = {208, 106, 46, 8, 0.0, THEME_WARNING, false, 72, "C5", 0, 0, 0};
      numPlatforms = 4;
      break;
    case 2:
      platforms[0] = {58, 150, 62, 8, -0.35, THEME_PRIMARY, false, 60, "C4", 0, 0, 0};
      platforms[1] = {132, 164, 58, 8, 0.0, THEME_ACCENT, false, 67, "G4", 0, 0, 0};
      platforms[2] = {202, 150, 62, 8, 0.35, THEME_SECONDARY, false, 64, "E4", 0, 0, 0};
      platforms[3] = {126, 112, 72, 8, 0.0, THEME_WARNING, false, 72, "C5", 0, 0, 0};
      numPlatforms = 4;
      break;
  }
  refreshPlatformNotes();
}

void clearDropBalls() {
  for (int i = 0; i < MAX_DROP_BALLS; i++) {
    dropBalls[i].active = false;
    dropBalls[i].noteOffTime = 0;
  }
  numActiveDropBalls = 0;
}

void clearDropPlatforms() {
  for (int i = 0; i < MAX_PLATFORMS; i++) {
    platforms[i].active = false;
    platforms[i].noteOffTime = 0;
    platforms[i].flashVelocity = 0;
  }
  numPlatforms = 0;
}

void updateDropNotes() {
  if (!deviceConnected) return;

  unsigned long now = millis();
  for (int i = 0; i < MAX_DROP_BALLS; i++) {
    if (dropBalls[i].noteOffTime > 0 && now >= dropBalls[i].noteOffTime) {
      sendNote(performance.generativeChannel, dropBalls[i].note, 0, false);
      dropBalls[i].noteOffTime = 0;
    }
  }
  for (int i = 0; i < MAX_PLATFORMS; i++) {
    if (platforms[i].noteOffTime > 0 && now >= platforms[i].noteOffTime) {
      sendNote(performance.generativeChannel, platforms[i].note, 0, false);
      platforms[i].noteOffTime = 0;
    }
  }
}

void triggerDropNote(int note, int velocity, unsigned long &noteOffTime) {
  if (!deviceConnected) return;

  if (noteOffTime > 0) {
    sendNote(performance.generativeChannel, note, 0, false);
  }
  sendNote(performance.generativeChannel, note, velocity, true);
  noteOffTime = millis() + dropNoteLength + velocity;
}

void stopDropNotes() {
  for (int i = 0; i < MAX_DROP_BALLS; i++) {
    if (dropBalls[i].noteOffTime > 0) {
      sendNote(performance.generativeChannel, dropBalls[i].note, 0, false);
      dropBalls[i].noteOffTime = 0;
    }
  }
  for (int i = 0; i < MAX_PLATFORMS; i++) {
    if (platforms[i].noteOffTime > 0) {
      sendNote(performance.generativeChannel, platforms[i].note, 0, false);
      platforms[i].noteOffTime = 0;
    }
  }
}

int getDropVelocity(float speed) {
  int velocity = 54 + (int)(speed * 20.0);
  return constrain(velocity, 48, 122);
}

int getDropBallNote(int x) {
  int stableDegrees[] = {0, 2, 4, 5, 4, 2, 0, 6};
  int degree = map(constrain(x, 10, 310), 10, 310, 0, 7);
  return getNoteInScale(performance.scale, stableDegrees[degree], dropOctave);
}

#endif
