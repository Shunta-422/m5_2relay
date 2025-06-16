#include <M5GFX.h>
#include <Wire.h>
#include "MODULE_2RELAY.h"

#define I2C_SDA 12
#define I2C_SCL 11

M5GFX display;
MODULE_2RELAY relay;

void drawRect() {
  display.fillScreen(0);
  display.setTextColor(ORANGE);
  display.setTextSize(12);
  display.println("Module AC 2Relay");
  display.setTextSize(10);
  display.println("A REVERSE");
  display.println("B STEP");
  display.println("C RUNNING");
  display.println("FW VERSION: " + String(relay.getVersion()));
  for (uint8_t i = 0; i < 2; i++) {
    if (relay.getRelayState(i) == 1) {
      display.fillRect(20, 80 + 70 * i, 100, 20);
    } else {
      display.drawRect(20, 80 + 70 * i, 100, 20);
    }
  }
}

void setup() {
  display.init();
  display.begin();
  display.setFont(&fonts::lgfxJapanGothic_12);
  while (!relay.begin(&Wire, I2C_SDA, I2C_SCL, MODULE_2RELAY_ADDR)) {
    Serial.println("AC 2RELAY INIT ERROR");
    delay(1000);
  }
  
  
  drawRect();
}
uint8_t mode = 0;
void loop() {
  switch (mode) {
    case 0:
      relay.setAllRelay(false);
      drawRect();
      break;
    case 1:
      relay.setRelay(0, true);
      drawRect();
      break;
    case 2:
      relay.setRelay(1, true);
      drawRect();
      break;
    case 3:
      relay.setAllRelay(false);
      drawRect();
      break;
  }
  delay(1000); // 1秒待機
  mode++;
  if (mode > 3) {
    mode = 0;
  }
}