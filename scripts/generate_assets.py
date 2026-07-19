#!/usr/bin/env python3
"""
Skyline Sprint — High-quality original pixel-art asset generator.
Produces crisp neon-cyberpunk sprites that match the industrial skyline theme.
All assets are original geometric designs (no commercial IP).
"""

import struct
import math
import os

def save_tga(path, width, height, pixels):
    """Save uncompressed 32-bit BGRA TGA (top-to-bottom)."""
    os.makedirs(os.path.dirname(path), exist_ok=True)
    header = bytearray(18)
    header[2] = 2  # uncompressed true-color
    header[12] = width & 0xFF
    header[13] = (width >> 8) & 0xFF
    header[14] = height & 0xFF
    header[15] = (height >> 8) & 0xFF
    header[16] = 32  # 32 bits per pixel
    header[17] = 8 | 32  # 8 bits alpha, top-to-bottom

    data = bytearray()
    for r, g, b, a in pixels:
        data.append(b)
        data.append(g)
        data.append(r)
        data.append(a)

    with open(path, "wb") as f:
        f.write(header)
        f.write(data)
    print(f"  wrote {path} ({width}x{height})")


def save_wav(path, sample_rate, samples):
    os.makedirs(os.path.dirname(path), exist_ok=True)
    pcm_data = bytearray()
    for s in samples:
        s = max(-1.0, min(1.0, s))
        val = int(s * 32767)
        pcm_data.extend(struct.pack("<h", val))

    data_size = len(samples) * 2
    header = struct.pack(
        "<4sI4s4sIHHIIHH4sI",
        b'RIFF',
        36 + data_size,
        b'WAVE',
        b'fmt ',
        16,
        1,  # PCM
        1,  # Mono
        sample_rate,
        sample_rate * 2,
        2,  # Block align
        16,  # Bits per sample
        b'data',
        data_size
    )
    with open(path, "wb") as f:
        f.write(header)
        f.write(pcm_data)
    print(f"  wrote {path}")


# ---------------------------------------------------------------------------
# Palette (matches ui_ux_design.md)
# ---------------------------------------------------------------------------
C = {
    "deep":     (17, 24, 39),
    "far":      (31, 41, 55),
    "near":     (55, 65, 81),
    "body":     (51, 65, 85),
    "edge":     (148, 163, 184),
    "cyan":     (34, 211, 238),
    "cyan_dim": (14, 116, 144),
    "cyan_glow":(103, 232, 249),
    "purple":   (167, 139, 250),
    "purple_d": (109, 40, 217),
    "yellow":   (250, 204, 21),
    "yellow_g": (253, 224, 71),
    "orange":   (251, 146, 60),
    "red":      (251, 113, 133),
    "green":    (74, 222, 128),
    "white":    (248, 250, 252),
    "leg":      (148, 163, 184),
    "dark":     (15, 23, 42),
    "metal":    (71, 85, 105),
    "panel":    (30, 41, 59),
}


def px(pixels, w, x, y, color, a=255):
    """Safe pixel set."""
    if 0 <= x < w and 0 <= y < len(pixels) // w:
        idx = y * w + x
        pixels[idx] = (color[0], color[1], color[2], a)


def fill_rect(pixels, w, x0, y0, x1, y1, color, a=255):
    for y in range(y0, y1 + 1):
        for x in range(x0, x1 + 1):
            px(pixels, w, x, y, color, a)


def fill_circle(pixels, w, cx, cy, r, color, a=255):
    r2 = r * r
    for y in range(cy - r, cy + r + 1):
        for x in range(cx - r, cx + r + 1):
            if (x - cx) ** 2 + (y - cy) ** 2 <= r2:
                px(pixels, w, x, y, color, a)


def fill_ellipse(pixels, w, cx, cy, rx, ry, color, a=255):
    for y in range(cy - ry, cy + ry + 1):
        for x in range(cx - rx, cx + rx + 1):
            if ry == 0 or rx == 0:
                continue
            if ((x - cx) / rx) ** 2 + ((y - cy) / ry) ** 2 <= 1.0:
                px(pixels, w, x, y, color, a)


