#include "defs.hpp"
#include "display.hpp"
#include "rle.hpp"

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

  isOled = false;
  ready = true;
  announce("Loading...");
}

void Driver::tick() {
  const uint32_t interval_ms = 100;
  static uint32_t start_ms = 0;
  if (millis() - start_ms < interval_ms) {
    return;
  }
  start_ms += interval_ms;

  drawOledBanner();
  
  oled_disp.display();
}

void Driver::announce(const char *message) {
  SER.println(message);
  if (!ready) {
    return;
  }

  oled_disp.fillRect(0, 48, 127, OFFY, SH110X_BLACK);
  oled_disp.setCursor(0, 48);
  oled_disp.setTextColor(SH110X_WHITE);
  oled_disp.print(message);
}

void Driver::drawOLED(uint8_t *data, size_t length) {

  uint8_t *packed = data + 6;
  size_t packed_len = length - 6;

  uint8_t unpacked[OLED_DATA_LEN];
  //SER.print("Unpacking");
  int unpacked_len = unpack_7to8_rle(unpacked, OLED_DATA_LEN, packed, packed_len);

  // Serial1.printf("reset %d as %u\n", unpacked_len, packed_len);

  if (unpacked_len < 0) {
    SER.printf("Hit exception, %d\n", unpacked_len);
    while (1) {};
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
  // SER.printf("first %u, len %u, delta size %d as %u\n", first, len, unpacked_len, packed_len);

  if (unpacked_len < 0) {
    SER.printf("Hit exception, %d\n", unpacked_len);
    while (1) {};
  }

  memcpy(oledData+(8*first), unpacked, 8*len);
  // SER.println("Completed memcpy.");

  drawOLEDData(oledData, OLED_DATA_LEN);
}

void Driver::draw7seg(uint8_t *array) {
  if (!ready) return;
  uint8_t subArray[] = {array[7],array[8],array[9],array[10]};
  uint8_t dots = array[6];
  // draw static display
  if (isOled){
    isOled = false;
    drawOledStatic();
  }

  bool dot;
  for (unsigned idx = 0; idx < 4; idx++)
  {
    seg7_disp.writeDigitRaw(idx, subArray[idx]);
    seg7_disp.writeDigitAscii(idx, 32, dots & (1 << idx) != 0);
  }
  seg7_disp.clear();
  seg7_disp.writeDisplay();
  // @todo Write 7segment rendering code
}

void Driver::drawOLEDData(uint8_t *data, size_t data_len) {
  if (!ready) return;
  
  //draw static display
  if (!isOled) {
    isOled = true;
    seg7_disp.clear();
    seg7_disp.print("DLGE");
    seg7_disp.writeDisplay();
  }

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
}

void Driver::drawOledStatic(){
  // TODO: create static/moving background for when OLED is not in use
}

void Driver::drawOledBanner(){
  // TODO: write battery level indicator
  // TODO: write isWorking indicator
  // TODO: write whatever else I want on the banner
}

} // namespace Display