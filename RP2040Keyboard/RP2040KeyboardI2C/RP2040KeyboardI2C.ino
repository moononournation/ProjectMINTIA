/* Key matrix config */
#define KEY_ROWS 5
#define KEY_COLS 13
const int row_pins[KEY_ROWS] = {13, 14, 15, 26, 27};
const int col_pins[KEY_COLS] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};

#define I2C_DEV_ADDR 0x55
#define I2C_SDA 28
#define I2C_SCL 29

#include <Wire.h>

#define KEY_ESC 27
#define KEY_BACKSPACE 8
#define KEY_TAB 9
#define KEY_CAPS_LOCK 0
#define KEY_RETURN 13
#define KEY_LEFT_SHIFT 16
#define KEY_FN 0
#define KEY_LEFT_CTRL 17
#define KEY_LEFT_ALT 18
#define KEY_LEFT_GUI 91
#define KEY_DELETE 46
#define KEY_PAGEUP 33
#define KEY_PAGEDOWN 34
#define KEY_HOME 36
#define KEY_END 35
#define KEY_UP_ARROW 38
#define KEY_DOWN_ARROW 40
#define KEY_LEFT_ARROW 37
#define KEY_RIGHT_ARROW 39

uint8_t key_map[KEY_ROWS][KEY_COLS] = {
  {KEY_ESC, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', KEY_BACKSPACE},
  {KEY_TAB, 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']'},
  {KEY_CAPS_LOCK, '`', 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', KEY_RETURN},
  {KEY_LEFT_SHIFT, KEY_FN, 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', KEY_HOME, KEY_UP_ARROW, KEY_END},
  {KEY_LEFT_CTRL, KEY_LEFT_ALT, KEY_LEFT_GUI, KEY_DELETE, '\'', '\\', ' ', '/', '=', '.', KEY_LEFT_ARROW, KEY_DOWN_ARROW, KEY_RIGHT_ARROW},
};
uint8_t shift_key_map[KEY_ROWS][KEY_COLS] = {
  {KEY_ESC, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', KEY_BACKSPACE},
  {KEY_TAB, 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}'},
  {KEY_CAPS_LOCK, '~', 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', KEY_RETURN},
  {KEY_LEFT_SHIFT, KEY_FN, 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', KEY_HOME, KEY_UP_ARROW, KEY_END},
  {KEY_LEFT_CTRL, KEY_LEFT_ALT, KEY_LEFT_GUI, KEY_DELETE, '"', '|', ' ', '?', '+', '>', KEY_LEFT_ARROW, KEY_DOWN_ARROW, KEY_RIGHT_ARROW},
};

uint32_t old_key_state[KEY_ROWS] = {0};
uint32_t key_state[KEY_ROWS] = {0};
uint32_t key_mask;

void setup()
{
  // Serial.begin(115200);
  // Serial.println("MINTIA Keyboard I2C");

  Wire.setSDA(I2C_SDA);
  Wire.setSCL(I2C_SCL);
  Wire.begin(I2C_DEV_ADDR);
  Wire.onReceive(recv);
  Wire.onRequest(req);

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
  delay(8); // align with normal USB Keyboard with refresh rate 125 Hz
}

// These are called in an **INTERRUPT CONTEXT** which means NO serial port
// access (i.e. Serial.print is illegal) and no memory allocations, etc.

// Called when the I2C slave gets written to
void recv(int len)
{
  int i;
  // Just stuff the sent bytes into a global the main routine can pick up and use
  for (i = 0; i < len; i++)
  {
    // Serial.print(Wire.read());
  }
  // Serial.println();
}

// Called when the I2C slave is read from
void req()
{
  uint32_t oks, ks, bit;
  bool shift_pressed = (key_state[3] & 1) == 0;
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
          if (ks & bit) {
            // Serial.printf("row: %d, col: %d, released\n", i, j);
          } else {
            if (shift_pressed) {
              Wire.write(shift_key_map[i][j]);
            } else {
              Wire.write(key_map[i][j]);
            }
            // Serial.printf("row: %d, col: %d, pressed\n", i, j);
          }
        }
      }
    }
  }

  // at least send 1 byte response
  Wire.write(0);

  for (int i = 0; i < KEY_ROWS; ++i)
  {
    old_key_state[i] = key_state[i];
  }
}
