/*******************************************************************************
 * Arduino Tetris
 * Revise from: https://github.com/moononournation/Handheld-Color-Console.git
 ******************************************************************************/

#pragma once

// #define RANDOM_PIN 0

#define FONT_WIDTH 6
#define FONT_HEIGHT 8

#define BOARD_WIDTH 10
#define BOARD_HEIGHT 20

#define PIT_COLOR RGB565_WHITE
#define BG_COLOR RGB565_BLACK

#define DROP_WAIT_INIT 1100

#define INPUT_WAIT_ROT 300
#define INPUT_WAIT_MOVE 500

#define INPUT_WAIT_NEW_SHAPE 400

// used to clear the position from the screen
struct Backup
{
  byte x, y, rot;
};

class Tetris
{
  // shapes definitions

  byte l_shape[4][4][2]{
      {{0, 0}, {0, 1}, {0, 2}, {1, 2}},
      {{0, 1}, {1, 1}, {2, 0}, {2, 1}},
      {{0, 0}, {1, 0}, {1, 1}, {1, 2}},
      {{0, 0}, {0, 1}, {1, 0}, {2, 0}},
  };

  byte j_shape[4][4][2]{
      {{1, 0}, {1, 1}, {0, 2}, {1, 2}},
      {{0, 0}, {1, 0}, {2, 0}, {2, 1}},
      {{0, 0}, {1, 0}, {0, 1}, {0, 2}},
      {{0, 0}, {0, 1}, {1, 1}, {2, 1}},
  };

  byte o_shape[1][4][2]{
      {{0, 0}, {0, 1}, {1, 0}, {1, 1}}};

  byte s_shape[2][4][2]{
      {{0, 1}, {1, 0}, {1, 1}, {2, 0}},
      {{0, 0}, {0, 1}, {1, 1}, {1, 2}}};

  byte z_shape[2][4][2]{
      {{0, 0}, {1, 0}, {1, 1}, {2, 1}},
      {{1, 0}, {0, 1}, {1, 1}, {0, 2}}};

  byte t_shape[4][4][2]{
      {{0, 0}, {1, 0}, {2, 0}, {1, 1}},
      {{0, 0}, {0, 1}, {1, 1}, {0, 2}},
      {{1, 0}, {0, 1}, {1, 1}, {2, 1}},
      {{1, 0}, {0, 1}, {1, 1}, {1, 2}}};

  byte i_shape[2][4][2]{
      {{0, 0}, {1, 0}, {2, 0}, {3, 0}},
      {{0, 0}, {0, 1}, {0, 2}, {0, 3}}};

  // All game shapes and their colors

  byte *all_shapes[7] = {l_shape[0][0], j_shape[0][0], o_shape[0][0], s_shape[0][0], z_shape[0][0], t_shape[0][0], i_shape[0][0]};

  unsigned int colors[7] = {RGB565_ORANGE, RGB565_BLUE, RGB565_YELLOW, RGB565_GREEN, RGB565_RED, RGB565_MAGENTA, RGB565_CYAN};

  // how many rotated variations each shape has

  byte shapes[7] = {4, 4, 1, 2, 2, 4, 2};

  bool newShape;

  // game progress

  int lines, level;

  // current shapes

  byte current;

  // tetris guidelines have all 7 shapes
  // selected in sequence to avoid
  // long runs without a shape

  byte next[7];
  byte next_c;

  unsigned long lastInput, lastDrop;

  byte board[BOARD_HEIGHT][BOARD_WIDTH];

  byte _x, _y, _rot;
  Backup _old;
  Arduino_GFX *_gfx;
  int16_t _w, _h, _cx, _cy;
  int16_t _block_size, _margin, _titleFontSize, _ggFontSize;
  int16_t _board_top, _board_right, _board_left, _board_center;
  int16_t _score_board_left, _score_board_top, _score_board_w, _score_board_h;
  int16_t _next_box_w, _next_box_h, _next_box_left, _next_box_top;

  unsigned long dropWait;

public:
  Tetris() : newShape(true), lines(0)
  {
  }

