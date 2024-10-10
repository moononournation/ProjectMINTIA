# Project MINTIA

## Entry Level

### DS3231 Set Time

[Compilation Time Source Code](https://github.com/jerabaul29/Compile_time_Cpp_UNIX_timestamp)

Simply use compilation time set to the DS3231 RTC.

Please do not reset after upload until upload another program.

### ATtiny85 Watch

Check available font list

```shell
magick -list font
```

Generation Preview

```shell
magick -depth 1 -font Courier-New -pointsize 20 label:"_0123456789_" -crop 120x16+12+4 font_2x.png

magick -depth 1 -font SpotMono-Bold -pointsize 30 label:"_0123456789_" -crop 180x24+18+13 font_3x.png
```

Generation Header Code

```shell
magick -depth 1 -font Courier-New -pointsize 20 label:"_0123456789_" -crop 120x16+12+4 -flip -rotate 90 font_2x.xbm

magick -depth 1 -font SpotMono-Bold -pointsize 30 label:"_0123456789_" -crop 180x24+18+13 -flip -rotate 90 font_3x.xbm
```
