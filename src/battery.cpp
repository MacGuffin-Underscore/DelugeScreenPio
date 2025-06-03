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
    const uint32_t interval_ms = 5000;
    static uint32_t start_ms = 0;

    if (millis() - start_ms < interval_ms) {
        return;
    }
    start_ms += interval_ms;
}
} // namespace Battery