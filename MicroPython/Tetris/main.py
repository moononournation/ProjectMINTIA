import machine
import time
import random
from st7789 import ST7789

# --- 1. Hardware Connection Configuration (Seeed Studio XIAO RP2350) ---
# Backlight Pin Control
blk_pin = machine.Pin(12, machine.Pin.OUT)
blk_pin.value(1)  # Turn on the screen backlight

# High-Speed Hardware SPI Display Pipeline Configuration
spi0 = machine.SPI(0, baudrate=31250000, polarity=1, phase=1, sck=machine.Pin(6), mosi=machine.Pin(7))
dc_pin = machine.Pin(9, machine.Pin.OUT)
rst_pin = machine.Pin(4, machine.Pin.OUT)
cs_pin = machine.Pin(5, machine.Pin.OUT)
display = ST7789(spi0, dc_pin, rst_pin, cs_pin, width=240, height=240)

# Physical Push Buttons (Internal pull-ups enabled; active-low logic)
pin_left   = machine.Pin(2, machine.Pin.IN, machine.Pin.PULL_UP)
pin_right  = machine.Pin(3, machine.Pin.IN, machine.Pin.PULL_UP)
pin_rotate = machine.Pin(21, machine.Pin.IN, machine.Pin.PULL_UP)
pin_drop   = machine.Pin(1, machine.Pin.IN, machine.Pin.PULL_UP)

# --- 2. Game Metrics & Interface Layout Settings ---
BOARD_WIDTH = 10
BOARD_HEIGHT = 20
BLOCK_SIZE = 11  
OFFSET_X = 85      # Shifted right to preserve room on the left for the HUD Panel
OFFSET_Y = 10

NORMAL_TICK_MS = 400  # Default falling interval under normal conditions
FAST_TICK_MS = 40     # Ultra-fast acceleration drop interval when pin_drop is pressed

# RGB565 16-bit Color Table Definition
BLACK   = 0x0000
GRAY    = 0x5AAB
WHITE   = 0xFFFF
RED     = 0xF800
GREEN   = 0x07E0
BLUE    = 0x001F
CYAN    = 0x07FF
YELLOW  = 0xFFE0
MAGENTA = 0xF81F
ORANGE  = 0xFD20

# Minimal built-in 8x5 bitmapped characters (0-9, S, N)
# Each character comprises 5 sequential vertical byte columns
FONT_NUMBERS = {
    '0': b'\x3e\x41\x41\x41\x3e',
    '1': b'\x00\x42\x7f\x40\x00',
    '2': b'\x42\x61\x51\x49\x46',
    '3': b'\x21\x41\x45\x4b\x31',
    '4': b'\x18\x14\x12\x7f\x10',
    '5': b'\x27\x45\x45\x45\x39',
    '6': b'\x3c\x4a\x49\x49\x30',
    '7': b'\x01\x71\x09\x05\x03',
    '8': b'\x36\x49\x49\x49\x36',
    '9': b'\x06\x49\x49\x29\x1e',
    'S': b'\x26\x49\x49\x49\x32', # S letter label for Score
    'N': b'\x7f\x02\x0c\x10\x7f', # N letter label for Next
}

# FIXED: Explicitly defined 2D coordinates [X, Y] per shape to eliminate compilation errors
SHAPES_LIST = [
    [[0,1], [1,1], [2,1], [3,1]], # 1: I Shape
    [[1,1], [2,1], [1,2], [2,2]], # 2: O Shape
    [[1,0], [0,1], [1,1], [2,1]], # 3: T Shape
    [[1,0], [2,0], [0,1], [1,1]], # 4: S Shape
    [[0,0], [1,0], [1,1], [2,1]], # 5: Z Shape
    [[0,0], [0,1], [1,1], [2,1]], # 6: J Shape
    [[2,0], [0,1], [1,1], [2,1]]  # 7: L Shape
]

COLOR_MAP = {
    0: BLACK, 1: CYAN, 2: YELLOW, 3: MAGENTA,
    4: GREEN, 5: RED, 6: BLUE, 7: ORANGE
}

