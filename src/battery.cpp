#include <Adafruit_MAX1704X.h>

#include "battery.hpp"
#include "defs.hpp"
#include "display.hpp"

namespace Battery{

Status status;

void Status::begin(){
    // check if battery board exists
    
    // set up
}

void Status::tick(){
    const uint16_t interval = 5000;
    static unsigned long previousMillis = 0;
  
    if ((millis() - previousMillis) >= interval) {
    previousMillis = millis();


    } // timer
}
} // namespace Battery