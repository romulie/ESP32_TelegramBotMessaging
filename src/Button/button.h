// simple anti-bounce button connected between controller input and GND
// uses internal or external PULLUP resistor

#ifndef BUTTON_H
#define BUTTON_H

#include <Arduino.h>

class Button{

private:
  byte pin_;
  uint32_t timer_;
  bool flag_;

public:
  Button(byte pin);
  bool click();
};

#endif