class Tetris:
    def __init__(self):
        self.width = BOARD_WIDTH
        self.height = BOARD_HEIGHT
        self.grid = [[0 for _ in range(self.width)] for _ in range(self.height)]
        self.score = 0
        self.game_over = False
        
        self.current_piece = [] # Coordinates array holding the 4 active vertex nodes
        self.current_id = 0     # Active piece identifier integer (1 to 7)
        self.current_x = 0      # Local anchoring horizontal position map offset
        self.current_y = 0      # Local anchoring vertical position map offset
        
        # Pre-generate subsequent queue data ahead of spawning loops
        self.next_id = random.randint(1, 7)
        
        display.fill(BLACK)
        self.draw_borders(GRAY)
        self.draw_score()
        self.spawn_piece() # Triggers initialization drawing inside the next panel
        self.draw_render()

    def draw_borders(self, color):
        # Master Game Matrix Bounding Box
        bx = OFFSET_X - 2
        by = OFFSET_Y - 2
        bw = (self.width * BLOCK_SIZE) + 4
        bh = (self.height * BLOCK_SIZE) + 4
        display.fill_rect(bx, by, bw, 2, color)
        display.fill_rect(bx, by, 2, bh, color)
        display.fill_rect(bx + bw - 2, by, 2, bh, color)
        display.fill_rect(bx, by + bh - 2, bw, 2, color)
        
        # Next Piece Preview Compartment Outer Box (Located at X:15-65, Y:90-140)
        display.fill_rect(15, 90, 50, 2, color)   # Top border
        display.fill_rect(15, 90, 2, 50, color)   # Left border
        display.fill_rect(63, 90, 2, 50, color)   # Right border
        display.fill_rect(15, 138, 50, 2, color)  # Bottom border

    def draw_char(self, char, x, y, color, size=2):
        """ Draws a custom scaled bitmapped character node onto specified coordinate regions """
        if char not in FONT_NUMBERS: return
        bitmap = FONT_NUMBERS[char]
        for col_idx, col in enumerate(bitmap):
            for row_idx in range(8):
                if (col >> row_idx) & 1:
                    display.fill_rect(x + col_idx * size, y + row_idx * size, size, size, color)

    def draw_score(self):
        """ Refreshes numerical score readings inside HUD panel area (X: 10-70) """
        display.fill_rect(10, 10, 70, 60, BLACK) # Wipe out historical score traces
        
        # Render out HUD "S:" label indicator
        self.draw_char('S', 20, 15, WHITE, size=2)
        display.fill_rect(35, 23, 3, 3, WHITE) # Top colon dot node
        display.fill_rect(35, 29, 3, 3, WHITE) # Bottom colon dot node
        
        # Print actual integer numerical strings left-to-right
        score_str = str(self.score)
        start_x = 20
        start_y = 40
        for idx, num_char in enumerate(score_str):
            # Column pitch sizing calculations: 5 * size(2) + 4px kerning gap = 14px step width
            self.draw_char(num_char, start_x + (idx * 14), start_y, YELLOW, size=2)

    def draw_next_preview(self):
        """ Re-draws piece layouts centered dynamically inside preview frame blocks """
        display.fill_rect(17, 92, 45, 45, BLACK) # Clean slate preview panel buffer area
        
        # Render small HUD "N:" hint title
        self.draw_char('N', 20, 72, WHITE, size=1)
        display.fill_rect(30, 75, 2, 2, WHITE)
        display.fill_rect(30, 79, 2, 2, WHITE)
        
        # Load up configuration geometry template array shapes matching incoming IDs
        template = SHAPES_LIST[self.next_id - 1]
        color = COLOR_MAP[self.next_id]
        
        # Base placement centering reference origins inside 50x50 perimeter boundaries
        p_offset_x = 22
        p_offset_y = 102
        
        # Edge-case adjustment parameters optimizing centering layouts on specific models
        if self.next_id == 1: # Line I Shape
            p_offset_x = 17
            p_offset_y = 102
        elif self.next_id == 2: # Cube O Shape
            p_offset_x = 22
            p_offset_y = 97

        for pt in template:
            # FIXED: Correct sub-index unpacking from 2D inner layout blocks
            px = p_offset_x + (pt[0] * BLOCK_SIZE)
            py = p_offset_y + (pt[1] * BLOCK_SIZE)
            display.fill_rect(px, py, BLOCK_SIZE - 1, BLOCK_SIZE - 1, color)

    def spawn_piece(self):
        # 1. Inherit historical data stored previously under preview frames
        self.current_id = self.next_id
        template = SHAPES_LIST[self.current_id - 1]
        
        # FIXED: Execute clean inner-array duplications preventing shallow-reference bugs
        self.current_piece = [[pt[0], pt[1]] for pt in template]
        self.current_x = self.width // 2 - 2
        self.current_y = -1
        
        # 2. Cycle subsequent random integers planning future generations
        self.next_id = random.randint(1, 7)
        
        # 3. Synchronize screen display pipelines refreshing preview arrays
        self.draw_next_preview()
        
        # Collision validation checking initialization points
        if self.check_collision(self.current_x, self.current_y, self.current_piece):
            self.game_over = True

    def check_collision(self, nx, ny, piece):
        for pt in piece:
            # FIXED: Unpacking array coordinates cleanly to target correct integers
            gx = nx + pt[0]
            gy = ny + pt[1]
            if gy < 0:
                if gx < 0 or gx >= self.width:
                    return True
                continue
            if gx < 0 or gx >= self.width or gy >= self.height:
                return True
            if self.grid[gy][gx] != 0:
                return True
        return False

    def rotate_piece(self):
        if self.current_id == 2: # Bypassing O-shapes since rotations look completely uniform
            return
        rotated = []
        for pt in self.current_piece:
            # Traditional 2D matrix anti-clockwise rotational mapping layout formulas
            # FIXED: Explicitly target structural dimension array mappings [0] and [1]
            rotated.append([2 - pt[1], pt[0]])
        if not self.check_collision(self.current_x, self.current_y, rotated):
            self.current_piece = rotated

    def lock_piece(self):
        for pt in self.current_piece:
            gx = self.current_x + pt[0]
            gy = self.current_y + pt[1]
            if gy >= 0:
                self.grid[gy][gx] = self.current_id
        self.clear_lines()
        self.spawn_piece()

    def clear_lines(self):
        new_grid = [row for row in self.grid if any(cell == 0 for cell in row)]
        cleared = self.height - len(new_grid)
        if cleared > 0:
            self.score += (cleared * 100)
            self.draw_score() # Update numerical points instantly upon clearing line configurations
            for _ in range(cleared):
                new_grid.insert(0, [0 for _ in range(self.width)])
            self.grid = new_grid

    def move(self, dx, dy):
        if not self.check_collision(self.current_x + dx, self.current_y + dy, self.current_piece):
            self.current_x += dx
            self.current_y += dy
            return True
        if dy > 0:
            self.lock_piece()
            return False
        return False

    def draw_render(self):
        # Pull reference from grid static structure arrays
        display_buf = [[self.grid[y][x] for x in range(self.width)] for y in range(self.height)]

        # Inject floating active components before pushing out rendering maps
        if not self.game_over:
            for pt in self.current_piece:
                # FIXED: Access indices pt[0] and pt[1] correctly
                gx = self.current_x + pt[0]
                gy = self.current_y + pt[1]
                if 0 <= gy < self.height and 0 <= gx < self.width:
                    display_buf[gy][gx] = self.current_id

        # Write buffered frame logic out sequentially onto ST7789 TFT screen blocks
        for y in range(self.height):
            for x in range(self.width):
                cell_val = display_buf[y][x]
                color = COLOR_MAP[cell_val]
                screen_x = OFFSET_X + (x * BLOCK_SIZE)
                screen_y = OFFSET_Y + (y * BLOCK_SIZE)
                display.fill_rect(screen_x, screen_y, BLOCK_SIZE - 1, BLOCK_SIZE - 1, color)

