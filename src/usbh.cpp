#include <Arduino.h>
#include <pio_usb.h>
#include <usb_midi_host.h>
#include <EZ_USB_MIDI_HOST.h>

#include "defs.hpp"
#include "display.hpp"
#include "usbh.hpp"
#include "buttons.hpp"



namespace Usbh {

MidiHost midiHost;

USING_NAMESPACE_MIDI
USING_NAMESPACE_EZ_USB_MIDI_HOST
struct mycustomsettings : public MidiHostSettingsDefault
{

};

RPPICOMIDI_EZ_USB_MIDI_HOST_INSTANCE(usbhMIDI, mycustomsettings)

void MidiHost::begin(){
    ready = false;

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



    // Do other processing that might generate pending MIDI SER data
    //sendNextNote();

    if (Buttons::buttonA){
        Buttons::buttonA = false; // debounce


    }
    if (Buttons::buttonB){
        Buttons::buttonB = false; // debounce
        

    }
    // Command screen to flip
    if (Buttons::buttonC){
        Buttons::buttonC = false; // debounce
    }
    
    // TODO: add CC controls for encoders

    // Tell the USB Host to send as much pending MIDI SER data as possible
    usbhMIDI.writeFlushAll();
}

/* This is code that should probably be removed if not used in your project */
#pragma region Project Specific
// This is left over from the test code, it wont be used in the final
void sendNextNote()
{
    static uint8_t firstNote = 0x5b; // Mackie Control rewind
    static uint8_t lastNote = 0x5f; // Mackie Control stop
    static uint8_t offNote = lastNote;
    static uint8_t onNote = firstNote;
    const uint32_t intervalMs = 1000;
    static uint32_t startMs = 0;
    if (millis() - startMs < intervalMs)
        return; // not enough time
    startMs += intervalMs;
    for (uint8_t midiDevAddr = 1; midiDevAddr <= RPPICOMIDI_TUH_MIDI_MAX_DEV; midiDevAddr++) {
        auto intf = usbhMIDI.getInterfaceFromDeviceAndCable(midiDevAddr, usbhMIDI.getNumOutCables(midiDevAddr)-1);
        if (intf == nullptr)
            continue; // not connected
        intf->sendNoteOn(offNote, 0, 1);
        intf->sendNoteOn(onNote, 0x7f, 1);
        
    }
    if (++offNote > lastNote)
        offNote = firstNote;
    if (++onNote > lastNote)
        onNote = firstNote;
}

#pragma endregion

#pragma region MIDI Callbacks
void onMidiError(int8_t errCode)
{
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

void onNoteOff(Channel channel, byte note, byte velocity)
{
    printAddrAndCable();
    SER.printf("C%u: Note off#%u v=%u\r\n", channel, note, velocity);
}

void onNoteOn(Channel channel, byte note, byte velocity)
{
    printAddrAndCable();
    SER.printf("C%u: Note on#%u v=%u\r\n", channel, note, velocity);
}

void onPolyphonicAftertouch(Channel channel, byte note, byte amount)
{
    printAddrAndCable();
    SER.printf("C%u: PAT#%u=%u\r\n", channel, note, amount);
}

void onControlChange(Channel channel, byte controller, byte value)
{
    printAddrAndCable();
    SER.printf("C%u: CC#%u=%u\r\n", channel, controller, value);
}

void onProgramChange(Channel channel, byte program)
{
    printAddrAndCable();
    SER.printf("C%u: Prog=%u\r\n", channel, program);
}

void onAftertouch(Channel channel, byte value)
{
    printAddrAndCable();
    SER.printf("C%u: AT=%u\r\n", channel, value);
}

void onPitchBend(Channel channel, int value)
{
    printAddrAndCable();
    SER.printf("C%u: PB=%d\r\n", channel, value);
}

void onSysEx(byte * array, unsigned size)
{
    printAddrAndCable();
    SER.printf("SysEx:\r\n");
    unsigned multipleOf8 = size/8;
    unsigned remOf8 = size % 8;
    for (unsigned idx=0; idx < multipleOf8; idx++) {
        for (unsigned jdx = 0; jdx < 8; jdx++) {
            SER.printf("%02x ", *array++);
        }
        SER.printf("\r\n");
    }
    for (unsigned idx = 0; idx < remOf8; idx++) {
        SER.printf("%02x ", *array++);
    }
    SER.printf("\r\n");

    using namespace Display;
    // use incoming data to decide what to do
    // if message is sysex oled
    if(1 == 1){
        driver.drawOLED(array, size);
    }
    // if message is sysex oledDelta
    else if (2 == 2){
        driver.drawOLEDDelta(array, size);
    }
    // if message is sysex seg7
    else if (3 == 3){
        driver.draw7seg(array, size);
    }
    // if message is control
}

void onSMPTEqf(byte data)
{
    printAddrAndCable();
    uint8_t type = (data >> 4) & 0xF;
    data &= 0xF;    
    static const char* fps[4] = {"24", "25", "30DF", "30ND"};
    switch (type) {
        case 0: SER.printf("SMPTE FRM LS %u \r\n", data); break;
        case 1: SER.printf("SMPTE FRM MS %u \r\n", data); break;
        case 2: SER.printf("SMPTE SEC LS %u \r\n", data); break;
        case 3: SER.printf("SMPTE SEC MS %u \r\n", data); break;
        case 4: SER.printf("SMPTE MIN LS %u \r\n", data); break;
        case 5: SER.printf("SMPTE MIN MS %u \r\n", data); break;
        case 6: SER.printf("SMPTE HR LS %u \r\n", data); break;
        case 7:
            SER.printf("SMPTE HR MS %u FPS:%s\r\n", data & 0x1, fps[(data >> 1) & 3]);
            break;
        default:
          SER.printf("invalid SMPTE data byte %u\r\n", data);
          break;
    }
}

void onSongPosition(unsigned beats)
{
    printAddrAndCable();
    SER.printf("SongP=%u\r\n", beats);
}

void onSongSelect(byte songnumber)
{
    printAddrAndCable();
    SER.printf("SongS#%u\r\n", songnumber);
}

void onTuneRequest()
{
    printAddrAndCable();
    SER.printf("Tune\r\n");
}

void onMidiClock()
{
    printAddrAndCable();
    SER.printf("Clock\r\n");
}

void onMidiStart()
{
    printAddrAndCable();
    SER.printf("Start\r\n");
}

void onMidiContinue()
{
    printAddrAndCable();
    SER.printf("Cont\r\n");
}

void onMidiStop()
{
    printAddrAndCable();
    SER.printf("Stop\r\n");
}

void onActiveSense()
{
    printAddrAndCable();
    SER.printf("ASen\r\n");
}

void onSystemReset()
{
    printAddrAndCable();
    SER.printf("SysRst\r\n");
}

void onMidiTick()
{
    printAddrAndCable();
    SER.printf("Tick\r\n");
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
        intf->setHandleNoteOff(onNoteOff);                      // 0x80
        intf->setHandleNoteOn(onNoteOn);                        // 0x90
        intf->setHandleAfterTouchPoly(onPolyphonicAftertouch);  // 0xA0
        intf->setHandleControlChange(onControlChange);          // 0xB0
        intf->setHandleProgramChange(onProgramChange);          // 0xC0
        intf->setHandleAfterTouchChannel(onAftertouch);         // 0xD0
        intf->setHandlePitchBend(onPitchBend);                  // 0xE0
        intf->setHandleSystemExclusive(onSysEx);                // 0xF0, 0xF7
        intf->setHandleTimeCodeQuarterFrame(onSMPTEqf);         // 0xF1
        intf->setHandleSongPosition(onSongPosition);            // 0xF2
        intf->setHandleSongSelect(onSongSelect);                // 0xF3
        intf->setHandleTuneRequest(onTuneRequest);              // 0xF6
        intf->setHandleClock(onMidiClock);                      // 0xF8
        // 0xF9 as 10ms Tick is not MIDI 1.0 standard but implemented in the Arduino MIDI Library
        intf->setHandleTick(onMidiTick);                        // 0xF9
        intf->setHandleStart(onMidiStart);                      // 0xFA
        intf->setHandleContinue(onMidiContinue);                // 0xFB
        intf->setHandleStop(onMidiStop);                        // 0xFC
        intf->setHandleActiveSensing(onActiveSense);            // 0xFE
        intf->setHandleSystemReset(onSystemReset);              // 0xFF
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
        intf->disconnectCallbackFromType(TimeCodeQuarterFrame);
        intf->disconnectCallbackFromType(SongPosition);
        intf->disconnectCallbackFromType(SongSelect);
        intf->disconnectCallbackFromType(TuneRequest);
        intf->disconnectCallbackFromType(Clock);
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
    registerMidiInCallbacks(devAddr);
    listConnectedDevices();
}

void MidiHost::onMIDIdisconnect(uint8_t devAddr)
{
    SER.printf("MIDI device at address %u unplugged\r\n", devAddr);
    unregisterMidiInCallbacks(devAddr);
    // Note that listConnectedDevices() will still list the just unplugged
    //  device as connected until this function returns
    listConnectedDevices();
}
#pragma endregion

} // namespace Usbh