  void init(Arduino_GFX *g)
  {
    _gfx = g;
    _w = _gfx->width();
    _h = _gfx->height();
    _cx = _w / 2;
    _cy = _h / 2;
    _block_size = MIN(((_w - 2) / BOARD_WIDTH), ((_h - 2) / BOARD_HEIGHT));
    _titleFontSize = _w / FONT_WIDTH / 6;
    _ggFontSize = _block_size * 10 / FONT_WIDTH / 4;
    _margin = (_h - (BOARD_HEIGHT * _block_size)) / 2;
    _board_top = _margin;

    _next_box_w = (_block_size * 6);
    _next_box_h = (_block_size * 6);
    _next_box_left = (_w - _margin - _next_box_w);
    _next_box_top = _margin;
    _score_board_w = _next_box_w;
    _score_board_h = ((_margin * 5) + (FONT_HEIGHT * 6));
    _score_board_left = _next_box_left;
    _score_board_top = _next_box_top + _next_box_h + _margin;

    _board_right = (_score_board_left - _margin);
    _board_left = (_board_right - (BOARD_WIDTH * _block_size) + 1);
    _board_center = (_board_right + _board_left) / 2;

    // Random reference analog pin MUST NOT be connected to anything...
    // pinMode(RANDOM_PIN, INPUT);
    // randomSeed(analogRead(RANDOM_PIN));
    randomSeed(get_rand_32());
    random(7);
    random(7);
  }

  void drawBlock(uint16_t poX, uint16_t poY, uint16_t color)
  {
    _gfx->fillRect(poX, poY, _block_size, _block_size, color);

    uint16_t light_color = color | 0b1100011000011000;
    _gfx->drawFastHLine(poX, poY, _block_size - 1, light_color);

    uint16_t dark_color = color & 0b0011100111100111;
    _gfx->drawFastHLine(poX + 1, poY + _block_size - 1, _block_size - 1, dark_color);
    _gfx->drawFastVLine(poX + _block_size - 1, poY + 1, _block_size - 1, dark_color);
  }

  void fillRandomBlock()
  {
    for (int i = 0; i < _w; i += _block_size)
    {
      for (int j = 0; j < _h; j += _block_size)
      {
        drawBlock(i, j, colors[random(7)]);
      }
    }
  }

  void drawPreGameScreen()
  {
    fillRandomBlock();

    _gfx->setTextColor(RGB565_BLUE, BG_COLOR);
    _gfx->setTextSize(_titleFontSize, _titleFontSize, 1);
    _gfx->printCenterText("Tetris", _cx, _cy);
    _gfx->setTextColor(RGB565_WHITE, BG_COLOR);
    _gfx->setTextSize(1);
    _gfx->printCenterText("Click to play", _cx, _cy + (FONT_HEIGHT * _titleFontSize));
  }

  void drawStartGameScreen()
  {
    fillRandomBlock();

    // draw board
    _gfx->drawRect(_board_left - 1, _board_top - 1, (BOARD_WIDTH * _block_size) + 2, (BOARD_HEIGHT * _block_size) + 2, PIT_COLOR);
    _gfx->fillRect(_board_left, _board_top, BOARD_WIDTH * _block_size, BOARD_HEIGHT * _block_size, BG_COLOR);
  }

  void drawNextBox()
  {
    _gfx->drawRect(_next_box_left - 1, _next_box_top - 1, _next_box_w + 2, _next_box_h + 2, PIT_COLOR);
    _gfx->fillRect(_next_box_left, _next_box_top, _next_box_w, _next_box_h, BG_COLOR);
    byte *shape = all_shapes[next[next_c]];

    byte max_x = 1;
    byte max_y = 1;
    byte *block;
    for (int i = 0; i < 4; i++)
    {
      block = shape + i * 2;
      if (block[0] > max_x)
      {
        max_x = block[0];
      }
      if (block[1] > max_y)
      {
        max_y = block[1];
      }
    }

    int16_t l = _next_box_left + (_block_size * (5 - max_x) / 2);
    int16_t t = _next_box_top + (_block_size * (5 - max_y) / 2);
    for (int i = 0; i < 4; i++)
    {
      block = shape + i * 2;
      drawBlock(
          l + block[0] * _block_size,
          t + block[1] * _block_size,
          colors[next[next_c]]);
    }
  }

