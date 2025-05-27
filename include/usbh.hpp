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
    // RPPICOMIDI_EZ_USB_MIDI_HOST_INSTANCE(usbhMIDI, mycustomsettings)

    static void sendNextNote();
    static void printAddrAndCable();
    static void listConnectedDevices();

private:
    bool ready;
    pio_usb_configuration_t pio_cfg ;
    /* MIDI handling*/
    static void onMidiError(int8_t errCode);
    static void onNoteOff(Channel channel, byte note, byte velocity);
    static void onNoteOn(Channel channel, byte note, byte velocity);
    static void onPolyphonicAftertouch(Channel channel, byte note, byte amount);
    static void onControlChange(Channel channel, byte controller, byte value);
    static void onProgramChange(Channel channel, byte program);
    static void onAftertouch(Channel channel, byte value);
    static void onPitchBend(Channel channel, int value);
    static void onSysEx(byte * array, unsigned size);
    static void onSMPTEqf(byte data);
    static void onSongPosition(unsigned beats);
    static void onSongSelect(byte songnumber);
    static void onTuneRequest();
    static void onMidiClock();
    static void onMidiStart();
    static void onMidiContinue();
    static void onMidiStop();
    static void onActiveSense();
    static void onSystemReset();
    static void onMidiTick();
    static void onMidiInWriteFail(uint8_t devAddr, uint8_t cable, bool fifoOverflow);
    static void registerMidiInCallbacks(uint8_t midiDevAddr);
    static void unregisterMidiInCallbacks(uint8_t midiDevAddr);
    static void onMIDIconnect(uint8_t devAddr, uint8_t nInCables, uint8_t nOutCables);
    static void onMIDIdisconnect(uint8_t devAddr);
    
};

extern MidiHost midiHost;

} // namespace Usbh