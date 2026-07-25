#ifndef BOUNCING_BALL_MODE_H
#define BOUNCING_BALL_MODE_H

#include "common_definitions.h"
#include "ui_elements.h"
#include "midi_utils.h"

// Pong-style Ambient MIDI mode variables
struct Ball {
  float x, y;
  float vx, vy;
  uint16_t color;
  int size;
  bool active;
};

#define MAX_BALLS 4
Ball balls[MAX_BALLS];
int numActiveBalls = 1;

// Simple wall system - notes triggered by wall hits
struct Wall {
  int x, y, w, h;
  int note;
  String noteName;
  uint16_t color;
  bool active;
  unsigned long activeTime;
  unsigned long noteOffTime;
  int flashVelocity;
  int side; // 0=top, 1=right, 2=bottom, 3=left
};

#define NUM_WALLS 24  // 8 top + 8 bottom + 4 left + 4 right
Wall walls[NUM_WALLS];
int ballOctave = 4;
int zenDensity = 65;
int zenNoteLength = 240;

// Function declarations
void initializeBouncingBallMode();
void drawBouncingBallMode();
void handleBouncingBallMode();
void initializeBalls();
void initializeWalls();
void updateBouncingBall();
void updateBalls();
void drawBalls();
void drawWalls();
void checkWallCollisions();
void drawZenButton(int x, int y, int w, int h, String text, uint16_t color, bool pressed = false);
void drawZenControls();
void applyZenCalm();
void applyZenChaos();
void updateZenNotes();
void triggerZenWall(int wallIndex, float speed);
void stopZenNotes();
int getZenWallNote(int segment, int side);
int getZenVelocity(float speed);

// Implementations
void initializeBouncingBallMode() {
  ballOctave = 4;
  zenDensity = 65;
  zenNoteLength = 240;
  numActiveBalls = 1;
  initializeBalls();
  initializeWalls();
}

void drawBouncingBallMode() {
  tft.fillScreen(THEME_BG);
  drawHeader("ZEN", "Ambient Generative");

  tft.fillRoundRect(44, 54, 238, 132, 6, THEME_PANEL);
  tft.drawRoundRect(44, 54, 238, 132, 6, THEME_BORDER);

  tft.fillRoundRect(8, 188, 304, 12, 3, THEME_PANEL);
  tft.setTextColor(THEME_TEXT_DIM, THEME_PANEL);
  tft.drawCentreString(getRootName() + " " + scales[performance.scale].name +
                       "  D" + String(zenDensity) + " L" + String(zenNoteLength) +
                       "  B" + String(numActiveBalls), 160, 190, 1);
  
  drawWalls();
  drawBalls();
  drawZenControls();
}

