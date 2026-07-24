/*******************************************************************
 MIDI Controller Main Launcher for ESP32 Cheap Yellow Display
 Main file - handles setup, menu, and mode switching
 *******************************************************************/

#include <SPI.h>
#include <TFT_eSPI.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>

// Include mode files
#include "keyboard_mode.h"
#include "sequencer_mode.h"
#include "bouncing_ball_mode.h"
#include "physics_drop_mode.h"
#include "random_generator_mode.h"
#include "xy_pad_mode.h"
#include "arpeggiator_mode.h"
#include "grid_piano_mode.h"
#include "auto_chord_mode.h"
#include "lfo_mode.h"
#include "ui_elements.h"
#include "midi_utils.h"

// Global objects
TFT_eSPI tft = TFT_eSPI();

// BLE MIDI globals
BLECharacteristic *pCharacteristic;
bool deviceConnected = false;
uint8_t midiPacket[] = {0x80, 0x80, 0x00, 0x60, 0x7F};

// Touch state
TouchState touch;

// App state
AppMode currentMode = MENU;

// Forward declarations
void drawMenu();
void drawMenuCard(int index, bool pressed = false);

// Scalable App Icon System
// To add new apps:
// 1. Add new mode to AppMode enum in common_definitions.h
// 2. Create mode header file (e.g., new_mode.h)
// 3. Include header in this file
// 4. Add to initialization, loop, and enterMode switch statements
// 5. Add entry to apps[] array below
// 6. Add graphics case to drawAppGraphics() function
// 7. Increment numApps
struct AppIcon {
  String name;
  String symbol;
  uint16_t color;
  AppMode mode;
};

#define MAX_APPS 12  // Can easily expand to 3x4 grid
AppIcon apps[] = {
  {"KEYS", "", 0xF965, KEYBOARD},
  {"BEATS", "", 0xFD60, SEQUENCER},
  {"ZEN", "", 0xBFE0, BOUNCING_BALL},
  {"DROP", "", 0x07D0, PHYSICS_DROP},
  {"RNG", "", 0x35DF, RANDOM_GENERATOR},
  {"XY PAD", "", 0x8A7F, XY_PAD},
  {"ARP", "", 0xF81F, ARPEGGIATOR},
  {"GRID", "", 0x05FF, GRID_PIANO},
  {"CHORD", "", 0xFBE0, AUTO_CHORD},
  {"LFO", "", 0x97F3, LFO}
};

int numApps = 10;

const int MENU_CELL_W = 58;
const int MENU_CELL_H = 62;
const int MENU_ICON_SIZE = 36;
const int MENU_SPACING = 4;
const int MENU_ROW_SPACING = 10;
const int MENU_COLS = 5;
const int MENU_START_X = (320 - (MENU_COLS * MENU_CELL_W + (MENU_COLS - 1) * MENU_SPACING)) / 2;
const int MENU_START_Y = 62;

class MIDICallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      Serial.println("BLE client connected");
      if (currentMode == MENU) {
        drawMenu(); // Redraw menu to clear "BLE WAITING..."
      }
      updateStatus();
    }
    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      Serial.println("BLE client disconnected");
      if (currentMode == MENU) {
        drawMenu(); // Redraw menu to show "BLE WAITING..."
      }
      updateStatus();
      // Stop all notes
      for (int i = 0; i < 128; i++) {
        sendMIDI(0x80, i, 0);
      }
      // Restart advertising so new connections can be made
      BLEDevice::startAdvertising();
    }
};

