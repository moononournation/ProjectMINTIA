/* Key matrix config */
#define KEY_ROWS 5
#define KEY_COLS 13
const int row_pins[KEY_ROWS] = {13, 14, 15, 26, 27};
const int col_pins[KEY_COLS] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};

#include <Keyboard.h>
/**
 * @brief key constant definition
 * https://github.com/earlephilhower/Keyboard/blob/master/src/HID_Keyboard.h
 */

uint8_t key_map[KEY_ROWS][KEY_COLS] = {
  {KEY_ESC, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', KEY_BACKSPACE},
  {KEY_TAB, 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']'},
  {'`', KEY_CAPS_LOCK, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', KEY_RETURN},
  {'\'', KEY_LEFT_SHIFT, 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', KEY_HOME, KEY_UP_ARROW, KEY_END},
  {0, KEY_LEFT_CTRL, KEY_LEFT_ALT, KEY_LEFT_GUI, KEY_DELETE, '\\', ' ', '/', '=', '.', KEY_LEFT_ARROW, KEY_DOWN_ARROW, KEY_RIGHT_ARROW},
};

#define FN_ROW_IDX 4
#define FN_COL_IDX 0
#define FN_KEY_PAIRS 20
uint8_t key_fn_map[FN_KEY_PAIRS * 2] = {
  /* from and to key pair */
  '1', KEY_F1,
  '2', KEY_F2,
  '3', KEY_F3,
  '4', KEY_F4,
  '5', KEY_F5,
  '6', KEY_F6,
  '7', KEY_F7,
  '8', KEY_F8,
  '9', KEY_F9,
  '0', KEY_F10,
  '-', KEY_F11,
  KEY_BACKSPACE, KEY_F12,
  KEY_LEFT_SHIFT, KEY_RIGHT_SHIFT,
  KEY_HOME, KEY_PAGE_UP,
  KEY_END, KEY_PAGE_DOWN,
  KEY_LEFT_CTRL, KEY_RIGHT_CTRL,
  KEY_LEFT_ALT, KEY_RIGHT_ALT,
  KEY_LEFT_GUI, KEY_RIGHT_GUI,
  KEY_DELETE, KEY_INSERT,
  '=', KEY_MENU,
};

uint32_t old_key_state[KEY_ROWS] = {0};
uint32_t key_state[KEY_ROWS] = {0};
uint32_t key_mask;

void setup()
{
  Serial.begin(115200);
  Serial.println("MINTIA Keyboard USB");

  Keyboard.begin();

  // init row pins
  for (int i = 0; i < KEY_ROWS; ++i)
  {
    pinMode(row_pins[i], OUTPUT);
    digitalWrite(row_pins[i], HIGH);
  }

  // init column pins
  key_mask = 0;
  for (int j = 0; j < KEY_COLS; ++j)
  {
    pinMode(col_pins[j], INPUT_PULLUP);
    key_mask |= 1 << col_pins[j];
  }
}

void loop()
{
  for (int i = 0; i < KEY_ROWS; ++i)
  {
    digitalWrite(row_pins[i], LOW);
    key_state[i] = sio_hw->gpio_in & key_mask;
    digitalWrite(row_pins[i], HIGH);
  }

  uint32_t oks, ks, bit;
  bool fn_pressed = ((key_state[FN_ROW_IDX]) & (1 << FN_COL_IDX)) == 0;
  bool has_key_pressed = false;
  for (int i = 0; i < KEY_ROWS; ++i)
  {
    oks = old_key_state[i];
    ks = key_state[i];

    if (ks != oks)
    {
      for (int j = 0; j < KEY_COLS; ++j)
      {
        bit = 1 << col_pins[j];
        if ((ks & bit) != (oks & bit)) {
          uint8_t k = key_map[i][j];
          if (fn_pressed) {
            // translate fn key
            for (int f = 0; f < (FN_KEY_PAIRS * 2); f += 2) {
              if (k == key_fn_map[f]) {
                Serial.printf("translate %d to %d./n", k, key_fn_map[f + 1]);
                k = key_fn_map[f + 1];
              }
            }
          }

          if (ks & bit) {
            Keyboard.release(k);
            Serial.printf("row: %d, col: %d, released\n", i, j);
          } else {
            has_key_pressed = true;
            Keyboard.press(k);
            Serial.printf("row: %d, col: %d, pressed\n", i, j);
          }
        }
      }
    }
  }
  if (!has_key_pressed) {
    Keyboard.releaseAll();
  }

  for (int i = 0; i < KEY_ROWS; ++i)
  {
    old_key_state[i] = key_state[i];
  }
}