void drawZenButton(int x, int y, int w, int h, String text, uint16_t color, bool pressed) {
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

void drawZenControls() {
  tft.fillRect(0, 200, 320, 40, THEME_BG);

  drawZenButton(8, 202, 36, 16, "ADD", THEME_SUCCESS);
  drawZenButton(50, 202, 44, 16, "CALM", THEME_PRIMARY);
  drawZenButton(100, 202, 48, 16, "CHAOS", THEME_ERROR);
  drawZenButton(156, 202, 26, 16, "D-", THEME_SECONDARY);
  drawZenButton(188, 202, 26, 16, "D+", THEME_SECONDARY);
  drawZenButton(222, 202, 34, 16, "L-", THEME_SECONDARY);
  drawZenButton(264, 202, 34, 16, "L+", THEME_SECONDARY);

  drawZenButton(8, 222, 44, 16, "RESET", THEME_WARNING);
  drawZenButton(60, 222, 46, 16, "SCALE", THEME_ACCENT);
  drawZenButton(114, 222, 34, 16, "KEY-", THEME_SECONDARY);
  drawZenButton(156, 222, 34, 16, "KEY+", THEME_SECONDARY);
  drawZenButton(198, 222, 28, 16, "O", THEME_PRIMARY);

  tft.fillRoundRect(236, 222, 76, 16, 4, THEME_PANEL);
  tft.drawRoundRect(236, 222, 76, 16, 4, THEME_BORDER);
  tft.setTextColor(THEME_TEXT_DIM, THEME_PANEL);
  tft.drawCentreString("Oct " + String(ballOctave), 274, 226, 1);
}

void initializeBalls() {
  for (int i = 0; i < MAX_BALLS; i++) {
    balls[i].x = random(80, 240);
    balls[i].y = random(80, 150);
    // Slower, more zen-like movement
    balls[i].vx = random(-15, 15) / 10.0; // -1.5 to 1.5
    balls[i].vy = random(-15, 15) / 10.0;
    if (abs(balls[i].vx) < 0.5) balls[i].vx = (balls[i].vx >= 0) ? 0.8 : -0.8;
    if (abs(balls[i].vy) < 0.5) balls[i].vy = (balls[i].vy >= 0) ? 0.8 : -0.8;
    // Softer, more zen colors
    balls[i].color = random(0x2000, 0x8FFF);
    balls[i].size = random(4, 7);
    balls[i].active = (i < numActiveBalls);
  }
}

void initializeWalls() {
  int wallIndex = 0;
  
  // Top wall - 8 segments
  for (int i = 0; i < 8; i++) {
    walls[wallIndex].x = 50 + i * 28;
    walls[wallIndex].y = 60;
    walls[wallIndex].w = 28;
    walls[wallIndex].h = 3;
    walls[wallIndex].note = getZenWallNote(i, 0);
    walls[wallIndex].noteName = getNoteNameFromMIDI(walls[wallIndex].note);
    walls[wallIndex].color = THEME_PRIMARY;
    walls[wallIndex].active = false;
    walls[wallIndex].noteOffTime = 0;
    walls[wallIndex].flashVelocity = 0;
    walls[wallIndex].side = 0;
    wallIndex++;
  }
  
  // Right wall - 4 segments
  for (int i = 0; i < 4; i++) {
    walls[wallIndex].x = 272;
    walls[wallIndex].y = 63 + i * 28;
    walls[wallIndex].w = 3;
    walls[wallIndex].h = 28;
    walls[wallIndex].note = getZenWallNote(i, 1);
    walls[wallIndex].noteName = getNoteNameFromMIDI(walls[wallIndex].note);
    walls[wallIndex].color = THEME_SECONDARY;
    walls[wallIndex].active = false;
    walls[wallIndex].noteOffTime = 0;
    walls[wallIndex].flashVelocity = 0;
    walls[wallIndex].side = 1;
    wallIndex++;
  }
  
  // Bottom wall - 8 segments
  for (int i = 0; i < 8; i++) {
    walls[wallIndex].x = 50 + i * 28;
    walls[wallIndex].y = 177;
    walls[wallIndex].w = 28;
    walls[wallIndex].h = 3;
    walls[wallIndex].note = getZenWallNote(7 - i, 2);
    walls[wallIndex].noteName = getNoteNameFromMIDI(walls[wallIndex].note);
    walls[wallIndex].color = THEME_ACCENT;
    walls[wallIndex].active = false;
    walls[wallIndex].noteOffTime = 0;
    walls[wallIndex].flashVelocity = 0;
    walls[wallIndex].side = 2;
    wallIndex++;
  }
  
  // Left wall - 4 segments
  for (int i = 0; i < 4; i++) {
    walls[wallIndex].x = 50;
    walls[wallIndex].y = 63 + i * 28;
    walls[wallIndex].w = 3;
    walls[wallIndex].h = 28;
    walls[wallIndex].note = getZenWallNote(3 - i, 3);
    walls[wallIndex].noteName = getNoteNameFromMIDI(walls[wallIndex].note);
    walls[wallIndex].color = THEME_WARNING;
    walls[wallIndex].active = false;
    walls[wallIndex].noteOffTime = 0;
    walls[wallIndex].flashVelocity = 0;
    walls[wallIndex].side = 3;
    wallIndex++;
  }
}

void handleBouncingBallMode() {
  // Back button
  if (touch.justPressed && isButtonPressed(10, 10, 50, 25)) {
    stopZenNotes();
    exitToMenu();
    return;
  }
  
  if (touch.justPressed) {
    // Add ball button
    if (isButtonPressed(8, 202, 36, 16)) {
      if (numActiveBalls < MAX_BALLS) {
        numActiveBalls++;
        initializeBalls();
        drawBouncingBallMode();
      }
      return;
    }

    if (isButtonPressed(50, 202, 44, 16)) {
      applyZenCalm();
      drawBouncingBallMode();
      return;
    }

    if (isButtonPressed(100, 202, 48, 16)) {
      applyZenChaos();
      drawBouncingBallMode();
      return;
    }

    if (isButtonPressed(156, 202, 26, 16)) {
      zenDensity = max(10, zenDensity - 10);
      drawBouncingBallMode();
      return;
    }

    if (isButtonPressed(188, 202, 26, 16)) {
      zenDensity = min(100, zenDensity + 10);
      drawBouncingBallMode();
      return;
    }

    if (isButtonPressed(222, 202, 34, 16)) {
      zenNoteLength = max(60, zenNoteLength - 60);
      drawBouncingBallMode();
      return;
    }

    if (isButtonPressed(264, 202, 34, 16)) {
      zenNoteLength = min(900, zenNoteLength + 60);
      drawBouncingBallMode();
      return;
    }
    
    // Reset button
    if (isButtonPressed(8, 222, 44, 16)) {
      stopZenNotes();
      numActiveBalls = 1;
      initializeBalls();
      drawBouncingBallMode();
      return;
    }
    
    // Scale button
    if (isButtonPressed(60, 222, 46, 16)) {
      stopZenNotes();
      nudgeGlobalScale(1);
      initializeWalls();
      drawBouncingBallMode();
      return;
    }
    
    // Key controls
    if (isButtonPressed(114, 222, 34, 16)) {
      stopZenNotes();
      nudgeGlobalRoot(-1);
      initializeWalls();
      drawBouncingBallMode();
      return;
    }
    
    if (isButtonPressed(156, 222, 34, 16)) {
      stopZenNotes();
      nudgeGlobalRoot(1);
      initializeWalls();
      drawBouncingBallMode();
      return;
    }
    
    // Octave button
    if (isButtonPressed(198, 222, 28, 16)) {
      stopZenNotes();
      ballOctave = (ballOctave == 7) ? 2 : ballOctave + 1;
      initializeWalls();
      drawBouncingBallMode();
      return;
    }
  }
  
  // Update physics and display
  updateBouncingBall();
}

void updateBouncingBall() {
  updateZenNotes();

  // Smooth 60 FPS animation
  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate > 16) {
    // Clear entire play area to prevent flickering
    tft.fillRect(53, 63, 219, 114, THEME_PANEL);
    
    updateBalls();
    checkWallCollisions();
    
    // Draw walls
    drawWalls();
    
    // Draw balls
    drawBalls();
    
    lastUpdate = millis();
  }
}

