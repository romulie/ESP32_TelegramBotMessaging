#include "timer.h"


  Timer::Timer(uint32_t period = 0) : period_(period) {}; 
 
  void Timer::setPeriod(uint32_t period = 0) { period_ = period; };
 
  bool Timer::ready(){
    if (period_ !=0 && millis() - time_ >= period_){
        time_ = millis();
        return true;
      }
      return false;
  }
