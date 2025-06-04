#include <bits/stdc++.h>

#include "defs.hpp"
#include "display.hpp"
#include "images.hpp"
#include "rle.hpp"

using namespace std;
// dict of dlg -> standard
std::map<int, int> seg7Dict = {
  { 0 , 6 },
  { 1 , 5 },
  { 2 , 4 },
  { 3 , 3 },
  { 4 , 2 },
  { 5 , 1 },
  { 6 , 0 }
};

namespace Display {

Driver driver;


void Driver::begin() {
  memchr(oledData, 0, OLED_DATA_LEN);

  delay(250);
  oled_disp.begin(OLED_DISP_ADDR, true);
  oled_disp.setRotation(1);
  oled_disp.setCursor(0, 0);
  oled_disp.setTextColor(SH110X_WHITE);

  seg7_disp.begin(SEG7_DISP_ADDR);
  seg7_disp.setBrightness(2);
  seg7_disp.print("DLGE");
  seg7_disp.writeDisplay();

  staticFlip = false;
  ready = true;

  oled_disp.drawBitmap(46, 17, epd_bitmap_dlge3, 33, 30, SH110X_WHITE);
  drawOledBanner();
  announce("Loading...");
  oled_disp.display();
}

void Driver::tick() {
  const uint8_t interval = 50;
  static unsigned long previousMillis = 0;
  
  if ((millis() - previousMillis) >= interval) {
  previousMillis = millis();

  drawOledBanner();
  clearAnnounce();

  oled_disp.display();
  seg7_disp.writeDisplay();

  } // timer
}

void Driver::announce(const char *message) {
  if (!ready) {
    return;
  }
  lastAnnounce = millis();

  oled_disp.fillRect(0, 2, 100, OFFY, SH110X_BLACK);
  oled_disp.setCursor(0, 2);
  oled_disp.setTextColor(SH110X_WHITE);
  oled_disp.print(message);
}

void Driver::clearAnnounce(){
  const int announceHold = 1000;
  if (millis() - lastAnnounce >= announceHold)
  {
    oled_disp.fillRect(0, 0, 100, 14, SH110X_BLACK);
    lastAnnounce = millis();
  }
}

void Driver::handleScreenSysexMessage(uint8_t *data, size_t length){
  // use incoming data to decide what to do
  if(data[3] == uint8_t{0x40}){
    if (data[4] == uint8_t{0x01}) {
      drawOLED(data, length);
    }   
    else if (data[4] == uint8_t{0x02}) {
      drawOLEDDelta(data, length);
    }
  }
  else if (data[3] == uint8_t{0x41} && data[4] == uint8_t{0x00}){
      driver.draw7seg(data, length);
  }
  else{
    return;
  }
}

void Driver::drawOLED(uint8_t *data, size_t length) {
  uint8_t *packed = data + 6;
  size_t packed_len = length - 6;

  uint8_t unpacked[OLED_DATA_LEN];
  int unpacked_len = unpack_7to8_rle(unpacked, OLED_DATA_LEN, packed, packed_len);
  // Serial1.printf("reset %d as %u\n", unpacked_len, packed_len);

  if (unpacked_len < 0) {
    SER.printf("Hit exception, %d\n", unpacked_len);
    return;
  }

  if (unpacked_len == OLED_DATA_LEN) {
    memcpy(oledData, unpacked, unpacked_len);
  }

  drawOLEDData(unpacked, unpacked_len);
}

void Driver::drawOLEDDelta(uint8_t *data, size_t length) {
  uint8_t first = data[5];
  uint8_t len = data[6];

  uint8_t *packed = data + 7;
  size_t packed_len = length - 7;

  uint8_t unpacked[OLED_DATA_LEN];
  int unpacked_len = unpack_7to8_rle(unpacked, OLED_DATA_LEN, packed, packed_len);

  if (unpacked_len < 0) {
    SER.printf("Hit exception, %d\n", unpacked_len);
    return;
  }

  memcpy(oledData+(8*first), unpacked, 8*len);

  drawOLEDData(oledData, OLED_DATA_LEN);
}

void Driver::draw7seg(uint8_t *data, size_t length) {
  if (!ready) return;
  uint8_t subArray[] = {data[7],data[8],data[9],data[10]};
  uint8_t dots = data[6];

  seg7_disp.clear();

  bool dot;
  uint8_t digit;
  uint8_t * test;

  for (unsigned idx = 0; idx < 4 * 7; idx++)
  {
    unsigned hexIdx = idx/7;
    unsigned hexJdx = idx%7;
  }

  for (unsigned idx = 0; idx < 4; idx++)
  {
    for (unsigned jdx = 0; jdx < 7; jdx++)
    {
      unsigned hexIdx = (idx*7+jdx)/8;
      unsigned hexJdx = (idx*7+jdx)%8;
      digit |= (subArray[idx] & (1 << jdx)) ? (1 << seg7Dict[jdx]) : 0;
    }
    digit |= (dots & (1 << idx)) ? (1 << 7) : 0;
    unsigned pos = idx > 1 ? idx : idx + 1;
    seg7_disp.writeDigitRaw(pos, digit);
  }

  // draw static display
  drawOledStatic();
}

void Driver::drawOLEDData(uint8_t *data, size_t data_len) {
  if (!ready) return;

  oled_disp.startWrite();
  oled_disp.fillRect(OFFX, OFFY, OFFX+128, OFFY+48, SH110X_BLACK);

  const uint8_t blk_width = 128;

  for (uint8_t blk = 0; blk < 6; blk++) {
    for (uint8_t rstride = 0; rstride < 8; rstride++) {
      uint8_t mask = 1 << rstride;
      for (uint8_t j = 0; j < blk_width; j++) {
        if ((blk*blk_width+j) > data_len) {
          break;
        }
        uint8_t idata = (data[blk*blk_width+j] & mask);

        uint8_t y = blk*8 + rstride;

        if (idata > 0) {
          oled_disp.drawPixel(OFFX+j, OFFY+y, SH110X_WHITE);
        }
      }
    }
  }
  oled_disp.endWrite();
  showing_remote = true;

  //draw static display
  drawSeg7Static();
}

void Driver::drawOledStatic(){
  const uint16_t interval = 1000;
  static unsigned long previousMillis = 0;
  
  if ((millis() - previousMillis) >= interval) {
  previousMillis = millis();
  // TODO: create static/moving background for when OLED is not in use
  oled_disp.startWrite();
  oled_disp.fillRect(0, 17, 150, 48, SH110X_BLACK);
  oled_disp.drawBitmap(46, 17, epd_bitmap_dlge3, 33, 30, SH110X_WHITE);
  oled_disp.endWrite();
  } // timer
}

void Driver::drawOledBanner(){
  // const uint16_t interval_ms = 1000;
  // static uint16_t start_ms = 0;
  // if (millis() - start_ms < interval_ms) {
  //   return;
  // }
  // start_ms += interval_ms;

  const uint16_t interval = 1000;
  static unsigned long previousMillis = 0;
  
  if ((millis() - previousMillis) >= interval) {
  previousMillis = millis();

  bobDown = !bobDown;

  oled_disp.startWrite();

  // TODO: write battery level indicator
  // TODO: write isWorking indicator
 
  // TODO: create static/moving background for when OLED is not in use
  oled_disp.fillRect(117, 0, 11, 12, SH110X_BLACK);
  if(bobDown){
    oled_disp.drawBitmap(117, 2, epd_bitmap_dlge1, 11, 10, SH110X_WHITE);
  }
  else {
    oled_disp.drawBitmap(117, 0, epd_bitmap_dlge1, 11, 10, SH110X_WHITE);
  }
  // TODO: write whatever else I want on the banner

  oled_disp.endWrite();
  }// timer
}

void Driver::drawSeg7Static(){
  const uint16_t interval = 1000;
  static unsigned long previousMillis = 0;
  
  if ((millis() - previousMillis) >= interval) {
  previousMillis = millis();
  
  seg7_disp.clear();
  seg7_disp.write("DLGE",4);
  } // timer
}
} // namespace Display