void updateBalls() {
  for (int i = 0; i < numActiveBalls; i++) {
    if (!balls[i].active) continue;
    
    // Update position
    balls[i].x += balls[i].vx;
    balls[i].y += balls[i].vy;
    
    // Bounce off walls with proper collision detection
    if (balls[i].x - balls[i].size <= 53) {
      balls[i].vx = abs(balls[i].vx);
      balls[i].x = 53 + balls[i].size;
    }
    if (balls[i].x + balls[i].size >= 272) {
      balls[i].vx = -abs(balls[i].vx);
      balls[i].x = 272 - balls[i].size;
    }
    if (balls[i].y - balls[i].size <= 63) {
      balls[i].vy = abs(balls[i].vy);
      balls[i].y = 63 + balls[i].size;
    }
    if (balls[i].y + balls[i].size >= 177) {
      balls[i].vy = -abs(balls[i].vy);
      balls[i].y = 177 - balls[i].size;
    }
  }
}

void drawBalls() {
  for (int i = 0; i < numActiveBalls; i++) {
    if (!balls[i].active) continue;
    tft.fillCircle(balls[i].x, balls[i].y, balls[i].size, balls[i].color);
    tft.drawCircle(balls[i].x, balls[i].y, balls[i].size, THEME_TEXT);
  }
}

void drawWalls() {
  for (int i = 0; i < NUM_WALLS; i++) {
    uint16_t color = walls[i].color;
    
    // Bright flash when active
    if (walls[i].active) {
      unsigned long elapsed = millis() - walls[i].activeTime;
      if (elapsed < 200) {
        color = walls[i].flashVelocity > 105 ? THEME_TEXT : THEME_ACCENT;
      } else {
        walls[i].active = false;
      }
    }
    
    // Draw wall
    tft.fillRect(walls[i].x, walls[i].y, walls[i].w, walls[i].h, color);
    
    // Add note name for longer walls
    if (walls[i].w > walls[i].h && walls[i].w > 50) {
      tft.setTextColor(THEME_BG, color);
      tft.drawCentreString(walls[i].noteName, 
                          walls[i].x + walls[i].w/2, 
                          walls[i].y - 2, 1);
    }
  }
}


