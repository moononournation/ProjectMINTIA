import time
import random
import board
import busio
import digitalio
import displayio
from adafruit_st7789 import ST7789
from adafruit_display_text import label
from fourwire import FourWire
from terminalio import FONT

# --- DISPLAY SETUP ---
displayio.release_displays()

spi = busio.SPI(clock=board.D4, MOSI=board.D5)
display_bus = FourWire(spi, command=board.D18, chip_select=board.D3, reset=board.D9)

# 240x240 ST7789 display configuration
display = ST7789(display_bus, rotation=180, width=240, height=240, rowstart=80, colstart=0, backlight_pin=board.D16)
display.auto_refresh=False

# Main display group
main_group = displayio.Group()
display.root_group = main_group

# --- GAME CONSTANTS ---
GRID_COLS = 10
GRID_ROWS = 20
BLOCK_SIZE = 11  # Pixels per block
BOARD_X = 10     # X offset on screen
BOARD_Y = 10     # Y offset on screen

# Colors (16-bit hex)
BLACK = 0x0000
WHITE = 0xFFFF
GRAY = 0x52AA
COLORS = [
    0x00FFFF,  # I - Cyan
    0xFFFF00,  # O - Yellow
    0x800080,  # T - Purple
    0x00FF00,  # S - Green
    0xFF0000,  # Z - Red
    0x0000FF,  # J - Blue
    0xFFA500,  # L - Orange
]

# SRS standard orientation matrix
TETROMINOES = {
    'I': [[(0,1), (1,1), (2,1), (3,1)], [(2,0), (2,1), (2,2), (2,3)]],
    'O': [[(1,0), (2,0), (1,1), (2,1)]],
    'T': [[(1,0), (0,1), (1,1), (2,1)], [(1,0), (1,1), (2,1), (1,2)], [(0,1), (1,1), (2,1), (1,2)], [(1,0), (0,1), (1,1), (1,2)]],
    'L': [[(2,0), (0,1), (1,1), (2,1)], [(1,0), (1,1), (1,2), (2,2)], [(0,1), (1,1), (2,1), (0,2)], [(0,0), (1,0), (1,1), (1,2)]],
    'J': [[(0,0), (0,1), (1,1), (2,1)], [(1,0), (2,0), (1,1), (1,2)], [(0,1), (1,1), (2,1), (2,2)], [(1,0), (1,1), (0,2), (1,2)]],
    'S': [[(1,0), (2,0), (0,1), (1,1)], [(1,0), (1,1), (2,1), (2,2)]],
    'Z': [[(0,0), (1,0), (1,1), (2,1)], [(2,0), (1,1), (2,1), (1,2)]]
}
SHAPE_KEYS = list(TETROMINOES.keys())

# --- BUTTON SETUP ---
def make_button(pin):
    btn = digitalio.DigitalInOut(pin)
    btn.direction = digitalio.Direction.INPUT
    btn.pull = digitalio.Pull.UP
    return btn

btn_left = make_button(board.D8)
btn_right = make_button(board.D10)
btn_rotate = make_button(board.D11)
btn_drop = make_button(board.D7)

# --- GRAPHICS CANVAS ---
palette = displayio.Palette(10)
palette[0] = BLACK
for i, color in enumerate(COLORS):
    palette[i+1] = color
palette[8] = GRAY
palette[9] = WHITE

# Total layout bitmap canvas to handle main board and side preview frame
bitmap = displayio.Bitmap(240, 240, 10)
tile_grid = displayio.TileGrid(bitmap, pixel_shader=palette, x=0, y=0)
main_group.append(tile_grid)

# UI Text Labels
score_label = label.Label(FONT, text="SCORE: 0", color=WHITE, x=135, y=20)
level_label = label.Label(FONT, text="LEVEL: 1", color=WHITE, x=135, y=40)
lines_label = label.Label(FONT, text="LINES: 0", color=WHITE, x=135, y=60)
next_label = label.Label(FONT, text="NEXT:", color=WHITE, x=135, y=95)
game_over_label = label.Label(FONT, text="", color=0xFF0000, x=135, y=170)

main_group.append(score_label)
main_group.append(level_label)
main_group.append(lines_label)
main_group.append(next_label)
main_group.append(game_over_label)

# --- GAME STATE ---
board_matrix = [[0] * GRID_COLS for _ in range(GRID_ROWS)]
score = 0
level = 1
lines_cleared = 0
fall_delay = 0.5

current_piece = None
piece_color_idx = 0
piece_x = 0
piece_y = 0
piece_rot = 0

next_piece = random.choice(SHAPE_KEYS)  # Seed the first lookahead piece

# --- HELPER FUNCTIONS ---
def draw_borders():
    # Main Board Container Frame
    w = GRID_COLS * BLOCK_SIZE + 1
    h = GRID_ROWS * BLOCK_SIZE + 1
    for x in range(BOARD_X, BOARD_X + w + 1):
        bitmap[x, BOARD_Y] = 8
        bitmap[x, BOARD_Y + h] = 8
    for y in range(BOARD_Y, BOARD_Y + h + 1):
        bitmap[BOARD_X, y] = 8
        bitmap[BOARD_X + w, y] = 8

    # Next Piece Box Frame (4x4 block window)
    nx, ny = 135, 105
    nw, nh = 4 * BLOCK_SIZE + 2, 4 * BLOCK_SIZE + 2
    for x in range(nx, nx + nw):
        bitmap[x, ny] = 8
        bitmap[x, ny + nh - 1] = 8
    for y in range(ny, ny + nh):
        bitmap[nx, y] = 8
        bitmap[nx + nw - 1, y] = 8

