#include <Arduino.h>
#include <pio_usb.h>
#include <EZ_USB_MIDI_HOST.h>

#include "defs.hpp"
#include "usbh.hpp"

// /* MIDI IN MESSAGE REPORTING */
namespace Usbh {

MidiHost midiHost;

void MidiHost::begin(){
    
    usbhMIDI.begin(&USBHost, 1, onMIDIconnect, onMIDIdisconnect);
    pio_cfg.pin_dp = PIN_USB_HOST_DP;

    USBHost.configure_pio_usb(1, &pio_cfg);
}

void MidiHost::tick(){

}

void MidiHost::sendNextNote()
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

#pragma region MIDI
void MidiHost::onMidiError(int8_t errCode)
{
    SER.printf("MIDI Errors: %s %s %s\r\n", (errCode & (1UL << ErrorParse)) ? "Parse":"",
        (errCode & (1UL << ErrorActiveSensingTimeout)) ? "Active Sensing Timeout" : "",
        (errCode & (1UL << WarningSplitSysEx)) ? "Split SysEx":"");
}

void MidiHost::printAddrAndCable()
{
    uint8_t midiDevAddr, cable;
    usbhMIDI.getCurrentReadDevAndCable(midiDevAddr, cable);
    SER.printf("[%02d,%02d] ",midiDevAddr, cable);
}

void MidiHost::onNoteOff(Channel channel, byte note, byte velocity)
{
    printAddrAndCable();
    SER.printf("C%u: Note off#%u v=%u\r\n", channel, note, velocity);
}

void MidiHost::onNoteOn(Channel channel, byte note, byte velocity)
{
    printAddrAndCable();
    SER.printf("C%u: Note on#%u v=%u\r\n", channel, note, velocity);
}

void MidiHost::onPolyphonicAftertouch(Channel channel, byte note, byte amount)
{
    printAddrAndCable();
    SER.printf("C%u: PAT#%u=%u\r\n", channel, note, amount);
}

void MidiHost::onControlChange(Channel channel, byte controller, byte value)
{
    printAddrAndCable();
    SER.printf("C%u: CC#%u=%u\r\n", channel, controller, value);
}

void MidiHost::onProgramChange(Channel channel, byte program)
{
    printAddrAndCable();
    SER.printf("C%u: Prog=%u\r\n", channel, program);
}

void MidiHost::onAftertouch(Channel channel, byte value)
{
    printAddrAndCable();
    SER.printf("C%u: AT=%u\r\n", channel, value);
}

void MidiHost::onPitchBend(Channel channel, int value)
{
    printAddrAndCable();
    SER.printf("C%u: PB=%d\r\n", channel, value);
}

void MidiHost::onSysEx(byte * array, unsigned size)
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
}

void MidiHost::onSMPTEqf(byte data)
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

void MidiHost::onSongPosition(unsigned beats)
{
    printAddrAndCable();
    SER.printf("SongP=%u\r\n", beats);
}

void MidiHost::onSongSelect(byte songnumber)
{
    printAddrAndCable();
    SER.printf("SongS#%u\r\n", songnumber);
}

void MidiHost::onTuneRequest()
{
    printAddrAndCable();
    SER.printf("Tune\r\n");
}

void MidiHost::onMidiClock()
{
    printAddrAndCable();
    SER.printf("Clock\r\n");
}

void MidiHost::onMidiStart()
{
    printAddrAndCable();
    SER.printf("Start\r\n");
}

void MidiHost::onMidiContinue()
{
    printAddrAndCable();
    SER.printf("Cont\r\n");
}

void MidiHost::onMidiStop()
{
    printAddrAndCable();
    SER.printf("Stop\r\n");
}

void MidiHost::onActiveSense()
{
    printAddrAndCable();
    SER.printf("ASen\r\n");
}

void MidiHost::onSystemReset()
{
    printAddrAndCable();
    SER.printf("SysRst\r\n");
}

void MidiHost::onMidiTick()
{
    printAddrAndCable();
    SER.printf("Tick\r\n");
}

void MidiHost::onMidiInWriteFail(uint8_t devAddr, uint8_t cable, bool fifoOverflow)
{
    if (fifoOverflow)
        SER.printf("[%02d,%02d] MIDI IN FIFO overflow\r\n", devAddr, cable);
    else
        SER.printf("[%02d,%02d] MIDI IN FIFO error\r\n", devAddr, cable);
}

void MidiHost::registerMidiInCallbacks(uint8_t midiDevAddr)
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

void MidiHost::unregisterMidiInCallbacks(uint8_t midiDevAddr)
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
void MidiHost::listConnectedDevices()
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

}