void checkWallCollisions() {
  for (int b = 0; b < numActiveBalls; b++) {
    if (!balls[b].active) continue;
    
    static float lastX[MAX_BALLS], lastY[MAX_BALLS];
    static bool initialized = false;
    
    if (!initialized) {
      for (int i = 0; i < MAX_BALLS; i++) {
        lastX[i] = balls[i].x;
        lastY[i] = balls[i].y;
      }
      initialized = true;
    }
    
    // Check collision with each wall segment
    for (int w = 0; w < NUM_WALLS; w++) {
      if (walls[w].active) continue; // Skip if wall is already active
      
      bool collision = false;
      
      // Check collision based on ball position and wall bounds
      if (walls[w].side == 0) { // Top walls
        if (balls[b].y - balls[b].size <= walls[w].y + walls[w].h &&
            balls[b].x >= walls[w].x && balls[b].x <= walls[w].x + walls[w].w &&
            lastY[b] > balls[b].y) {
          collision = true;
        }
      }
      else if (walls[w].side == 1) { // Right walls
        if (balls[b].x + balls[b].size >= walls[w].x &&
            balls[b].y >= walls[w].y && balls[b].y <= walls[w].y + walls[w].h &&
            lastX[b] < balls[b].x) {
          collision = true;
        }
      }
      else if (walls[w].side == 2) { // Bottom walls
        if (balls[b].y + balls[b].size >= walls[w].y &&
            balls[b].x >= walls[w].x && balls[b].x <= walls[w].x + walls[w].w &&
            lastY[b] < balls[b].y) {
          collision = true;
        }
      }
      else if (walls[w].side == 3) { // Left walls
        if (balls[b].x - balls[b].size <= walls[w].x + walls[w].w &&
            balls[b].y >= walls[w].y && balls[b].y <= walls[w].y + walls[w].h &&
            lastX[b] > balls[b].x) {
          collision = true;
        }
      }
      
      if (collision) {
        float speed = sqrt((balls[b].vx * balls[b].vx) + (balls[b].vy * balls[b].vy));
        triggerZenWall(w, speed);
        
        walls[w].active = true;
        walls[w].activeTime = millis();
        
        Serial.printf("Wall segment hit: %s\n", walls[w].noteName.c_str());
        break; // Only trigger one wall per ball per frame
      }
    }
    
    // Update last positions
    lastX[b] = balls[b].x;
    lastY[b] = balls[b].y;
  }
}

void triggerZenWall(int wallIndex, float speed) {
  if (random(100) >= zenDensity) return;

  int velocity = getZenVelocity(speed);
  walls[wallIndex].flashVelocity = velocity;

  if (deviceConnected) {
    if (walls[wallIndex].noteOffTime > 0) {
      sendNote(performance.generativeChannel, walls[wallIndex].note, 0, false);
    }
    sendNote(performance.generativeChannel, walls[wallIndex].note, velocity, true);
    walls[wallIndex].noteOffTime = millis() + zenNoteLength + (velocity * 2);
  }
}

void updateZenNotes() {
  if (!deviceConnected) return;

  unsigned long now = millis();
  for (int i = 0; i < NUM_WALLS; i++) {
    if (walls[i].noteOffTime > 0 && now >= walls[i].noteOffTime) {
      sendNote(performance.generativeChannel, walls[i].note, 0, false);
      walls[i].noteOffTime = 0;
    }
  }
}

void stopZenNotes() {
  for (int i = 0; i < NUM_WALLS; i++) {
    if (walls[i].noteOffTime > 0) {
      sendNote(performance.generativeChannel, walls[i].note, 0, false);
      walls[i].noteOffTime = 0;
    }
  }
}

void applyZenCalm() {
  stopZenNotes();
  numActiveBalls = 1;
  zenDensity = 35;
  zenNoteLength = 480;
  initializeBalls();
  for (int i = 0; i < MAX_BALLS; i++) {
    balls[i].vx *= 0.55;
    balls[i].vy *= 0.55;
  }
}

void applyZenChaos() {
  stopZenNotes();
  numActiveBalls = MAX_BALLS;
  zenDensity = 95;
  zenNoteLength = 120;
  initializeBalls();
  for (int i = 0; i < MAX_BALLS; i++) {
    balls[i].vx *= 1.45;
    balls[i].vy *= 1.45;
  }
}

int getZenWallNote(int segment, int side) {
  int stableDegrees[] = {0, 4, 2, 5, 4, 0, 5, 2, 3, 1, 6, 4};
  int degree = stableDegrees[(segment + side * 2) % 12];
  int octave = ballOctave + (side == 1 || side == 3 ? 1 : 0);
  return getNoteInScale(performance.scale, degree, octave);
}

int getZenVelocity(float speed) {
  int velocity = 56 + (int)(speed * 24.0);
  return constrain(velocity, 48, 118);
}

#endif
