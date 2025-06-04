#include <Arduino.h>
#include <Adafruit_TinyUSB.h>
#include <pio_usb.h>
#include <malloc.h>

#include "battery.hpp"
#include "buttons.hpp"
#include "defs.hpp"
#include "display.hpp"
#include "usbh.hpp"

void serial_flush_buffer()
{
  while (Serial.read() >= 0)
   ; // do nothing
}

static void blinkLED(void)
{
    const uint16_t interval = 500;
    static unsigned long previousMillis = 0;
    static bool ledState = false;
  
    if ((millis() - previousMillis) >= interval) {
    previousMillis = millis();

    ledState = !ledState;
    digitalWrite(LED_BUILTIN, ledState ? HIGH:LOW); 
    serial_flush_buffer();
    }
}

/* APPLICATION STARTS HERE */
void setup()
{
    SER.begin(9200); //115200
    set_sys_clock_hz(240000000UL, true); // 120Mhz for bit-banging USB

    while(!SER);   // wait for serial port
    pinMode(LED_BUILTIN, OUTPUT);

    pinMode(18, OUTPUT); // turn on 5v output
    digitalWrite(18, HIGH);

    // Check for CPU frequency, must be multiple of 120Mhz for bit-banging USB
    uint32_t cpu_hz = clock_get_hz(clk_sys);
    if ( cpu_hz != 120000000UL && cpu_hz != 240000000UL ) {
        delay(2000);   // wait for native usb
        SER.printf("Error: CPU Clock = %u, PIO USB require CPU clock must be multiple of 120 Mhz\r\n", cpu_hz);
        SER.printf("Change your CPU Clock to either 120 or 240 Mhz in Menu->CPU Speed \r\n", cpu_hz);
        digitalWrite(LED_BUILTIN, HIGH); // lock high so we know it's fucked
        while(1) delay(1);
    }
    SER.print("Initializing...");
    
    // init all classes
    Battery::status.begin();
    Buttons::begin();
    Usbh::midiHost.begin();
    Display::driver.begin();
    delay(2000);
}

void loop() {
    Battery::status.tick();
    Buttons::tick();
    Display::driver.tick();
    Usbh::midiHost.tick();

    blinkLED(); 
}