def main():
    game = Tetris()
    last_tick = time.ticks_ms()

    last_btn_time = 0
    DEBOUNCE_MS = 130 # Timing filter isolating redundant physical pin vibration traces
    while not game.game_over:
        current_time = time.ticks_ms()
        action_taken = False

        # 1. Dynamic Drop Evaluation: Read state on pin_drop, toggle tick frequencies
        if pin_drop.value() == 0:
            current_tick_interval = FAST_TICK_MS
        else:
            current_tick_interval = NORMAL_TICK_MS

        # 2. Process secondary shift action controls under debounce limits
        if current_time - last_btn_time > DEBOUNCE_MS:
            if pin_left.value() == 0:
                game.move(-1, 0)
                action_taken = True
                last_btn_time = current_time
            elif pin_right.value() == 0:
                game.move(1, 0)
                action_taken = True
                last_btn_time = current_time
            elif pin_rotate.value() == 0:
                game.rotate_piece()
                action_taken = True
                last_btn_time = current_time

        if action_taken:
            game.draw_render()

        # 3. Master Engine Clock Loop calculations driving core block gravity
        if time.ticks_diff(time.ticks_ms(), last_tick) > current_tick_interval:
            game.move(0, 1)
            game.draw_render()
            last_tick = time.ticks_ms()

        time.sleep(0.01)

    # Game Over Flash Animation sequence loop loops
    for _ in range(3):
        game.draw_borders(RED)
        time.sleep(0.2)
        game.draw_borders(GRAY)
        time.sleep(0.2)

    print(f"Game Over! Final Score: {game.score}")

if __name__ == '__main__':
    main()