def hline(pixels, w, x0, x1, y, color, a=255):
    for x in range(x0, x1 + 1):
        px(pixels, w, x, y, color, a)


def vline(pixels, w, x, y0, y1, color, a=255):
    for y in range(y0, y1 + 1):
        px(pixels, w, x, y, color, a)


# ---------------------------------------------------------------------------
# Font (8x8 bitmap, ASCII 32-127)
# ---------------------------------------------------------------------------
font_glyphs = {
    ' ': [0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00],
    '!': [0x18,0x18,0x18,0x18,0x00,0x00,0x18,0x00],
    '"': [0x24,0x24,0x24,0x00,0x00,0x00,0x00,0x00],
    '#': [0x24,0x24,0x7E,0x24,0x7E,0x24,0x24,0x00],
    '$': [0x1C,0x3E,0x1C,0x0E,0x3E,0x1C,0x08,0x00],
    '%': [0x42,0x44,0x08,0x10,0x22,0x42,0x00,0x00],
    '&': [0x1C,0x22,0x18,0x24,0x22,0x1D,0x00,0x00],
    "'": [0x18,0x18,0x08,0x00,0x00,0x00,0x00,0x00],
    '(': [0x0C,0x10,0x20,0x20,0x20,0x10,0x0C,0x00],
    ')': [0x30,0x08,0x04,0x04,0x04,0x08,0x30,0x00],
    '*': [0x00,0x24,0x18,0x3C,0x18,0x24,0x00,0x00],
    '+': [0x00,0x08,0x08,0x3E,0x08,0x08,0x00,0x00],
    ',': [0x00,0x00,0x00,0x00,0x18,0x18,0x08,0x10],
    '-': [0x00,0x00,0x00,0x3E,0x00,0x00,0x00,0x00],
    '.': [0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x00],
    '/': [0x02,0x04,0x08,0x10,0x20,0x40,0x00,0x00],
    '0': [0x3E,0x42,0x46,0x4E,0x52,0x62,0x3E,0x00],
    '1': [0x18,0x28,0x08,0x08,0x08,0x08,0x3E,0x00],
    '2': [0x3E,0x42,0x02,0x3E,0x40,0x40,0x7F,0x00],
    '3': [0x3E,0x42,0x02,0x1E,0x02,0x42,0x3E,0x00],
    '4': [0x08,0x18,0x28,0x48,0x7F,0x08,0x08,0x00],
    '5': [0x7F,0x40,0x7E,0x02,0x02,0x42,0x3E,0x00],
    '6': [0x3E,0x40,0x40,0x7E,0x42,0x42,0x3E,0x00],
    '7': [0x7F,0x02,0x04,0x08,0x10,0x20,0x40,0x00],
    '8': [0x3E,0x42,0x42,0x3E,0x42,0x42,0x3E,0x00],
    '9': [0x3E,0x42,0x42,0x3F,0x02,0x02,0x3E,0x00],
    ':': [0x00,0x18,0x18,0x00,0x18,0x18,0x00,0x00],
    ';': [0x00,0x18,0x18,0x00,0x18,0x18,0x08,0x10],
    '<': [0x04,0x08,0x10,0x20,0x10,0x08,0x04,0x00],
    '=': [0x00,0x00,0x3E,0x00,0x3E,0x00,0x00,0x00],
    '>': [0x20,0x10,0x08,0x04,0x08,0x10,0x20,0x00],
    '?': [0x3E,0x42,0x02,0x0C,0x18,0x00,0x18,0x00],
    '@': [0x3C,0x42,0x5E,0x52,0x52,0x40,0x3E,0x00],
    'A': [0x18,0x24,0x42,0x42,0x7E,0x42,0x42,0x00],
    'B': [0x7C,0x22,0x22,0x3C,0x22,0x22,0x7C,0x00],
    'C': [0x3C,0x42,0x40,0x40,0x40,0x42,0x3C,0x00],
    'D': [0x78,0x24,0x22,0x22,0x22,0x24,0x78,0x00],
    'E': [0x7E,0x40,0x40,0x7C,0x40,0x40,0x7E,0x00],
    'F': [0x7E,0x40,0x40,0x7C,0x40,0x40,0x40,0x00],
    'G': [0x3C,0x42,0x40,0x4E,0x42,0x42,0x3C,0x00],
    'H': [0x42,0x42,0x42,0x7E,0x42,0x42,0x42,0x00],
    'I': [0x3E,0x08,0x08,0x08,0x08,0x08,0x3E,0x00],
    'J': [0x0F,0x02,0x02,0x02,0x02,0x42,0x3C,0x00],
    'K': [0x44,0x48,0x50,0x60,0x50,0x48,0x44,0x00],
    'L': [0x40,0x40,0x40,0x40,0x40,0x40,0x7E,0x00],
    'M': [0x42,0x66,0x5A,0x42,0x42,0x42,0x42,0x00],
    'N': [0x42,0x62,0x52,0x4A,0x46,0x42,0x42,0x00],
    'O': [0x3C,0x42,0x42,0x42,0x42,0x42,0x3C,0x00],
    'P': [0x7C,0x42,0x42,0x7C,0x40,0x40,0x40,0x00],
    'Q': [0x3C,0x42,0x42,0x42,0x4A,0x44,0x3A,0x00],
    'R': [0x7C,0x42,0x42,0x7C,0x48,0x44,0x42,0x00],
    'S': [0x3E,0x40,0x40,0x3E,0x02,0x02,0x3E,0x00],
    'T': [0x7F,0x08,0x08,0x08,0x08,0x08,0x08,0x00],
    'U': [0x42,0x42,0x42,0x42,0x42,0x42,0x3C,0x00],
    'V': [0x42,0x42,0x42,0x42,0x24,0x24,0x18,0x00],
    'W': [0x42,0x42,0x42,0x42,0x4A,0x5A,0x24,0x00],
    'X': [0x42,0x24,0x18,0x18,0x24,0x42,0x42,0x00],
    'Y': [0x42,0x24,0x18,0x08,0x08,0x08,0x08,0x00],
    'Z': [0x7F,0x02,0x04,0x08,0x10,0x20,0x7F,0x00],
    '[': [0x3E,0x20,0x20,0x20,0x20,0x20,0x3E,0x00],
    '\\': [0x40,0x20,0x10,0x08,0x04,0x02,0x00,0x00],
    ']': [0x3F,0x02,0x02,0x02,0x02,0x02,0x3F,0x00],
    '^': [0x08,0x14,0x22,0x00,0x00,0x00,0x00,0x00],
    '_': [0x00,0x00,0x00,0x00,0x00,0x00,0x7F,0x00],
    '`': [0x10,0x08,0x04,0x00,0x00,0x00,0x00,0x00],
    'a': [0x00,0x00,0x3C,0x02,0x3E,0x42,0x3F,0x00],
    'b': [0x40,0x40,0x7C,0x42,0x42,0x42,0x7C,0x00],
    'c': [0x00,0x00,0x3E,0x40,0x40,0x42,0x3C,0x00],
    'd': [0x02,0x02,0x3E,0x42,0x42,0x42,0x3F,0x00],
    'e': [0x00,0x00,0x3C,0x42,0x7E,0x40,0x3C,0x00],
    'f': [0x1C,0x22,0x20,0x7C,0x20,0x20,0x20,0x00],
    'g': [0x00,0x00,0x3E,0x42,0x42,0x3E,0x02,0x3C],
    'h': [0x40,0x40,0x7C,0x42,0x42,0x42,0x42,0x00],
    'i': [0x08,0x00,0x18,0x08,0x08,0x08,0x1C,0x00],
    'j': [0x02,0x00,0x06,0x02,0x02,0x02,0x42,0x3C],
    'k': [0x40,0x40,0x44,0x48,0x70,0x48,0x44,0x00],
    'l': [0x18,0x08,0x08,0x08,0x08,0x08,0x1C,0x00],
    'm': [0x00,0x00,0x6C,0x92,0x92,0x92,0x92,0x00],
    'n': [0x00,0x00,0x7C,0x42,0x42,0x42,0x42,0x00],
    'o': [0x00,0x00,0x3C,0x42,0x42,0x42,0x3C,0x00],
    'p': [0x00,0x00,0x7C,0x42,0x42,0x7C,0x40,0x40],
    'q': [0x00,0x00,0x3E,0x42,0x42,0x3E,0x02,0x02],
    'r': [0x00,0x00,0x5C,0x62,0x40,0x40,0x40,0x00],
    's': [0x00,0x00,0x3E,0x40,0x3E,0x02,0x3C,0x00],
    't': [0x20,0x20,0xFC,0x20,0x20,0x22,0x1C,0x00],
    'u': [0x00,0x00,0x42,0x42,0x42,0x46,0x3A,0x00],
    'v': [0x00,0x00,0x42,0x42,0x24,0x24,0x18,0x00],
    'w': [0x00,0x00,0x42,0x42,0x4A,0x5A,0x24,0x00],
    'x': [0x00,0x00,0x42,0x24,0x18,0x24,0x42,0x00],
    'y': [0x00,0x00,0x42,0x42,0x42,0x3E,0x02,0x3C],
    'z': [0x00,0x00,0x7E,0x04,0x18,0x20,0x7E,0x00],
    '{': [0x0C,0x10,0x10,0x20,0x10,0x10,0x0C,0x00],
    '|': [0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x00],
    '}': [0x30,0x08,0x08,0x04,0x08,0x08,0x30,0x00],
    '~': [0x00,0x00,0x32,0x4C,0x00,0x00,0x00,0x00],
}


