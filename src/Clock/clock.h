// Simple clock class which counts hours, minutes and seconds
// This clock should be driven by the .tick() method every second to increment 

#ifndef CLOCK_H
#define CLOCK_H

#include <Arduino.h>

class Clock{
private:
  byte secs = 0, mins = 0, hours = 0; 

public: 
  Clock();
  Clock(byte secs_, byte mins_, byte hours_);
  void tick();
  void reset();            
  byte getSeconds() const; 
  byte getMinutes() const; 
  byte getHours() const;
};

#endif
