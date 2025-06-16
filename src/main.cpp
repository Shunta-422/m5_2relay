#include <M5GFX.h>
#include <Wire.h>

#define I2C_SDA 12
#define I2C_SCL 11
#define MODULE_2RELAY_ADDR 0x25
#define MODULE_2RELAY_REG  0x00
#define MODULE_2RELAY_VERSION_REG 0xFE
#define MODULE_2RELAY_ADDR_CONFIG_REG 0xFF

bool writeBytes(uint8_t addr, uint8_t reg, uint8_t* buffer, uint8_t length);
bool readBytes(uint8_t addr, uint8_t reg, uint8_t* buffer, uint8_t length);
bool begin(TwoWire* wire, uint8_t sda, uint8_t scl, uint8_t addr);
bool getRelayState(uint8_t index);
bool setRelay(uint8_t index, bool state);

bool begin(TwoWire* wire, uint8_t sda, uint8_t scl, uint8_t addr) {
  wire->begin(sda, scl);
  delay(10);
  wire->beginTransmission(addr);
  uint8_t error = wire->endTransmission();
  if(error == 0){
    return true;
  }
  else {
    return false;
  }
}

bool writeBytes(uint8_t addr, uint8_t reg, uint8_t* buffer, uint8_t length) {
  Wire.beginTransmission(addr);
  Wire.write(reg);
  Wire.write(buffer, length);
  return (Wire.endTransmission() == 0);
}

bool readBytes(uint8_t addr, uint8_t reg, uint8_t* buffer, uint8_t length) {
  uint8_t index = 0;

  Wire.beginTransmission(addr);
  Wire.write(reg);
  Wire.endTransmission();
  if(Wire.requestFrom(addr, length)) {
    for(uint8_t i = 0; i < length; i++) {
      buffer[index++] = Wire.read();
    }
    return true;
  }
  return false;
}

bool getRelayState(uint8_t index) {
  uint8_t data = 0;

  if(readBytes(MODULE_2RELAY_ADDR, MODULE_2RELAY_REG + index, &data, 1)) {
    return data ? true : false;
  }
  return false;
}

bool setRelay(uint8_t index, bool state) {
  uint8_t data = state ? 0xFF : 0x00;

  if (writeBytes(MODULE_2RELAY_ADDR, MODULE_2RELAY_REG + index, &data, 1)) {
    return data ? true : false;
  }
  return false;
} 


M5GFX display;

void setup() {
  display.init();
  display.begin();
  display.setFont(&fonts::lgfxJapanGothic_12);
  while(!begin(&Wire, I2C_SDA, I2C_SCL, MODULE_2RELAY_ADDR)) {
    display.println("AC 2RELAY INIT ERROR");
    delay(1000);
  }
  display.println("AC 2RELAY INIT OK");
}
uint8_t mode = 0;
void loop() {
  switch (mode) {
    case 0:
      setRelay(0, false);
      setRelay(1, false);
      if(getRelayState(0)) {
        display.println("Relay 0: ON");
      } else {
        display.println("Relay 0: OFF");
      }
      if(getRelayState(1)) {
        display.println("Relay 1: ON");
      } else {
        display.println("Relay 1: OFF");
      }
      break;
    case 1:
      setRelay(0, true);
      setRelay(1, false);
      if(getRelayState(0)) {
        display.println("Relay 0: ON");
      } else {
        display.println("Relay 0: OFF");
      }
      if(getRelayState(1)) {
        display.println("Relay 1: ON");
      } else {
        display.println("Relay 1: OFF");
      }
      break;
    case 2:
      setRelay(0, false);
      setRelay(1, true);
      if(getRelayState(0)) {
        display.println("Relay 0: ON");
      } else {
        display.println("Relay 0: OFF");
      }
      if(getRelayState(1)) {
        display.println("Relay 1: ON");
      } else {
        display.println("Relay 1: OFF");
      }
      break;
    case 3:
      setRelay(0, true);
      setRelay(1, true);
      if(getRelayState(0)) {
        display.println("Relay 0: ON");
      } else {
        display.println("Relay 0: OFF");
      }
      if(getRelayState(1)) {
        display.println("Relay 1: ON");
      } else {
        display.println("Relay 1: OFF");
      }
      break;
  }
  delay(1000); // 1秒待機
  mode++;
  if (mode > 3) {
    mode = 0;
  }
}