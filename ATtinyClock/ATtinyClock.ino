#include <DS3231.h>
DS3231 dsc3231;

#include "ssd1306.h"
SSD1306 oled;

#include "func.h"

void setup()
{
  Wire.begin();
  oled.begin();
  oled.fill(0x00); // clear in black
}

void loop()
{
  draw_time();
}
