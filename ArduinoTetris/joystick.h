/*******************************************************************************
 * Arduino Tetris
 * Revise from: https://github.com/moononournation/Handheld-Color-Console.git
 ******************************************************************************/

#pragma once

// defined in Dev Device Pins
// #define BTN_A 21
// #define BTN_B 20
// #define BTN_SELECT 16
// #define BTN_START 17
// #define BTN_UP 0
// #define BTN_DOWN 1
// #define BTN_LEFT 2
// #define BTN_RIGHT 3

#define INPUT_RATE 200 // ms

bool joystick_btn_a_pressed;
bool joystick_btn_b_pressed;
bool joystick_btn_select_pressed;
bool joystick_btn_start_pressed;
bool joystick_btn_up_pressed;
bool joystick_btn_down_pressed;
bool joystick_btn_left_pressed;
bool joystick_btn_right_pressed;

unsigned long joystick_last_btn_a_pressed;
unsigned long joystick_last_btn_b_pressed;
unsigned long joystick_last_btn_select_pressed;
unsigned long joystick_last_btn_start_pressed;
unsigned long joystick_last_btn_up_pressed;
unsigned long joystick_last_btn_down_pressed;
unsigned long joystick_last_btn_left_pressed;
unsigned long joystick_last_btn_right_pressed;

static void joystick_btn_a_press()
{
  if ((millis() - joystick_last_btn_a_pressed) > INPUT_RATE) {
    joystick_btn_a_pressed = true;
    joystick_last_btn_a_pressed = millis();
  }
}
static void joystick_btn_b_press()
{
  if ((millis() - joystick_last_btn_b_pressed) > INPUT_RATE) {
    joystick_btn_b_pressed = true;
    joystick_last_btn_b_pressed = millis();
  }
}
static void joystick_btn_select_press()
{
  if ((millis() - joystick_last_btn_select_pressed) > INPUT_RATE) {
    joystick_btn_select_pressed = true;
    joystick_last_btn_select_pressed = millis();
  }
}
static void joystick_btn_start_press()
{
  if ((millis() - joystick_last_btn_start_pressed) > INPUT_RATE) {
    joystick_btn_start_pressed = true;
    joystick_last_btn_start_pressed = millis();
  }
}
static void joystick_btn_up_press()
{
  if ((millis() - joystick_last_btn_up_pressed) > INPUT_RATE) {
    joystick_btn_up_pressed = true;
    joystick_last_btn_up_pressed = millis();
  }
}
static void joystick_btn_down_press()
{
  if ((millis() - joystick_last_btn_down_pressed) > INPUT_RATE) {
    joystick_btn_down_pressed = true;
    joystick_last_btn_down_pressed = millis();
  }
}
static void joystick_btn_left_press()
{
  if ((millis() - joystick_last_btn_left_pressed) > INPUT_RATE) {
    joystick_btn_left_pressed = true;
    joystick_last_btn_left_pressed = millis();
  }
}
static void joystick_btn_right_press()
{
  if ((millis() - joystick_last_btn_right_pressed) > INPUT_RATE) {
    joystick_btn_right_pressed = true;
    joystick_last_btn_right_pressed = millis();
  }
}

void joystick_init()
{
  pinMode(BTN_A, INPUT_PULLUP);
  pinMode(BTN_B, INPUT_PULLUP);
  pinMode(BTN_SELECT, INPUT_PULLUP);
  pinMode(BTN_START, INPUT_PULLUP);
  pinMode(BTN_UP, INPUT_PULLUP);
  pinMode(BTN_DOWN, INPUT_PULLUP);
  pinMode(BTN_LEFT, INPUT_PULLUP);
  pinMode(BTN_RIGHT, INPUT_PULLUP);

  joystick_btn_a_pressed = false;
  joystick_btn_b_pressed = false;
  joystick_btn_select_pressed = false;
  joystick_btn_start_pressed = false;
  joystick_btn_up_pressed = false;
  joystick_btn_down_pressed = false;
  joystick_btn_left_pressed = false;
  joystick_btn_right_pressed = false;

  joystick_last_btn_a_pressed = 0;
  joystick_last_btn_b_pressed = 0;
  joystick_last_btn_select_pressed = 0;
  joystick_last_btn_start_pressed = 0;
  joystick_last_btn_up_pressed = 0;
  joystick_last_btn_down_pressed = 0;
  joystick_last_btn_left_pressed = 0;
  joystick_last_btn_right_pressed = 0;

  attachInterrupt(BTN_A, joystick_btn_a_press, FALLING);
  attachInterrupt(BTN_B, joystick_btn_b_press, FALLING);
  attachInterrupt(BTN_SELECT, joystick_btn_select_press, FALLING);
  attachInterrupt(BTN_START, joystick_btn_start_press, FALLING);
  attachInterrupt(BTN_UP, joystick_btn_up_press, FALLING);
  attachInterrupt(BTN_DOWN, joystick_btn_down_press, FALLING);
  attachInterrupt(BTN_LEFT, joystick_btn_left_press, FALLING);
  attachInterrupt(BTN_RIGHT, joystick_btn_right_press, FALLING);
}

bool joystick_is_btn_a_pressing()
{
  return digitalRead(BTN_A) == 0;
}
bool joystick_is_btn_b_pressing()
{
  return digitalRead(BTN_B) == 0;
}
bool joystick_is_btn_select_pressing()
{
  return digitalRead(BTN_SELECT) == 0;
}
bool joystick_is_btn_start_pressing()
{
  return digitalRead(BTN_START) == 0;
}
bool joystick_is_btn_up_pressing()
{
  return digitalRead(BTN_UP) == 0;
}
bool joystick_is_btn_down_pressing()
{
  return digitalRead(BTN_DOWN) == 0;
}
bool joystick_is_btn_left_pressing()
{
  return digitalRead(BTN_LEFT) == 0;
}
bool joystick_is_btn_right_pressing()
{
  return digitalRead(BTN_RIGHT) == 0;
}

// pressed function
bool joystick_is_btn_a_pressed()
{
  if (joystick_btn_a_pressed)
  {
    joystick_btn_a_pressed = false;
    return true;
  }
  return false;
}
bool joystick_is_btn_b_pressed()
{
  if (joystick_btn_b_pressed)
  {
    joystick_btn_b_pressed = false;
    return true;
  }
  return false;
}
bool joystick_is_btn_select_pressed()
{
  if (joystick_btn_select_pressed)
  {
    joystick_btn_select_pressed = false;
    return true;
  }
  return false;
}
bool joystick_is_btn_start_pressed()
{
  if (joystick_btn_start_pressed)
  {
    joystick_btn_start_pressed = false;
    return true;
  }
  return false;
}
bool joystick_is_btn_up_pressed()
{
  if (joystick_btn_up_pressed)
  {
    joystick_btn_up_pressed = false;
    return true;
  }
  return false;
}
bool joystick_is_btn_down_pressed()
{
  if (joystick_btn_down_pressed)
  {
    joystick_btn_down_pressed = false;
    return true;
  }
  return false;
}
bool joystick_is_btn_left_pressed()
{
  if (joystick_btn_left_pressed)
  {
    joystick_btn_left_pressed = false;
    return true;
  }
  return false;
}
bool joystick_is_btn_right_pressed()
{
  if (joystick_btn_right_pressed)
  {
    joystick_btn_right_pressed = false;
    return true;
  }
  return false;
}
