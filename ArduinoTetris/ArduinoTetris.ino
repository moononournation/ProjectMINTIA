/*******************************************************************************
 * Arduino Tetris
 * Revise from: https://github.com/moononournation/Handheld-Color-Console.git
 ******************************************************************************/

// Dev Device Pins: <https://github.com/moononournation/Dev_Device_Pins.git>
#include "PINS_MINTIA_PICO_CONSOLE.h"

#include "joystick.h"
#include "Tetris.h"
Tetris t;

void setup()
{
  // Init Display
  if (!gfx->begin(GFX_SPEED))
  {
    Serial.println("gfx->begin() failed!");
  }
  gfx->fillScreen(RGB565_BLACK);

  t.init(gfx);
  t.drawPreGameScreen();

#ifdef GFX_BL
  pinMode(GFX_BL, OUTPUT);
  digitalWrite(GFX_BL, HIGH);
#endif // GFX_BL

  // initialize joystick
  joystick_init();
}

void loop()
{
  // wait for click
  while (!joystick_is_btn_a_pressed())
    ;

  // load game
  t.run();

  // game ended
  t.gameOver();
}
