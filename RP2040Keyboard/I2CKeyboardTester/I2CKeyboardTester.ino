#define I2C_DEV_ADDR 0x55
#define I2C_SDA 5
#define I2C_SCL 6

#include <Wire.h>

void setup() {
  Serial.begin(115200);
  // Serial.setDebugOutput(true);
  // while(!Serial);
  Serial.println("I2C Keyboard Client Test");

  Wire.begin(I2C_SDA, I2C_SCL);
  Wire.requestFrom(I2C_DEV_ADDR, 1);
  if (Wire.read() == -1)
  {
    Serial.println("I2C Keyboad not online!");
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  Wire.requestFrom(I2C_DEV_ADDR, 1);
  if (Wire.available() > 0) {
    while (Wire.available() > 0) {
      char c = Wire.read();
      if (c) {
        Serial.println(c);
      }
    }
  }
}