  void run()
  {
    // clear board
    for (int i = 0; i < BOARD_WIDTH; i++)
    {
      for (int j = 0; j < BOARD_HEIGHT; j++)
      {
        board[j][i] = 0;
      }
    }

    // next shape

    randomizer();

    // initialize game logic

    lastInput = 0;
    lastDrop = 0;
    dropWait = DROP_WAIT_INIT;
    level = 1;
    newShape = true;

    drawStartGameScreen();

    scoreBoard();

    do
    {
      // get clock
      const unsigned long now = millis();

      // display new shape
      if (newShape)
      {
        newShape = false;
        lastDrop = now;

        // a new shape enters the game
        chooseNewShape();

        drawNextBox();

        // check if new shape is placed over other shape(s)
        // on the board
        if (touches(0, 0, 0))
        {
          // draw shape to screen
          draw();
          return;
        }

        // draw shape to screen
        draw();
      }
      else
      {
        // check if enough time has passed since last time the shape
        // was moved down the board
        if (now - lastDrop > dropWait)
        {
          // update clock
          lastDrop = now;
          moveDown();
        }
      }

      if (!newShape)
      {
        userInput(now);
      }

    } while (true);
  }

  void gameOver()
  {
    _gfx->setTextColor(RGB565_RED, BG_COLOR);
    _gfx->setTextSize(_ggFontSize, _ggFontSize, 1);
    _gfx->printCenterText("GAME", _board_center, _cy - (FONT_HEIGHT * _ggFontSize / 2));
    _gfx->printCenterText("OVER", _board_center, _cy + (FONT_HEIGHT * _ggFontSize / 2));
  }

private:
  void chooseNewShape()
  {
    current = next[next_c];

    next_c++;

    if (next_c == 7)
    {
      randomizer();
    }

    // new shape must be positioned at the middle of
    // the top of the board
    // with zero rotation

    _rot = 0;
    _y = 0;
    _x = BOARD_WIDTH / 2;

    _old.rot = _rot;
    _old.y = _y;
    _old.x = _x;
  }

  void userInput(unsigned long now)
  {
    unsigned long waited = now - lastInput;

    if (joystick_is_btn_left_pressed() || (joystick_is_btn_left_pressing() && (waited > INPUT_WAIT_MOVE)))
    {
      // handle move left
      if (_x > 0 && !touches(-1, 0, 0))
      {
        --_x;
        lastInput = now;
        draw();
      }
    }
    else if (joystick_is_btn_right_pressed() || (joystick_is_btn_right_pressing() && (waited > INPUT_WAIT_MOVE)))
    {
      // handle move right
      if (_x < BOARD_WIDTH && !touches(1, 0, 0))
      {
        ++_x;
        lastInput = now;
        draw();
      }
    }
    else if (joystick_is_btn_down_pressed() || (joystick_is_btn_down_pressing() && (waited > dropWait)))
    {
      // handle move down
      lastDrop = now - (dropWait / 2);
      moveDown();
    }
    else if (joystick_is_btn_a_pressed() || (joystick_is_btn_a_pressing() && (waited > INPUT_WAIT_ROT)))
    {
      // handle rotate
      if (!touches(0, 0, 1))
      {
        ++_rot;
        _rot %= shapes[current];
        lastInput = now;
        draw();
      }
    }
  }

  void moveDown()
  {
    // prepare to move down
    // check if board is clear bellow

    if (touches(0, 1, 0))
    {
      // moving down touches another shape
      newShape = true;

      // this shape wont move again
      // add it to the board

      byte *shape = all_shapes[current];
      for (int i = 0; i < 4; i++)
      {
        byte *block = (shape + (_rot * 4 + i) * 2);
        board[block[1] + _y][block[0] + _x] = current + 1;
      }

      // check if lines were made

      score();
    }
    else
    {
      // move shape down
      ++_y;
      draw();
    }
  }

