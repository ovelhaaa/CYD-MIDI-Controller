#include "cyd_touch.h"

namespace {
constexpr uint8_t CMD_Z1 = 0xB0;
constexpr uint8_t CMD_Z2 = 0xC0;
constexpr uint8_t CMD_X = 0x90;
constexpr uint8_t CMD_Y = 0xD0;

void touchDelay() {
  delayMicroseconds(1);
}

void sendCommand(uint8_t command) {
  for (int bit = 7; bit >= 0; --bit) {
    digitalWrite(CYD_TOUCH_CLK, LOW);
    digitalWrite(CYD_TOUCH_MOSI, (command >> bit) & 0x01);
    touchDelay();
    digitalWrite(CYD_TOUCH_CLK, HIGH);
    touchDelay();
  }
}

uint16_t read16() {
  uint16_t value = 0;

  digitalWrite(CYD_TOUCH_MOSI, LOW);
  for (int bit = 15; bit >= 0; --bit) {
    digitalWrite(CYD_TOUCH_CLK, LOW);
    touchDelay();
    digitalWrite(CYD_TOUCH_CLK, HIGH);
    value <<= 1;
    if (digitalRead(CYD_TOUCH_MISO)) {
      value |= 1;
    }
    touchDelay();
  }

  return value;
}

int16_t readAdc(uint8_t command) {
  digitalWrite(CYD_TOUCH_CS, LOW);
  touchDelay();
  sendCommand(command);
  uint16_t value = read16();
  digitalWrite(CYD_TOUCH_CS, HIGH);

  return (value >> 3) & 0x0FFF;
}

int16_t bestTwoAverage(int16_t a, int16_t b, int16_t c) {
  int16_t ab = abs(a - b);
  int16_t ac = abs(a - c);
  int16_t bc = abs(b - c);

  if (ab <= ac && ab <= bc) return (a + b) / 2;
  if (ac <= ab && ac <= bc) return (a + c) / 2;
  return (b + c) / 2;
}
}

void cydTouchBegin() {
  pinMode(CYD_TOUCH_CS, OUTPUT);
  pinMode(CYD_TOUCH_CLK, OUTPUT);
  pinMode(CYD_TOUCH_MOSI, OUTPUT);
  pinMode(CYD_TOUCH_MISO, INPUT);

  digitalWrite(CYD_TOUCH_CS, HIGH);
  digitalWrite(CYD_TOUCH_CLK, LOW);
  digitalWrite(CYD_TOUCH_MOSI, LOW);
}

bool cydTouchRead(CYDTouchPoint &point) {
  int16_t x[3] = {};
  int16_t y[3] = {};

  int16_t z1 = readAdc(CMD_Z1);
  int16_t z2 = readAdc(CMD_Z2);
  int16_t z = z1 + 4095 - z2;

  if (z >= CYD_TOUCH_Z_THRESHOLD) {
    x[0] = readAdc(CMD_X);
    y[0] = readAdc(CMD_Y);
    x[1] = readAdc(CMD_X);
    y[1] = readAdc(CMD_Y);
    x[2] = readAdc(CMD_X);
    y[2] = readAdc(CMD_Y);
  }

  if (z < CYD_TOUCH_Z_THRESHOLD) {
    point.z = 0;
    return false;
  }

  point.x = bestTwoAverage(x[0], x[1], x[2]);
  point.y = bestTwoAverage(y[0], y[1], y[2]);
  point.z = z;
  return true;
}
