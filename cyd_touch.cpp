#include "cyd_touch.h"

namespace {
constexpr uint8_t CMD_Z1 = 0xB1;
constexpr uint8_t CMD_Z2 = 0xC1;
constexpr uint8_t CMD_X = 0x91;
constexpr uint8_t CMD_Y = 0xD1;

void touchDelay() {
  delayMicroseconds(1);
}

uint16_t transfer16(uint8_t command) {
  uint16_t value = 0;

  for (int bit = 7; bit >= 0; --bit) {
    digitalWrite(CYD_TOUCH_CLK, LOW);
    digitalWrite(CYD_TOUCH_MOSI, (command >> bit) & 0x01);
    touchDelay();
    digitalWrite(CYD_TOUCH_CLK, HIGH);
    touchDelay();
  }

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

  digitalWrite(CYD_TOUCH_CS, LOW);
  touchDelay();

  int16_t z1 = transfer16(CMD_Z1) >> 3;
  int16_t z2 = transfer16(CMD_Z2) >> 3;
  int16_t z = z1 + 4095 - z2;

  if (z >= CYD_TOUCH_Z_THRESHOLD) {
    transfer16(CMD_X);
    x[0] = transfer16(CMD_X) >> 3;
    y[0] = transfer16(CMD_Y) >> 3;
    x[1] = transfer16(CMD_X) >> 3;
    y[1] = transfer16(CMD_Y) >> 3;
    x[2] = transfer16(CMD_X) >> 3;
    y[2] = transfer16(CMD_Y) >> 3;
  }

  transfer16(0x00);

  digitalWrite(CYD_TOUCH_CS, HIGH);

  if (z < CYD_TOUCH_Z_THRESHOLD) {
    point.z = 0;
    return false;
  }

  point.x = bestTwoAverage(x[0], x[1], x[2]);
  point.y = bestTwoAverage(y[0], y[1], y[2]);
  point.z = z;
  return true;
}
