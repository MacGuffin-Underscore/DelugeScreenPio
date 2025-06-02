#pragma once

#include <Arduino.h>
#include <pio_usb.h>
#include <usb_midi_host.h>
#include <EZ_USB_MIDI_HOST.h>

#include "defs.hpp"

USING_NAMESPACE_MIDI
namespace Usbh {

class MidiHost{
public:
    MidiHost(){};

    void begin();
    void tick();

    Adafruit_USBH_Host USBHost;

private:
    bool ready;

    pio_usb_configuration_t pio_cfg ;
    void requestFlip();
    void requestImage();
    static void onMIDIconnect(uint8_t devAddr, uint8_t nInCables, uint8_t nOutCables);
    static void onMIDIdisconnect(uint8_t devAddr);
};

extern MidiHost midiHost;

} // namespace Usbh