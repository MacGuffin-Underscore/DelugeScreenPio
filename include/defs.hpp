#pragma once
#define SER Serial2

// Board-related
#define BUTTON_A_PIN 9
#define BUTTON_B_PIN 6
#define BUTTON_C_PIN 5

#define BATT_CTRL_ADDR 0x36
#define OLED_DISP_ADDR 0x3c
#define SEG7_DISP_ADDR 0x70

// Pin D+ for host, D- = D+ + 1
#ifndef PIN_USB_HOST_DP
#define PIN_USB_HOST_DP  16
#endif

// Pin for enabling Host VBUS.
#ifndef PIN_5V_EN
#define PIN_5V_EN        18
#endif

// Set 5v to HIGH
#ifndef PIN_5V_EN_STATE
#define PIN_5V_EN_STATE  1
#endif

// Display-related
#define OLED_DATA_LEN 6*128
#define SEG7_DATA_LEN 128
#define OFFX 0
#define OFFY 16

// Other
#define BLINK_RATE 500