  void draw()
  {
    byte *shape = all_shapes[current];
    for (int i = 0; i < 4; i++)
    {
      byte *block = (shape + (_rot * 4 + i) * 2);

      drawBlock(
          _board_left + block[0] * _block_size + _block_size * _x,
          _board_top + block[1] * _block_size + _block_size * _y,
          colors[current]);

      board[block[1] + _y][block[0] + _x] = 255;
    }

    // erase old
    for (int i = 0; i < 4; i++)
    {
      byte *block = (shape + (_old.rot * 4 + i) * 2);

      if (board[block[1] + _old.y][block[0] + _old.x] == 255)
        continue;

      _gfx->fillRect(
          _board_left + block[0] * _block_size + _block_size * _old.x,
          _board_top + block[1] * _block_size + _block_size * _old.y,
          _block_size,
          _block_size,
          BG_COLOR);
    }

    for (int i = 0; i < 4; i++)
    {
      byte *block = (shape + (_rot * 4 + i) * 2);
      board[block[1] + _y][block[0] + _x] = 0;
    }

    _old.x = _x;
    _old.y = _y;
    _old.rot = _rot;
  }

  bool touches(int xi, int yi, int roti)
  {
    byte *shape = all_shapes[current];
    for (int i = 0; i < 4; i++)
    {
      byte *block = (shape + (((_rot + roti) % shapes[current]) * 4 + i) * 2);

      int x2 = _x + block[0] + xi;
      int y2 = _y + block[1] + yi;

      if (y2 == BOARD_HEIGHT || x2 == BOARD_WIDTH || board[y2][x2] != 0)
      {
        return true;
      }
    }
    return false;
  }

  void score()
  {
    // we scan a max of 4 lines
    int ll;
    if (_y + 3 >= BOARD_HEIGHT)
    {
      ll = BOARD_HEIGHT - 1;
    }

    // scan board from current position
    for (int l = _y; l <= ll; l++)
    {
      // check if there's a complete line on the board
      bool line = true;

      for (int c = 0; c < BOARD_WIDTH; c++)
      {
        if (board[l][c] == 0)
        {
          line = false;
          break;
        }
      }

      if (!line)
      {
        // move to next line
        continue;
      }

      lines++;

      if (lines % 10 == 0)
      {
        level++;
        dropWait /= 2;
      }

      scoreBoard();

      // move board down
      for (int row = l; row > 0; row--)
      {
        for (int c = 0; c < BOARD_WIDTH; c++)
        {
          byte v = board[row - 1][c];

          board[row][c] = v;

          if (v == 0)
          {
            _gfx->fillRect(
                _board_left + _block_size * c,
                _board_top + _block_size * row,
                _block_size,
                _block_size,
                BG_COLOR);
          }
          else
          {
            drawBlock(
                _board_left + _block_size * c,
                _board_top + _block_size * row,
                colors[v - 1]);
          }
        }
      }

      // clear top line
      for (int c = 0; c < BOARD_WIDTH; c++)
      {
        board[0][c] = 0;
      }
    }

    delay(350);
  }

  void scoreBoard()
  {
    int16_t l = _score_board_left;
    int16_t t = _score_board_top;
    _gfx->drawRect(l - 1, t - 1, _score_board_w + 2, _score_board_h + 2, PIT_COLOR);
    _gfx->fillRect(l, t, _score_board_w, _score_board_h, BG_COLOR);
    l += _margin;
    t += _margin;
    _gfx->setCursor(l, t);
    _gfx->setTextColor(RGB565_YELLOW);
    _gfx->setTextSize(1);
    _gfx->print("Level");
    t += FONT_HEIGHT + _margin;
    _gfx->setCursor(l, t);
    _gfx->setTextColor(RGB565_WHITE);
    _gfx->setTextSize(2);
    _gfx->print(level);
    t += FONT_HEIGHT + FONT_HEIGHT + _margin;
    _gfx->setCursor(l, t);
    _gfx->setTextColor(RGB565_CYAN);
    _gfx->setTextSize(1);
    _gfx->print("Lines");
    t += FONT_HEIGHT + _margin;
    _gfx->setCursor(l, t);
    _gfx->setTextColor(RGB565_WHITE);
    _gfx->setTextSize(2);
    _gfx->print(lines);
  }

  // create a sequence of 7 random shapes

  void randomizer()
  {
    // randomize 7 shapes

    for (byte i = 0; i < 7; i++)
    {
      bool retry;
      byte shape;
      do
      {
        shape = random(7);

        // check if already in sequence

        retry = false;
        for (int j = 0; j < i; j++)
        {
          if (shape == next[j])
          {
            retry = true;
            break;
          }
        }
      } while (retry);
      next[i] = shape;
    }
    next_c = 0;
  }
};
