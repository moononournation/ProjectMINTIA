"""
MINTIA Console
"""

from machine import Pin

class Buttons():
    """
    Buttons class for examples, modify for your device.
    """

    def __init__(self):
        self.name = "mintia_pico_console"
        self.uo = Pin(2, Pin.IN, Pin.PULL_UP) # Joystick up
        self.down = Pin(3, Pin.IN, Pin.PULL_UP) # Joystick down
        self.left = Pin(2, Pin.IN, Pin.PULL_UP) # Joystick left
        self.right = Pin(3, Pin.IN, Pin.PULL_UP) # Joystick right
        self.a = Pin(21, Pin.IN, Pin.PULL_UP)
        self.b = Pin(20, Pin.IN, Pin.PULL_UP)
        self.select = Pin(16, Pin.IN, Pin.PULL_UP)
        self.start = Pin(17, Pin.IN, Pin.PULL_UP)