void setup() {
  Serial.begin(115200);
  
  // Touch setup
  cydTouchBegin();
  
  // Display setup
  tft.init();
  tft.setRotation(1);
  pinMode(21, OUTPUT);
  digitalWrite(21, HIGH);
  
  // BLE MIDI Setup
  Serial.println("Initializing BLE MIDI...");
  BLEDevice::init("CYD-MIDI");
  BLEDevice::setPower(ESP_PWR_LVL_P9);
  Serial.println("BLE Device initialized");
  
  BLEServer *server = BLEDevice::createServer();
  server->setCallbacks(new MIDICallbacks());
  Serial.println("BLE Server created");
  
  BLEService *service = server->createService(BLEUUID(SERVICE_UUID));
  Serial.println("BLE Service created");
  
  pCharacteristic = service->createCharacteristic(
    BLEUUID(CHARACTERISTIC_UUID),
    BLECharacteristic::PROPERTY_READ | 
    BLECharacteristic::PROPERTY_WRITE | 
    BLECharacteristic::PROPERTY_WRITE_NR |
    BLECharacteristic::PROPERTY_NOTIFY
  );
  
  pCharacteristic->addDescriptor(new BLE2902());
  service->start();
  Serial.println("BLE Service started");
  
  BLEAdvertising *advertising = server->getAdvertising();
  advertising->addServiceUUID(service->getUUID());
  BLEAdvertisementData adData;
  adData.setFlags(ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT);
  adData.setCompleteServices(BLEUUID(SERVICE_UUID));
  advertising->setAdvertisementData(adData);
  BLEAdvertisementData scanData;
  scanData.setName("CYD-MIDI");
  advertising->setScanResponseData(scanData);
  advertising->setScanResponse(true);
  advertising->setMinPreferred(0x06);
  advertising->setMaxPreferred(0x12);
  advertising->start();
  Serial.println("BLE Advertising started - Device discoverable as 'CYD-MIDI'");
  
  // Initialize mode systems
  initializeKeyboardMode();
  initializeSequencerMode();
  initializeBouncingBallMode();
  initializeRandomGeneratorMode();
  initializeXYPadMode();
  initializeArpeggiatorMode();
  initializeGridPianoMode();
  initializeAutoChordMode();
  initializeLFOMode();
  
  drawMenu();
  updateStatus();
  Serial.println("MIDI Controller ready!");
}

void loop() {
  updateTouch();
  
  switch (currentMode) {
    case MENU:
      if (touch.justPressed) handleMenuTouch();
      break;
    case KEYBOARD:
      handleKeyboardMode();
      break;
    case SEQUENCER:
      handleSequencerMode();
      break;
    case BOUNCING_BALL:
      handleBouncingBallMode();
      break;
    case PHYSICS_DROP:
      handlePhysicsDropMode();
      break;
    case RANDOM_GENERATOR:
      handleRandomGeneratorMode();
      break;
    case XY_PAD:
      handleXYPadMode();
      break;
    case ARPEGGIATOR:
      handleArpeggiatorMode();
      break;
    case GRID_PIANO:
      handleGridPianoMode();
      break;
    case AUTO_CHORD:
      handleAutoChordMode();
      break;
    case LFO:
      handleLFOMode();
      break;
  }
  
  delay(20);
}

void drawMenu() {
  tft.fillScreen(THEME_BG);
  
  // Header
  tft.fillRect(0, 0, 320, 48, THEME_SURFACE);
  tft.fillRect(0, 47, 320, 1, THEME_BORDER);
  tft.fillRect(0, 48, 320, 2, THEME_PRIMARY);
  tft.setTextColor(THEME_TEXT, THEME_SURFACE);
  tft.drawString("CYD MIDI", 12, 8, 4);
  tft.setTextColor(THEME_TEXT_DIM, THEME_SURFACE);
  tft.drawString("controller", 14, 30, 2);
  
  // Version number
  uint16_t statusColor = deviceConnected ? THEME_SUCCESS : THEME_ERROR;
  uint16_t statusBg = deviceConnected ? 0x0340 : 0x2800;
  tft.fillRoundRect(222, 11, 86, 24, 5, statusBg);
  tft.drawRoundRect(222, 11, 86, 24, 5, statusColor);
  tft.fillCircle(236, 23, 4, statusColor);
  tft.setTextColor(THEME_TEXT, statusBg);
  tft.drawString(deviceConnected ? "BLE ON" : "WAITING", 246, 17, 2);
  
  for (int i = 0; i < numApps; i++) {
    drawMenuCard(i);
  }

  tft.setTextColor(THEME_TEXT_DIM, THEME_BG);
  tft.drawCentreString(deviceConnected ? "MIDI ready" : "Open MIDIberry or BLE-MIDI Connect", 160, 220, 2);
}

