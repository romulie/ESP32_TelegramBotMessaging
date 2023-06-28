
#include "clock.h"


  Clock::Clock() = default;

  Clock::Clock(byte secs_, byte mins_, byte hours_) : secs(secs_), mins(mins_), hours(hours_) {};

  void Clock::tick(){
    ++secs;
    if (secs == 60)   { ++mins; secs = 0; }
    if (mins == 60)   { ++hours; mins = 0; }
    if (hours == 24)  { hours = 0; }
  }

  void Clock::reset()            { secs = 0; mins = 0; hours = 0; }

  byte Clock::getSeconds() const { return secs; }

  byte Clock::getMinutes() const { return mins; }

  byte Clock::getHours() const   { return hours; }

