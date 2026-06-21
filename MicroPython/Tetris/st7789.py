import machine
import time
import ustruct

class ST7789:
    def __init__(self, spi, dc, rst, cs, width=240, height=240):
        self.spi = spi
        self.dc = dc
        self.rst = rst
        self.cs = cs
        self.width = width
        self.height = height
        self.reset()
        self.init()

    def write_cmd(self, cmd):
        self.dc.value(0)
        self.cs.value(0)
        self.spi.write(bytes([cmd]))
        self.cs.value(1)

    def write_data(self, data):
        self.dc.value(1)
        self.cs.value(0)
        self.spi.write(data)
        self.cs.value(1)

    def reset(self):
        self.rst.value(1)
        time.sleep_ms(5)
        self.rst.value(0)
        time.sleep_ms(15)
        self.rst.value(1)
        time.sleep_ms(15)

    def init(self):
        self.write_cmd(0x01) # SWRESET
        time.sleep_ms(150)
        self.write_cmd(0x11) # SLPOUT
        time.sleep_ms(150)
        self.write_cmd(0x3A) # COLMOD
        self.write_data(bytes([0x55])) 
        self.write_cmd(0x36) # MADCTL
        self.write_data(bytes([0x00])) 
        self.write_cmd(0x21) # INVON
        self.write_cmd(0x13) # NORON
        self.write_cmd(0x29) # DISPON
        time.sleep_ms(50)

    def set_window(self, x0, y0, x1, y1):
        self.write_cmd(0x2A) # CASET
        self.write_data(ustruct.pack(">HH", x0, x1))
        self.write_cmd(0x2B) # RASET
        self.write_data(ustruct.pack(">HH", y0, y1))
        self.write_cmd(0x2C) # RAMWR

    def fill_rect(self, x, y, w, h, color):
        if (x >= self.width) or (y >= self.height): return
        if (x + w - 1) >= self.width: w = self.width - x
        if (y + h - 1) >= self.height: h = self.height - y
        self.set_window(x, y, x + w - 1, y + h - 1)
        pixel_count = w * h
        buf = ustruct.pack(">H", color) * min(pixel_count, 512)
        self.dc.value(1)
        self.cs.value(0)
        for _ in range(pixel_count // 512):
            self.spi.write(buf)
        if pixel_count % 512:
            self.spi.write(buf[:(pixel_count % 512) * 2])
        self.cs.value(1)

    def fill(self, color):
        self.fill_rect(0, 0, self.width, self.height, color)
