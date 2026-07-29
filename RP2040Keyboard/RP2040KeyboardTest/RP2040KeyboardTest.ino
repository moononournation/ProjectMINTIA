/* Key matrix config */
#define KEY_ROWS 5
#define KEY_COLS 13 /* pins 0-12 */
const int row_pins[KEY_ROWS] = {13, 14, 15, 26, 27};


uint16_t old_key_state[KEY_ROWS] = {0};

void setup()
{
  Serial.begin(115200);
  Serial.println("MINTIA Keyboard");

  for (int i = 0; i < KEY_ROWS; ++i)
  {
    pinMode(row_pins[i], OUTPUT);
    digitalWrite(row_pins[i], HIGH);
  }
  for (int j = 0; j < KEY_COLS; ++j)
  {
    pinMode(j, INPUT_PULLUP);
  }
}

void loop()
{
  uint16_t key_state[KEY_ROWS] = {0};
  for (int i = 0; i < KEY_ROWS; ++i)
  {
    digitalWrite(row_pins[i], LOW);
    key_state[i] = sio_hw->gpio_in & 0b0001111111111111;
    digitalWrite(row_pins[i], HIGH);
  }

  uint16_t oks, ks, bit;
  for (int i = 0; i < KEY_ROWS; ++i)
  {
    oks = old_key_state[i];
    ks = key_state[i];
    if (ks != oks)
    {
      for (int j = 0; j < KEY_COLS; ++j)
      {
        bit = 1 << j;
        if ((ks & bit) != (oks & bit)) {
          if (ks & bit) {
            Serial.printf("row: %d, col: %d, released\n", i, j);
          } else {
            Serial.printf("row: %d, col: %d, pressed\n", i, j);
          }
        }
      }
    }
  }

  for (int i = 0; i < KEY_ROWS; ++i)
  {
    old_key_state[i] = key_state[i];
  }
}