void drawMenuCard(int index, bool pressed) {
  int col = index % MENU_COLS;
  int row = index / MENU_COLS;
  int x = MENU_START_X + col * (MENU_CELL_W + MENU_SPACING);
  int y = MENU_START_Y + row * (MENU_CELL_H + MENU_ROW_SPACING);
  int yOffset = pressed ? 2 : 0;
  uint16_t panelColor = pressed ? THEME_SURFACE : THEME_PANEL;
  uint16_t borderColor = pressed ? apps[index].color : THEME_BORDER;
  uint16_t iconColor = pressed ? THEME_TEXT : apps[index].color;

  if (!pressed) {
    tft.fillRoundRect(x + 1, y + 2, MENU_CELL_W, MENU_CELL_H, 6, THEME_BG);
  }
  tft.fillRoundRect(x, y + yOffset, MENU_CELL_W, MENU_CELL_H, 6, panelColor);
  tft.drawRoundRect(x, y + yOffset, MENU_CELL_W, MENU_CELL_H, 6, borderColor);
  tft.fillRoundRect(x + 11, y + 7 + yOffset, MENU_ICON_SIZE, MENU_ICON_SIZE, 6, iconColor);

  drawAppGraphics(apps[index].mode, x + 11, y + 7 + yOffset, MENU_ICON_SIZE);

  tft.setTextColor(pressed ? THEME_TEXT : THEME_TEXT_DIM, panelColor);
  tft.drawCentreString(apps[index].name, x + MENU_CELL_W / 2, y + 47 + yOffset, 1);
}

void drawAppGraphics(AppMode mode, int x, int y, int iconSize) {
  switch (mode) {
    case KEYBOARD: // KEYS - piano keys
      {
        int keyWidth = 4;
        int totalWidth = 5 * keyWidth + 4 * 1; // 5 keys + 4 gaps
        int startX = x + (iconSize - totalWidth) / 2;
        for (int i = 0; i < 5; i++) {
          tft.fillRect(startX + i*5, y + iconSize/2 - 6, keyWidth, 12, THEME_BG);
        }
      }
      break;
    case SEQUENCER: // BEATS - grid pattern
      {
        int gridW = 4, gridH = 4, gapX = 2, gapY = 2;
        int totalW = 4 * gridW + 3 * gapX;
        int totalH = 3 * gridH + 2 * gapY;
        int startX = x + (iconSize - totalW) / 2;
        int startY = y + (iconSize - totalH) / 2;
        for (int r = 0; r < 3; r++) {
          for (int c = 0; c < 4; c++) {
            tft.fillRect(startX + c*(gridW+gapX), startY + r*(gridH+gapY), gridW, gridH, THEME_BG);
          }
        }
      }
      break;
    case BOUNCING_BALL: // ZEN - circle with dots
      {
        int centerX = x + iconSize/2;
        int centerY = y + iconSize/2;
        tft.drawCircle(centerX, centerY, 12, THEME_BG);
        tft.fillCircle(centerX - 6, centerY - 4, 2, THEME_BG);
        tft.fillCircle(centerX + 5, centerY + 2, 2, THEME_BG);
        tft.fillCircle(centerX - 2, centerY + 6, 2, THEME_BG);
      }
      break;
    case PHYSICS_DROP: // DROP - balls falling on platforms
      {
        int centerX = x + iconSize/2;
        int centerY = y + iconSize/2;
        // Draw platforms
        tft.fillRect(centerX - 10, centerY + 8, 8, 2, THEME_BG);
        tft.fillRect(centerX + 4, centerY + 4, 6, 2, THEME_BG);
        // Draw falling balls
        tft.fillCircle(centerX - 6, centerY - 8, 2, THEME_BG);
        tft.fillCircle(centerX + 2, centerY - 4, 2, THEME_BG);
        tft.fillCircle(centerX + 8, centerY, 2, THEME_BG);
      }
      break;
    case RANDOM_GENERATOR: // RNG - random dots
      {
        int centerX = x + iconSize/2;
        int centerY = y + iconSize/2;
        tft.fillCircle(centerX - 8, centerY - 6, 2, THEME_BG);
        tft.fillCircle(centerX - 1, centerY - 3, 2, THEME_BG);
        tft.fillCircle(centerX + 7, centerY + 1, 2, THEME_BG);
        tft.fillCircle(centerX - 4, centerY + 6, 2, THEME_BG);
      }
      break;
    case XY_PAD: // XY PAD - crosshairs
      {
        int centerX = x + iconSize/2;
        int centerY = y + iconSize/2;
        int crossSize = 14;
        tft.drawFastHLine(centerX - crossSize/2, centerY, crossSize, THEME_BG);
        tft.drawFastVLine(centerX, centerY - crossSize/2, crossSize, THEME_BG);
        tft.fillCircle(centerX, centerY, 3, THEME_BG);
      }
      break;
    case ARPEGGIATOR: // ARP - ascending notes
      {
        int centerX = x + iconSize/2;
        int centerY = y + iconSize/2;
        for (int i = 0; i < 4; i++) {
          tft.fillCircle(centerX - 7 + i*5, centerY + 5 - i*3, 2, THEME_BG);
        }
      }
      break;
    case GRID_PIANO: // GRID - grid pattern
      {
        int cellW = 5, cellH = 4, gapX = 1, gapY = 2;
        int totalW = 4 * cellW + 3 * gapX;
        int totalH = 3 * cellH + 2 * gapY;
        int startX = x + (iconSize - totalW) / 2;
        int startY = y + (iconSize - totalH) / 2;
        for (int r = 0; r < 3; r++) {
          for (int c = 0; c < 4; c++) {
            tft.drawRect(startX + c*(cellW+gapX), startY + r*(cellH+gapY), cellW, cellH, THEME_BG);
          }
        }
      }
      break;
    case AUTO_CHORD: // CHORD - stacked notes
      {
        int centerX = x + iconSize/2;
        int centerY = y + iconSize/2;
        int lineWidth = 14;
        tft.fillRect(centerX - lineWidth/2, centerY + 4, lineWidth, 2, THEME_BG);
        tft.fillRect(centerX - lineWidth/2, centerY, lineWidth, 2, THEME_BG);
        tft.fillRect(centerX - lineWidth/2, centerY - 4, lineWidth, 2, THEME_BG);
      }
      break;
    case LFO: // LFO - simple sine wave line
      {
        int centerX = x + iconSize/2;
        int centerY = y + iconSize/2;
        
        // Draw sine wave as connected line segments
        int lastX = centerX - 15;
        int lastY = centerY;
        
        for (int i = 1; i <= 15; i++) {
          int px = centerX - 15 + i * 2;
          float angle = (i * 3.14159) / 4.0; // One and a half cycles
          int py = centerY + (int)(6 * sin(angle));
          
          // Draw line from last point to current point
          tft.drawLine(lastX, lastY, px, py, THEME_BG);
          
          lastX = px;
          lastY = py;
        }
      }
      break;
  }
}

