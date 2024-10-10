#pragma once

char months[12][10] = {
    "January",
    "February",
    "March",
    "April",
    "May",
    "June",
    "July",
    "August",
    "September",
    "October",
    "November",
    "December"};

char dow[12][10] = {
    "Sunday",
    "Monday",
    "Tuesday",
    "Wednesday",
    "Thursday",
    "Friday",
    "Saturday"};

void print_digit(int value)
{
  if (value < 10)
  {
    oled.write('0');
  }
  oled.print(value);
}

void draw_time()
{
  // oled.fill(0x00);

  // 1st row: print date
  bool b;
  oled.set_font_size(1);
  oled.set_pos(0, 0);
  oled.print("20");
  print_digit(dsc3231.getYear());
  oled.print(' ');
  oled.print(months[dsc3231.getMonth(b) - 1]);
  oled.print(' ');
  oled.print(dsc3231.getDate());
  oled.print(' ');
  oled.print(dow[dsc3231.getDoW() - 1]);

  // 2nd-4th rows: print time
  oled.set_font_size(3);
  oled.set_pos(0, 1);
  print_digit(dsc3231.getHour(b, b));
  oled.draw_pattern(0 + (2 * FONT_3X_WIDTH) + 3, 1, 3, 2, 0b11100000);
  oled.set_pos(0 + (2 * FONT_3X_WIDTH) + 9, 1);
  print_digit(dsc3231.getMinute());
  oled.draw_pattern(0 + (4 * FONT_3X_WIDTH) + 9 + 3, 1, 3, 2, 0b11100000);
  oled.set_pos(0 + (4 * FONT_3X_WIDTH) + (2 * 9), 1);
  print_digit(dsc3231.getSecond());
}