def generate_font_tga():
    # 16 cols x 6 rows of 8x8 glyphs = 128x48
    width, height = 128, 48
    pixels = [(0, 0, 0, 0)] * (width * height)
    for i, ch in enumerate(sorted(font_glyphs.keys(), key=lambda c: ord(c))):
        if ord(ch) < 32 or ord(ch) > 127:
            continue
        idx = ord(ch) - 32
        gx = (idx % 16) * 8
        gy = (idx // 16) * 8
        glyph = font_glyphs[ch]
        for row in range(8):
            bits = glyph[row]
            for col in range(8):
                if bits & (0x80 >> col):
                    px(pixels, width, gx + col, gy + row, C["white"])
    save_tga("package/assets/images/font.tga", width, height, pixels)


# ---------------------------------------------------------------------------
# Player "Pulse" — rounded luminous courier (3 poses × 2 skins)
# ---------------------------------------------------------------------------
def _draw_pulse_body(pixels, w, cx, cy, accent, pose="idle"):
    """Shared body drawing. pose: idle | run | jump"""
    body = C["dark"]
    outline = C["edge"]
    visor = accent
    visor_glow = C["cyan_glow"] if accent == C["cyan"] else C["purple"]
    leg_col = C["leg"]

    # Outer soft glow (very faint)
    fill_ellipse(pixels, w, cx, cy - 2, 14, 16, accent, 40)

    # Main body (rounded capsule)
    fill_ellipse(pixels, w, cx, cy - 1, 11, 13, body)
    # Highlight on upper left
    fill_ellipse(pixels, w, cx - 3, cy - 5, 5, 6, C["near"], 180)

    # Outline ring (thin)
    for a in range(0, 360, 6):
        rad = math.radians(a)
        ox = int(cx + 11.5 * math.cos(rad))
        oy = int(cy - 1 + 13.5 * math.sin(rad))
        px(pixels, w, ox, oy, outline, 200)

    # Visor (bright horizontal bar with glow)
    hline(pixels, w, cx - 7, cx + 7, cy - 4, visor_glow, 120)
    hline(pixels, w, cx - 6, cx + 6, cy - 3, visor)
    hline(pixels, w, cx - 6, cx + 6, cy - 2, visor)
    hline(pixels, w, cx - 5, cx + 5, cy - 1, visor_glow, 160)
    # Visor ends
    px(pixels, w, cx - 7, cy - 3, outline)
    px(pixels, w, cx + 7, cy - 3, outline)

    # Small antenna / signal node on top
    vline(pixels, w, cx, cy - 16, cy - 14, accent)
    fill_circle(pixels, w, cx, cy - 17, 2, accent)
    px(pixels, w, cx, cy - 17, C["white"])

    # Legs depending on pose
    if pose == "idle":
        # Two short legs standing
        fill_rect(pixels, w, cx - 6, cy + 12, cx - 3, cy + 18, leg_col)
        fill_rect(pixels, w, cx + 3, cy + 12, cx + 6, cy + 18, leg_col)
        # Feet
        hline(pixels, w, cx - 7, cx - 2, cy + 19, outline)
        hline(pixels, w, cx + 2, cx + 7, cy + 19, outline)
    elif pose == "run":
        # Mid-stride: left forward, right back
        # Left leg (front)
        fill_rect(pixels, w, cx - 8, cy + 11, cx - 4, cy + 17, leg_col)
        hline(pixels, w, cx - 9, cx - 3, cy + 18, outline)
        # Right leg (back, slightly up)
        fill_rect(pixels, w, cx + 3, cy + 10, cx + 7, cy + 15, leg_col)
        hline(pixels, w, cx + 2, cx + 8, cy + 16, outline)
        # Motion streak
        hline(pixels, w, cx - 14, cx - 10, cy + 4, accent, 80)
    elif pose == "jump":
        # Tucked legs + upward feel
        fill_rect(pixels, w, cx - 5, cy + 11, cx - 2, cy + 15, leg_col)
        fill_rect(pixels, w, cx + 2, cy + 11, cx + 5, cy + 15, leg_col)
        # Small thruster glow under body
        fill_ellipse(pixels, w, cx, cy + 14, 6, 3, accent, 140)
        hline(pixels, w, cx - 4, cx + 4, cy + 16, visor_glow, 100)


def generate_player_sprites():
    size = 64
    for skin, accent, prefix in [
        (C["cyan"], C["cyan"], "runner"),
        (C["purple"], C["purple"], "spacesuit"),
    ]:
        for pose in ("idle", "run", "jump"):
            pixels = [(0, 0, 0, 0)] * (size * size)
            cx, cy = 32, 30
            _draw_pulse_body(pixels, size, cx, cy, accent, pose)
            # Extra skin detail: spacesuit has shoulder pads
            if prefix == "spacesuit":
                fill_rect(pixels, size, cx - 13, cy - 6, cx - 10, cy + 2, C["metal"])
                fill_rect(pixels, size, cx + 10, cy - 6, cx + 13, cy + 2, C["metal"])
                px(pixels, size, cx - 12, cy - 4, accent)
                px(pixels, size, cx + 12, cy - 4, accent)
            save_tga(f"package/assets/images/{prefix}_{pose}.tga", size, size, pixels)


# ---------------------------------------------------------------------------
# Enemy drone — original hexagonal floating unit
# ---------------------------------------------------------------------------
def generate_enemy_tga():
    size = 64
    pixels = [(0, 0, 0, 0)] * (size * size)
    cx, cy = 32, 30

    # Soft outer glow
    fill_ellipse(pixels, size, cx, cy, 22, 18, C["purple"], 35)

    # Hexagon body (flat-top)
    # Approximate hex by points
    def in_hex(px_, py_, r):
        dx = abs(px_ - cx)
        dy = abs(py_ - cy)
        return (dx <= r * 0.866) and (dy <= r * 0.5 + (r * 0.5 - dx * 0.5 / 0.866))

    for y in range(size):
        for x in range(size):
            dx = abs(x - cx)
            dy = abs(y - cy)
            # Flat-top hex test
            if dx * 0.57735 + dy <= 18 and dx <= 16:
                # Body gradient
                if dx * 0.57735 + dy <= 14 and dx <= 12:
                    # Inner core
                    if dx <= 4 and dy <= 3:
                        pixels[y * size + x] = (*C["red"], 255)
                    else:
                        pixels[y * size + x] = (*C["purple"], 255)
                else:
                    # Rim
                    pixels[y * size + x] = (*C["purple_d"], 255)

    # Bright eye / sensor
    fill_circle(pixels, size, cx, cy - 2, 5, C["red"])
    fill_circle(pixels, size, cx, cy - 2, 2, C["white"])

    # Bottom thruster glow
    fill_ellipse(pixels, size, cx, cy + 16, 8, 4, C["cyan"], 180)
    fill_ellipse(pixels, size, cx, cy + 18, 5, 3, C["cyan_glow"], 140)
    hline(pixels, size, cx - 6, cx + 6, cy + 20, C["cyan"], 100)

    # Side fins
    for side in (-1, 1):
        fill_rect(pixels, size, cx + side * 15, cy - 4, cx + side * 18, cy + 4, C["metal"])
        px(pixels, size, cx + side * 16, cy, C["cyan"])

    # Top antenna
    vline(pixels, size, cx, cy - 20, cy - 16, C["edge"])
    fill_circle(pixels, size, cx, cy - 21, 2, C["orange"])

    save_tga("package/assets/images/enemy.tga", size, size, pixels)


# ---------------------------------------------------------------------------
# Energy shard (collectible)
# ---------------------------------------------------------------------------
def generate_shard_tga():
    size = 32
    pixels = [(0, 0, 0, 0)] * (size * size)
    cx, cy = 16, 16

    # Soft glow
    fill_circle(pixels, size, cx, cy, 12, C["yellow"], 50)

    # Diamond (rotated square)
    for y in range(size):
        for x in range(size):
            dx = abs(x - cx)
            dy = abs(y - cy)
            if dx + dy <= 10:
                # Bright core
                if dx + dy <= 5:
                    pixels[y * size + x] = (*C["white"], 255)
                elif dx + dy <= 7:
                    pixels[y * size + x] = (*C["yellow_g"], 255)
                else:
                    pixels[y * size + x] = (*C["yellow"], 255)
            elif dx + dy <= 12:
                # Outer soft
                pixels[y * size + x] = (*C["yellow"], 100)

    # Sparkle points
    for sx, sy in [(16, 4), (16, 28), (4, 16), (28, 16)]:
        px(pixels, size, sx, sy, C["white"])

    save_tga("package/assets/images/shard.tga", size, size, pixels)


# ---------------------------------------------------------------------------
# Platform — industrial neon building facade (stretches cleanly)
# ---------------------------------------------------------------------------
def generate_platform_tga():
    # Wide canvas so horizontal neon bands look good when stretched
    width, height = 256, 160
    pixels = [(0, 0, 0, 0)] * (width * height)

    # Background panel (dark metal)
    fill_rect(pixels, width, 0, 0, width - 1, height - 1, C["panel"])

    # Subtle vertical panel divisions every 32 px
    for x in range(0, width, 32):
        vline(pixels, width, x, 0, height - 1, C["dark"], 180)
        vline(pixels, width, x + 1, 0, height - 1, C["metal"], 80)

    # Horizontal structural ribs
    for y in (20, 48, 76, 104, 132):
        hline(pixels, width, 0, width - 1, y, C["metal"])
        hline(pixels, width, 0, width - 1, y + 1, C["dark"])

    # Neon cyan light tubes (bright, full-width) — these stretch beautifully
    neon_rows = [
        (8, C["cyan_glow"], C["cyan"]),
        (36, C["cyan"], C["cyan_dim"]),
        (64, C["cyan_glow"], C["cyan"]),
        (92, C["cyan"], C["cyan_dim"]),
        (120, C["cyan_glow"], C["cyan"]),
        (148, C["cyan"], C["cyan_dim"]),
    ]
    for y, bright, dim in neon_rows:
        # Glow above/below
        hline(pixels, width, 0, width - 1, y - 1, bright, 60)
        hline(pixels, width, 0, width - 1, y + 2, bright, 60)
        # Core tube
        hline(pixels, width, 0, width - 1, y, bright)
        hline(pixels, width, 0, width - 1, y + 1, dim)

    # Top edge (lighter walkable surface)
    fill_rect(pixels, width, 0, 0, width - 1, 5, C["edge"])
    hline(pixels, width, 0, width - 1, 0, C["white"], 180)
    hline(pixels, width, 0, width - 1, 6, C["cyan"], 200)

    # Small window lights (random-looking but deterministic)
    for i, (wx, wy) in enumerate([(18, 28), (50, 56), (82, 28), (114, 84),
                                   (146, 56), (178, 28), (210, 84), (242, 56),
                                   (34, 112), (98, 112), (162, 112), (226, 112)]):
        col = C["purple"] if i % 3 == 0 else C["cyan_dim"]
        fill_rect(pixels, width, wx, wy, wx + 6, wy + 5, col)
        px(pixels, width, wx + 1, wy + 1, C["white"], 200)

    # Corner bolts / rivets
    for bx in (4, width - 5):
        for by in (12, 44, 72, 100, 128, 152):
            fill_circle(pixels, width, bx, by, 2, C["metal"])
            px(pixels, width, bx, by, C["edge"])

    # Bottom shadow edge
    hline(pixels, width, 0, width - 1, height - 1, C["dark"])

    save_tga("package/assets/images/platform.tga", width, height, pixels)


# ---------------------------------------------------------------------------
# Warning / hazard stripe
# ---------------------------------------------------------------------------
def generate_warning_tga():
    size = 32
    pixels = [(0, 0, 0, 0)] * (size * size)
    for y in range(size):
        for x in range(size):
            if ((x + y) // 6) % 2 == 0:
                pixels[y * size + x] = (*C["orange"], 255)
            else:
                pixels[y * size + x] = (*C["dark"], 255)
    save_tga("package/assets/images/warning.tga", size, size, pixels)


# ---------------------------------------------------------------------------
# Splash / logo banner
# ---------------------------------------------------------------------------
def generate_splash_tga():
    width, height = 320, 80
    pixels = [(0, 0, 0, 0)] * (width * height)

    # Gradient background
    for y in range(height):
        for x in range(width):
            t = x / (width - 1)
            r = int(17 * (1 - t) + 34 * t)
            g = int(24 * (1 - t) + 211 * t)
            b = int(39 * (1 - t) + 238 * t)
            pixels[y * width + x] = (r, g, b, 255)

    # Simple geometric mark on left (stylized pulse / city)
    fill_rect(pixels, width, 12, 20, 28, 60, C["dark"])
    hline(pixels, width, 12, 28, 20, C["cyan"])
    hline(pixels, width, 12, 28, 32, C["cyan"])
    hline(pixels, width, 12, 28, 44, C["cyan"])
    fill_circle(pixels, width, 20, 52, 4, C["cyan"])

    save_tga("package/assets/images/splash.tga", width, height, pixels)


# ---------------------------------------------------------------------------
# Audio (same as before, short original SFX)
# ---------------------------------------------------------------------------
def generate_audio():
    sample_rate = 22050

    # Jump — upward sine sweep
    duration = 0.12
    n = int(sample_rate * duration)
    jump = []
    for i in range(n):
        t = i / sample_rate
        freq = 280 + 520 * (t / duration)
        env = 1.0 - (t / duration) ** 0.7
        jump.append(math.sin(2 * math.pi * freq * t) * env * 0.45)
    save_wav("package/assets/audio/jump.wav", sample_rate, jump)

    # Pickup — bright two-tone chime
    pickup = []
    for freq, dur in [(660, 0.05), (990, 0.09)]:
        n = int(sample_rate * dur)
        for i in range(n):
            t = i / sample_rate
            env = 1.0 - (t / dur)
            pickup.append(math.sin(2 * math.pi * freq * t) * env * 0.4)
    save_wav("package/assets/audio/pickup.wav", sample_rate, pickup)

    # Failure — descending tone + noise
    duration = 0.35
    n = int(sample_rate * duration)
    failure = []
    state = 12345
    for i in range(n):
        t = i / sample_rate
        freq = 320 - 260 * (t / duration)
        state = (state * 1103515245 + 12345) & 0x7FFFFFFF
        noise = (state / 0x7FFFFFFF) * 2 - 1
        env = 1.0 - (t / duration)
        failure.append((math.sin(2 * math.pi * freq * t) * 0.55 + noise * 0.15) * env * 0.5)
    save_wav("package/assets/audio/failure.wav", sample_rate, failure)


if __name__ == "__main__":
    print("Generating improved Skyline Sprint pixel-art assets...")
    generate_font_tga()
    generate_player_sprites()
    generate_enemy_tga()
    generate_shard_tga()
    generate_platform_tga()
    generate_warning_tga()
    generate_splash_tga()
    generate_audio()
    print("All assets generated successfully.")
