#include "buttons.hpp"
#include "display.hpp"
#include "usbh.hpp"

using namespace Display;
using namespace Usbh;

namespace Buttons {

bool buttonA = false;
bool buttonB = false;
bool buttonC = false;

Bounce2::Button buttonA_db;
Bounce2::Button buttonB_db;
Bounce2::Button buttonC_db;

void begin() {
  buttonA_db.attach(BUTTON_A_PIN, INPUT_PULLUP);
  buttonB_db.attach(BUTTON_B_PIN, INPUT_PULLUP);
  buttonC_db.attach(BUTTON_C_PIN, INPUT_PULLUP);
}

void tick() {
  buttonA_db.update();
  buttonB_db.update();
  buttonC_db.update();

  if (buttonA_db.pressed()) buttonA = true;
  if (buttonB_db.pressed()) buttonB = true;
  if (buttonC_db.pressed()) buttonC = true;

    // flip display
    if (buttonA){
        buttonA = false; // debounce
        driver.staticFlip = true;
        midiHost.requestFlip();
        driver.announce("Flipping display");
    }
    // sleep mode? (draw statics only)
    else if (buttonB){
        buttonB = false; // debounce
        
        driver.announce("button B pressed");

        midiHost.requestImage();
    }
    else if (buttonC){
        buttonC = false; // debounce

        driver.announce("button C pressed");
    }
}

} // namespace Buttons