#include <Arduino.h>
#include <Adafruit_TinyUSB.h>
#include <pio_usb.h>

#include "display.hpp"
#include "usbh.hpp"
#include "buttons.hpp"
#include "defs.hpp"

static void blinkLED(void)
{
    const uint32_t intervalMs = 1000;
    static uint32_t startMs = 0;

    static bool ledState = false;
    if ( millis() - startMs < intervalMs)
        return;
    startMs += intervalMs;

    ledState = !ledState;
    digitalWrite(LED_BUILTIN, ledState ? HIGH:LOW); 
    Serial.printf("fuck");
}


/* APPLICATION STARTS HERE */
void setup()
{
    SER.begin(115200);
    
    set_sys_clock_hz(120000000UL, true);

    while(!SER);   // wait for serial port
    pinMode(LED_BUILTIN, OUTPUT);
    
    // Check for CPU frequency, must be multiple of 120Mhz for bit-banging USB
    uint32_t cpu_hz = clock_get_hz(clk_sys);
    if ( cpu_hz != 120000000UL && cpu_hz != 240000000UL ) {
        delay(2000);   // wait for native usb
        SER.printf("Error: CPU Clock = %u, PIO USB require CPU clock must be multiple of 120 Mhz\r\n", cpu_hz);
        SER.printf("Change your CPU Clock to either 120 or 240 Mhz in Menu->CPU Speed \r\n", cpu_hz);
        digitalWrite(LED_BUILTIN, HIGH); // lock high so we know it's fucked
        while(1) delay(1);
    }

    // init all classes
    Usbh::midiHost.begin();
    Display::driver.begin();
    Buttons::begin();
}

void loop() {
    Display::driver.tick();
    Usbh::midiHost.tick();
    Buttons::tick();
    
    // Do other non-USB host processing
    blinkLED();
}