def draw_block(bx, by, color_idx, is_preview=False):
    if is_preview:
        # Base coordinates inside the "Next" preview window
        px = 136 + bx * BLOCK_SIZE
        py = 106 + by * BLOCK_SIZE
    else:
        # Base coordinates inside the main board matrix
        px = BOARD_X + 1 + bx * BLOCK_SIZE
        py = BOARD_Y + 1 + by * BLOCK_SIZE
        
    for x in range(px, px + BLOCK_SIZE - 1):
        for y in range(py, py + BLOCK_SIZE - 1):
            bitmap[x, y] = color_idx

def render_board():
    # 1. Redraw active main board playing matrix
    for y in range(GRID_ROWS):
        for x in range(GRID_COLS):
            draw_block(x, y, board_matrix[y][x])
            
    # 2. Render falling brick
    if current_piece:
        for block in TETROMINOES[current_piece][piece_rot]:
            bx = piece_x + block[0]
            by = piece_y + block[1]
            if 0 <= by < GRID_ROWS and 0 <= bx < GRID_COLS:
                draw_block(bx, by, piece_color_idx)

    # 3. Render side preview piece box
    # Clear the 4x4 internal preview grid area first
    for y in range(4):
        for x in range(4):
            draw_block(x, y, 0, is_preview=True)
            
    # Draw next block layout shifted toward center of the frame
    np_color_idx = SHAPE_KEYS.index(next_piece) + 1
    for block in TETROMINOES[next_piece][0]:
        draw_block(block[0], block[1], np_color_idx, is_preview=True)

def spawn_piece():
    global current_piece, piece_color_idx, piece_x, piece_y, piece_rot, next_piece
    
    # Take the piece that was previously in the preview window
    current_piece = next_piece
    piece_color_idx = SHAPE_KEYS.index(current_piece) + 1
    piece_rot = 0
    piece_x = GRID_COLS // 2 - 2
    piece_y = 0
    
    # Roll a new piece to queue up for next time
    next_piece = random.choice(SHAPE_KEYS)
    
    if not check_collision(piece_x, piece_y, piece_rot):
        return False
    return True

def check_collision(nx, ny, nr):
    for block in TETROMINOES[current_piece][nr % len(TETROMINOES[current_piece])]:
        bx = nx + block[0]
        by = ny + block[1]
        if bx < 0 or bx >= GRID_COLS or by >= GRID_ROWS:
            return False
        if by >= 0 and board_matrix[by][bx] != 0:
            return False
    return True

def lock_piece():
    global score, lines_cleared, level, fall_delay
    for block in TETROMINOES[current_piece][piece_rot]:
        bx = piece_x + block[0]
        by = piece_y + block[1]
        if 0 <= by < GRID_ROWS and 0 <= bx < GRID_COLS:
            board_matrix[by][bx] = piece_color_idx
            
    cleared = 0
    y = GRID_ROWS - 1
    while y >= 0:
        if all(board_matrix[y]):
            del board_matrix[y]
            board_matrix.insert(0, [0] * GRID_COLS)
            cleared += 1
        else:
            y -= 1
            
    if cleared > 0:
        lines_cleared += cleared
        scoring_system = [0, 100, 300, 500, 800]
        score += scoring_system[cleared] * level
        level = (lines_cleared // 10) + 1
        fall_delay = max(0.05, 0.5 - (level * 0.04))
        
        score_label.text = f"SCORE: {score}"
        level_label.text = f"LEVEL: {level}"
        lines_label.text = f"LINES: {lines_cleared}"

# --- MAIN LOOP ---
draw_borders()
spawn_piece()
last_fall_time = time.monotonic()

game_running = True

while game_running:
    current_time = time.monotonic()
    action_taken = False

    if not btn_left.value:
        if check_collision(piece_x - 1, piece_y, piece_rot):
            piece_x -= 1
            action_taken = True
        time.sleep(0.12)
        
    if not btn_right.value:
        if check_collision(piece_x + 1, piece_y, piece_rot):
            piece_x += 1
            action_taken = True
        time.sleep(0.12)
        
    if not btn_rotate.value:
        next_rot = (piece_rot + 1) % len(TETROMINOES[current_piece])
        if check_collision(piece_x, piece_y, next_rot):
            piece_rot = next_rot
            action_taken = True
        time.sleep(0.15)
        
    if not btn_drop.value:
        if check_collision(piece_x, piece_y + 1, piece_rot):
            piece_y += 1
            action_taken = True
        time.sleep(0.05)

    if current_time - last_fall_time > fall_delay:
        if check_collision(piece_x, piece_y + 1, piece_rot):
            piece_y += 1
        else:
            lock_piece()
            if not spawn_piece():
                game_over_label.text = "GAME OVER"
                game_running = False
        last_fall_time = current_time
        action_taken = True

    if action_taken or not game_running:
        render_board()
        display.refresh()

while True:
    pass
