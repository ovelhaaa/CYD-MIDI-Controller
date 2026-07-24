#ifndef CYD_TOUCH_H
#define CYD_TOUCH_H

#include "common_definitions.h"

struct CYDTouchPoint {
  int16_t x = 0;
  int16_t y = 0;
  int16_t z = 0;
};

void cydTouchBegin();
bool cydTouchRead(CYDTouchPoint &point);

#endif
