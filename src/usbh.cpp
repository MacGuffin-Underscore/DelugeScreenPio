#include <Arduino.h>
#include <pio_usb.h>
#include <usb_midi_host.h>
#include <EZ_USB_MIDI_HOST.h>
#include <string>

#include "defs.hpp"
#include "display.hpp"
#include "usbh.hpp"
#include "buttons.hpp"


using namespace Display;

const uint8_t sysex_get_oled[6] = {0xf0, 0x7d, 0x02, 0x00, 0x01, 0xf7};
const uint8_t sysex_get_7seg[6] = {0xf0, 0x7d, 0x02, 0x01, 0x00, 0xf7};
const uint8_t sysex_get_display[6] = {0xf0, 0x7d, 0x02, 0x00, 0x02, 0xf7};
const uint8_t sysex_get_display_force[6] = {0xf0, 0x7d, 0x02, 0x00, 0x03, 0xf7};
const uint8_t sysex_get_debug[6] = {0xf0, 0x7d, 0x03, 0x00, 0x01, 0xf7};
const uint8_t sysex_flip_screen[6] = {0xf0, 0x7d, 0x02, 0x00, 0x04, 0xf7};

namespace Usbh {

MidiHost midiHost;

USING_NAMESPACE_MIDI
USING_NAMESPACE_EZ_USB_MIDI_HOST
struct mycustomsettings : public MidiHostSettingsDefault
{
    static const unsigned SysExMaxSize = 600; // for MIDI Library changed to 450
    static const unsigned MidiRxBufsize = RPPICOMIDI_EZ_USB_MIDI_HOST_GET_BUFSIZE(SysExMaxSize);
    static const unsigned MidiTxBufsize = RPPICOMIDI_EZ_USB_MIDI_HOST_GET_BUFSIZE(SysExMaxSize);
};

RPPICOMIDI_EZ_USB_MIDI_HOST_INSTANCE(usbhMIDI, mycustomsettings)

void MidiHost::begin(){
    ready = false;
    openMsg = false;

    pio_cfg = PIO_USB_DEFAULT_CONFIG;
    pio_cfg.pin_dp = PIN_USB_HOST_DP;

    USBHost.configure_pio_usb(1, &pio_cfg);

    usbhMIDI.begin(&USBHost, 1, onMIDIconnect, onMIDIdisconnect); // this is breaking shit

    ready = true;
}

void MidiHost::tick(){
    // Handle USB Stack processing
    USBHost.task();

    // Handle any incoming data; triggers MIDI IN callbacks
    usbhMIDI.readAll();

    // Update image from deluge
    requestImage();

    // TODO: add CC controls for encoders

    // Tell the USB Host to send as much pending MIDI SER data as possible
    usbhMIDI.writeFlushAll();
}

/* This is code that should probably be removed if not used in your project */
#pragma region Project Specific

void MidiHost::requestImage() {
    const uint16_t interval = 1000;
    static unsigned long previousMillis = 0;
  
    if ((millis() - previousMillis) >= interval) {
    previousMillis = millis();

    for (uint8_t midiDevAddr = 1; midiDevAddr <= RPPICOMIDI_TUH_MIDI_MAX_DEV; midiDevAddr++) {
        auto intf = usbhMIDI.getInterfaceFromDeviceAndCable(midiDevAddr, usbhMIDI.getNumOutCables(midiDevAddr)-1);
        if (intf == nullptr)
            continue; // not connected
        intf->sendSysEx(6, sysex_get_display, true);
    }
    } // timer
}

void MidiHost::requestFlip() {
    const uint16_t interval = 1000;
    static unsigned long previousMillis = 0;
  
    if ((millis() - previousMillis) >= interval) {
    previousMillis = millis();

    for (uint8_t midiDevAddr = 1; midiDevAddr <= RPPICOMIDI_TUH_MIDI_MAX_DEV; midiDevAddr++) {
        auto intf = usbhMIDI.getInterfaceFromDeviceAndCable(midiDevAddr, usbhMIDI.getNumOutCables(midiDevAddr)-1);
        if (intf == nullptr)
            continue; // not connected
        intf->sendSysEx(6, sysex_flip_screen, true);
    }
    } // timer
}

#pragma endregion

#pragma region MIDI Callbacks
void onMidiError(int8_t errCode)
{
    driver.announce("error, try restarting dlge");
    SER.printf("MIDI Errors: %s %s %s\r\n", (errCode & (1UL << ErrorParse)) ? "Parse":"",
        (errCode & (1UL << ErrorActiveSensingTimeout)) ? "Active Sensing Timeout" : "",
        (errCode & (1UL << WarningSplitSysEx)) ? "Split SysEx":"");
}

void printAddrAndCable()
{
    uint8_t midiDevAddr, cable;
    usbhMIDI.getCurrentReadDevAndCable(midiDevAddr, cable);
    SER.printf("[%02d,%02d] ",midiDevAddr, cable);
}

void onSysEx(byte * array, unsigned size)
{
    midiHost.openMsg = true;
    // Print message, bogs down SER when not in use
    // printAddrAndCable();
    // SER.printf("SysEx:\r\n");
    // unsigned multipleOf8 = size/8;
    // unsigned remOf8 = size % 8;
    // unsigned remIdx = 0;
    // for (unsigned idx=0; idx < multipleOf8; idx++) {
    //     for (unsigned jdx = 0; jdx < 8; jdx++) {
    //         SER.printf("%02x ", array[(idx*8)+jdx]);
    //         remIdx++;
    //     }
    //     SER.printf("\r\n");
    // }
    // for (unsigned idx = 0; idx < remOf8; idx++) {
    //     SER.printf("%02x ", array[remIdx+idx]);
    // }
    // SER.printf("\r\n");

    // send over to display if it is a screen
    if (size >= 5 && array[2] == uint8_t{0x02}){
        driver.handleScreenSysexMessage(array, size);
    }
    // if message is control
    // TODO: add this, later...
    midiHost.openMsg = false;
}
void onMidiInWriteFail(uint8_t devAddr, uint8_t cable, bool fifoOverflow)
{
    if (fifoOverflow)
        SER.printf("[%02d,%02d] MIDI IN FIFO overflow\r\n", devAddr, cable);
    else
        SER.printf("[%02d,%02d] MIDI IN FIFO error\r\n", devAddr, cable);
}

void registerMidiInCallbacks(uint8_t midiDevAddr)
{
    uint8_t ncables = usbhMIDI.getNumInCables(midiDevAddr);
    for (uint8_t cable = 0; cable < ncables; cable++) {
        auto intf = usbhMIDI.getInterfaceFromDeviceAndCable(midiDevAddr, cable);
        intf->setHandleSystemExclusive(onSysEx);                // 0xF0, 0xF7
        intf->setHandleError(onMidiError);
    }
    auto dev = usbhMIDI.getDevFromDevAddr(midiDevAddr);
    if (dev == nullptr)
        return;
    dev->setOnMidiInWriteFail(onMidiInWriteFail);
}

void unregisterMidiInCallbacks(uint8_t midiDevAddr)
{
    uint8_t ncables = usbhMIDI.getNumInCables(midiDevAddr);
    for (uint8_t cable = 0; cable < ncables; cable++) {
        auto intf = usbhMIDI.getInterfaceFromDeviceAndCable(midiDevAddr, cable);
        if (intf == nullptr)
            return;
        intf->disconnectCallbackFromType(NoteOn);
        intf->disconnectCallbackFromType(NoteOff);
        intf->disconnectCallbackFromType(AfterTouchPoly);
        intf->disconnectCallbackFromType(ControlChange);
        intf->disconnectCallbackFromType(ProgramChange);
        intf->disconnectCallbackFromType(AfterTouchChannel);
        intf->disconnectCallbackFromType(PitchBend);
        intf->disconnectCallbackFromType(SystemExclusive);
        // 0xF9 as 10ms Tick is not MIDI 1.0 standard but implemented in the Arduino MIDI Library
        intf->disconnectCallbackFromType(Tick);
        intf->disconnectCallbackFromType(Start);
        intf->disconnectCallbackFromType(Continue);
        intf->disconnectCallbackFromType(Stop);
        intf->disconnectCallbackFromType(ActiveSensing);
        intf->disconnectCallbackFromType(SystemReset);
        intf->setHandleError(nullptr);
    }
    auto dev = usbhMIDI.getDevFromDevAddr(midiDevAddr);
    if (dev == nullptr)
        return;
    dev->setOnMidiInWriteFail(nullptr);
}

/* CONNECTION MANAGEMENT */
void listConnectedDevices()
{
    SER.printf("Dev  VID:PID  Product Name[Manufacter]{serial string}\r\n");
    for (uint8_t midiDevAddr = 1; midiDevAddr <= RPPICOMIDI_TUH_MIDI_MAX_DEV; midiDevAddr++) {
        auto dev = usbhMIDI.getDevFromDevAddr(midiDevAddr);
        if (dev) {
            SER.printf("%02u  %04x:%04x %s[%s]{%s}\r\n",midiDevAddr, dev->getVID(), dev->getPID(),
                dev->getProductStr(), dev->getManufacturerStr(), dev->getSerialString());
        }
    }
}
void MidiHost::onMIDIconnect(uint8_t devAddr, uint8_t nInCables, uint8_t nOutCables)
{
    SER.printf("MIDI device at address %u has %u IN cables and %u SER cables\r\n", devAddr, nInCables, nOutCables);
    driver.announce("connected");

    registerMidiInCallbacks(devAddr);
    listConnectedDevices();
}

void MidiHost::onMIDIdisconnect(uint8_t devAddr)
{
    SER.printf("MIDI device at address %u unplugged\r\n", devAddr);
    driver.announce("disconnected");

    unregisterMidiInCallbacks(devAddr);
    // Note that listConnectedDevices() will still list the just unplugged
    //  device as connected until this function returns
    listConnectedDevices();
    driver.drawOledStatic();
    driver.drawSeg7Static();
}
#pragma endregion

} // namespace Usbh