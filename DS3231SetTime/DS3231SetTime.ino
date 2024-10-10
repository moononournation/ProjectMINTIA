#include <DS3231.h>
DS3231 dsc3231;

#include "CompilationTime.h"

void setup()
{
  Wire.begin();
  dsc3231.setEpoch(UNIX_TIMESTAMP, false);
}

void loop()
{
}