void handleMenuTouch() {
  for (int i = 0; i < numApps; i++) {
    int col = i % MENU_COLS;
    int row = i / MENU_COLS;
    int x = MENU_START_X + col * (MENU_CELL_W + MENU_SPACING);
    int y = MENU_START_Y + row * (MENU_CELL_H + MENU_ROW_SPACING);
    
    if (isButtonPressed(x, y, MENU_CELL_W, MENU_CELL_H)) {
      drawMenuCard(i, true);
      delay(70);
      enterMode(apps[i].mode);
      return;
    }
  }
}

void enterMode(AppMode mode) {
  currentMode = mode;
  switch (mode) {
    case KEYBOARD:
      drawKeyboardMode();
      break;
    case SEQUENCER:
      drawSequencerMode();
      break;
    case BOUNCING_BALL:
      drawBouncingBallMode();
      break;
    case PHYSICS_DROP:
      drawPhysicsDropMode();
      break;
    case RANDOM_GENERATOR:
      drawRandomGeneratorMode();
      break;
    case XY_PAD:
      drawXYPadMode();
      break;
    case ARPEGGIATOR:
      drawArpeggiatorMode();
      break;
    case GRID_PIANO:
      drawGridPianoMode();
      break;
    case AUTO_CHORD:
      drawAutoChordMode();
      break;
    case LFO:
      drawLFOMode();
      break;
  }
  updateStatus();
}

void exitToMenu() {
  currentMode = MENU;
  stopAllModes();
  delay(50);
  drawMenu();
  updateStatus();
}
