#pragma once

#include <Adafruit_MAX1704X.h>

namespace Battery {

class Status {
public:
    Status(){};

    void begin();
    void tick();

private:

};

extern Status status;

} // namespace Battery