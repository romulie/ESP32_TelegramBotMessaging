// Simple non-blocking timer


#ifndef TIMER_H
#define TIMER_H

#include <Arduino.h>

class Timer{

private:
  uint32_t period_ = 0, time_ = 0;

public:
  Timer(uint32_t period);
  void setPeriod(uint32_t period);
  bool ready();
};

#endif
