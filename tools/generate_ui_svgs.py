from pathlib import Path
from xml.sax.saxutils import escape

OUT = Path("docs/ui-svg")
W, H = 320, 240

COLORS = {
    "bg": "#000000",
    "surface": "#181c18",
    "panel": "#101410",
    "border": "#393c39",
    "primary": "#00bfff",
    "secondary": "#ffaa00",
    "accent": "#00ffff",
    "success": "#00ff00",
    "warning": "#ffff00",
    "error": "#ff0000",
    "text": "#ffffff",
    "dim": "#9c9c9c",
}


class SVG:
    def __init__(self, title):
        self.title = title
        self.parts = [
            f'<svg xmlns="http://www.w3.org/2000/svg" width="{W}" height="{H}" viewBox="0 0 {W} {H}">',
            f"<title>{escape(title)}</title>",
            f'<rect width="{W}" height="{H}" fill="{COLORS["bg"]}"/>',
        ]

    def rect(self, x, y, w, h, fill, stroke=None, r=0, sw=1):
        attrs = f'x="{x}" y="{y}" width="{w}" height="{h}" fill="{fill}"'
        if r:
            attrs += f' rx="{r}" ry="{r}"'
        if stroke:
            attrs += f' stroke="{stroke}" stroke-width="{sw}"'
        self.parts.append(f"<rect {attrs}/>")

    def circle(self, x, y, r, fill, stroke=None, sw=1):
        attrs = f'cx="{x}" cy="{y}" r="{r}" fill="{fill}"'
        if stroke:
            attrs += f' stroke="{stroke}" stroke-width="{sw}"'
        self.parts.append(f"<circle {attrs}/>")

    def line(self, x1, y1, x2, y2, stroke, sw=1):
        self.parts.append(f'<line x1="{x1}" y1="{y1}" x2="{x2}" y2="{y2}" stroke="{stroke}" stroke-width="{sw}"/>')

    def text(self, text, x, y, size=10, fill=None, anchor="start", weight="500"):
        fill = fill or COLORS["text"]
        self.parts.append(
            f'<text x="{x}" y="{y}" fill="{fill}" font-family="Arial, Helvetica, sans-serif" '
            f'font-size="{size}" font-weight="{weight}" text-anchor="{anchor}">{escape(str(text))}</text>'
        )

    def polyline(self, points, stroke, sw=2, fill="none"):
        pts = " ".join(f"{x},{y}" for x, y in points)
        self.parts.append(f'<polyline points="{pts}" fill="{fill}" stroke="{stroke}" stroke-width="{sw}" stroke-linecap="round" stroke-linejoin="round"/>')

    def finish(self):
        self.parts.append("</svg>")
        return "\n".join(self.parts) + "\n"


def header(s, title, subtitle="", connected=False):
    s.rect(0, 0, 320, 42, COLORS["surface"])
    s.line(0, 42, 320, 42, COLORS["border"])
    s.rect(8, 8, 46, 26, COLORS["panel"], COLORS["error"], 6)
    s.text("BACK", 31, 25, 10, COLORS["text"], "middle", "700")
    s.text(title, 160, 20, 18, COLORS["text"], "middle", "700")
    if subtitle:
        s.text(subtitle, 160, 35, 10, COLORS["dim"], "middle")
    s.circle(304, 14, 5, COLORS["success"] if connected else COLORS["error"], COLORS["border"])


def button(s, x, y, w, h, label, color, pressed=False):
    s.rect(x + 1, y + 2, w, h, COLORS["bg"], r=6)
    s.rect(x, y + (1 if pressed else 0), w, h, color if pressed else COLORS["panel"], color, 6)
    s.text(label, x + w / 2, y + h / 2 + 4 + (1 if pressed else 0), 10, COLORS["bg"] if pressed else COLORS["text"], "middle", "700")


def menu_card(s, idx, name, color, glyph):
    cell_w, cell_h, icon, spacing, row_spacing, cols = 58, 62, 36, 4, 10, 5
    start_x = (320 - (cols * cell_w + (cols - 1) * spacing)) // 2
    x = start_x + (idx % cols) * (cell_w + spacing)
    y = 62 + (idx // cols) * (cell_h + row_spacing)
    s.rect(x + 1, y + 2, cell_w, cell_h, COLORS["bg"], r=6)
    s.rect(x, y, cell_w, cell_h, COLORS["panel"], COLORS["border"], 6)
    s.rect(x + 11, y + 7, icon, icon, color, r=6)
    draw_icon(s, glyph, x + 11, y + 7, icon, COLORS["bg"])
    s.text(name, x + cell_w / 2, y + 55, 8, COLORS["dim"], "middle", "700")


def draw_icon(s, glyph, x, y, size, color):
    cx, cy = x + size / 2, y + size / 2
    if glyph == "keys":
        for i in range(5):
            s.rect(x + 8 + i * 4, y + 12, 3, 14, color)
    elif glyph == "beats":
        for r in range(3):
            for c in range(4):
                s.rect(x + 8 + c * 5, y + 10 + r * 6, 4, 4, color)
    elif glyph == "zen":
        s.circle(cx, cy, 12, "none", color, 2)
        s.circle(cx - 6, cy - 4, 2, color)
        s.circle(cx + 5, cy + 2, 2, color)
        s.circle(cx - 2, cy + 7, 2, color)
    elif glyph == "drop":
        s.circle(cx - 6, cy - 8, 2, color)
        s.circle(cx + 3, cy - 2, 2, color)
        s.rect(cx - 10, cy + 8, 9, 2, color)
        s.rect(cx + 3, cy + 4, 8, 2, color)
    elif glyph == "rng":
        for dx, dy in [(-8, -6), (-1, -3), (7, 2), (-4, 7)]:
            s.circle(cx + dx, cy + dy, 2, color)
    elif glyph == "xy":
        s.line(cx - 8, cy, cx + 8, cy, color, 2)
        s.line(cx, cy - 8, cx, cy + 8, color, 2)
        s.circle(cx, cy, 3, color)
    elif glyph == "arp":
        for i in range(4):
            s.circle(cx - 8 + i * 5, cy + 7 - i * 4, 2, color)
    elif glyph == "grid":
        for r in range(3):
            for c in range(4):
                s.rect(x + 7 + c * 6, y + 9 + r * 7, 5, 5, "none", color)
    elif glyph == "chord":
        for yy in [cy - 5, cy, cy + 5]:
            s.line(cx - 8, yy, cx + 8, yy, color, 3)
    elif glyph == "lfo":
        pts = [(x + 5 + i * 2, cy + 7 * __import__("math").sin(i * 0.8)) for i in range(14)]
        s.polyline(pts, color, 2)


def make_menu():
    s = SVG("CYD MIDI menu")
    s.rect(0, 0, 320, 48, COLORS["surface"])
    s.line(0, 47, 320, 47, COLORS["border"])
    s.rect(0, 48, 320, 2, COLORS["primary"])
    s.text("CYD MIDI", 12, 26, 18, COLORS["text"], weight="700")
    s.text("controller", 14, 40, 10, COLORS["dim"])
    s.rect(222, 11, 86, 24, "#280000", COLORS["error"], 5)
    s.circle(236, 23, 4, COLORS["error"])
    s.text("WAITING", 246, 28, 10, COLORS["text"], weight="700")
    apps = [
        ("KEYS", "#ff2ca3", "keys"), ("BEATS", "#ffae00", "beats"), ("ZEN", "#bfff00", "zen"),
        ("DROP", "#00fa80", "drop"), ("RNG", "#369bff", "rng"), ("XY PAD", "#8c4fff", "xy"),
        ("ARP", "#ff00ff", "arp"), ("GRID", "#00bfff", "grid"), ("CHORD", "#ffdf00", "chord"),
        ("LFO", "#97ffff", "lfo"),
    ]
    for i, app in enumerate(apps):
        menu_card(s, i, *app)
    s.text("Open MIDIberry or BLE-MIDI Connect", 160, 228, 11, COLORS["dim"], "middle")
    return s.finish()


def make_keys():
    s = SVG("KEYS")
    header(s, "KEYS", "C Major  1")
    for row in range(2):
        for key in range(10):
            x, y, w, h = key * 32, 52 + row * 60, 29, 56
            fill = COLORS["surface"] if row == 0 else COLORS["panel"]
            s.rect(x + 1, y + 1, w, h, fill, COLORS["border"], 4)
            s.text(["C3", "D3", "E3", "F3", "G3", "A3", "B3", "C4", "D4", "E4"][key], x + 16, y + 28, 10, COLORS["text"], "middle", "700")
            s.text("LOW" if row == 0 else "HIGH", x + 16, y + 47, 6, COLORS["dim"], "middle")
    for args in [
        (8, 174, 30, 18, "O-", COLORS["secondary"]),
        (44, 174, 30, 18, "O+", COLORS["secondary"]),
        (82, 174, 44, 18, "1", COLORS["accent"]),
        (134, 174, 44, 18, "TAP", COLORS["secondary"]),
        (186, 174, 34, 18, "K-", COLORS["warning"]),
        (228, 174, 34, 18, "K+", COLORS["warning"]),
        (270, 174, 42, 18, "SCALE", COLORS["accent"]),
    ]:
        button(s, *args)
    s.rect(8, 204, 304, 28, COLORS["panel"], COLORS["border"], 5)
    s.text("Oct 4  Vel 100  1  C Major", 160, 222, 8, COLORS["dim"], "middle")
    return s.finish()


def make_beats():
    s = SVG("BEATS")
    header(s, "BEATS", "FOUR  120 BPM  SW50")
    s.rect(6, 46, 308, 114, COLORS["panel"], COLORS["border"], 6)
    labels = ["KCK", "SNR", "HAT", "OPN"]
    cols = [COLORS["error"], COLORS["warning"], COLORS["primary"], COLORS["accent"]]
    pattern = [
        {0: 2, 4: 2, 8: 2, 12: 2},
        {4: 2, 12: 2},
        {2: 1, 6: 1, 10: 1, 14: 3},
        {15: 1},
    ]
    for beat in range(4):
        x = 48 + beat * 4 * 16 - 1
        s.line(x, 50, x, 154, COLORS["border"])
    for tr, label in enumerate(labels):
        y = 52 + tr * 28
        s.rect(10, y + 1, 34, 22, COLORS["panel"], cols[tr], 4)
        s.text(label, 27, y + 16, 7, cols[tr], "middle", "700")
        for st in range(16):
            x = 48 + st * 16
            level = pattern[tr].get(st, 0)
            fill = COLORS["surface"] if st % 4 == 0 else COLORS["bg"]
            stroke = COLORS["border"]
            if level == 1:
                fill = cols[tr]
            elif level == 2:
                fill = COLORS["text"]
                stroke = cols[tr]
            elif level == 3:
                fill = COLORS["surface"]
                stroke = cols[tr]
            s.rect(x, y, 15, 24, fill, stroke, 3)
            if level == 2:
                s.circle(x + 7.5, y + 12, 3, cols[tr])
            if level == 3:
                s.rect(x + 4, y + 6, 2, 12, cols[tr])
                s.rect(x + 9, y + 6, 2, 12, cols[tr])
    for args in [
        (8, 166, 46, 18, "PLAY", COLORS["success"]),
        (60, 166, 38, 18, "PAT", COLORS["accent"]),
        (104, 166, 38, 18, "GEN", COLORS["primary"]),
        (148, 166, 38, 18, "VAR", COLORS["warning"]),
        (194, 166, 38, 18, "FILL", COLORS["warning"]),
        (240, 166, 34, 18, "CLR", COLORS["error"]),
        (284, 166, 28, 18, "MIX", COLORS["secondary"]),
        (8, 188, 38, 18, "B-", COLORS["secondary"]),
        (100, 188, 38, 18, "B+", COLORS["secondary"]),
        (148, 188, 28, 18, "S-", COLORS["secondary"]),
        (216, 188, 28, 18, "S+", COLORS["secondary"]),
        (252, 188, 28, 18, "P-", COLORS["secondary"]),
        (284, 188, 28, 18, "P+", COLORS["secondary"]),
        (8, 210, 34, 18, "<", COLORS["accent"]),
        (48, 210, 34, 18, ">", COLORS["accent"]),
        (90, 210, 30, 18, "H-", COLORS["secondary"]),
        (160, 210, 30, 18, "H+", COLORS["secondary"]),
    ]:
        button(s, *args)
    s.rect(52, 188, 42, 18, COLORS["panel"], COLORS["border"], 5)
    s.text("120", 73, 201, 8, COLORS["text"], "middle", "700")
    s.rect(182, 188, 28, 18, COLORS["panel"], COLORS["border"], 5)
    s.text("50", 196, 201, 8, COLORS["dim"], "middle", "700")
    s.rect(126, 210, 28, 18, COLORS["panel"], COLORS["border"], 5)
    s.text("6", 140, 223, 8, COLORS["warning"], "middle", "700")
    s.rect(198, 210, 114, 18, COLORS["panel"], COLORS["border"], 5)
    s.text("P92 D58 CH10", 255, 223, 8, COLORS["dim"], "middle", "700")
    return s.finish()


def make_xy():
    s = SVG("XY PAD")
    header(s, "XY PAD", "Touch Control")
    s.rect(10, 52, 210, 142, COLORS["panel"], COLORS["border"], 6)
    s.line(16, 123, 214, 123, COLORS["dim"])
    s.line(115, 58, 115, 188, COLORS["dim"])
    s.circle(115, 123, 18, "none", COLORS["surface"])
    s.circle(150, 92, 9, COLORS["primary"])
    s.circle(150, 92, 5, COLORS["accent"])
    s.rect(10, 202, 210, 26, COLORS["panel"], r=4)
    s.text("X 84", 22, 220, 12, COLORS["primary"], weight="700")
    s.text("Y 93", 128, 220, 12, COLORS["accent"], weight="700")
    s.rect(232, 52, 88, 180, COLORS["panel"], COLORS["border"], 6)
    for y, lab, val, col in [(62, "X CC", "74", COLORS["primary"]), (150, "Y CC", "71", COLORS["accent"])]:
        s.text(lab, 276, y + 10, 10, col, "middle", "700")
        button(s, 240, y + 34, 32, 28, "-", COLORS["secondary"])
        button(s, 280, y + 34, 32, 28, "+", COLORS["secondary"])
        s.rect(254, y + 68, 44, 24, COLORS["bg"], r=4)
        s.text(val, 276, y + 85, 11, COLORS["text"], "middle", "700")
    return s.finish()


def make_chord():
    s = SVG("CHORD")
    header(s, "CHORD", "C Major  TRI")
    s.rect(6, 58, 308, 108, COLORS["panel"], COLORS["border"], 6)
    names = ["I", "ii", "iii", "IV", "V", "vi", "vii", "I+"]
    roots = ["C3", "D3", "E3", "F3", "G3", "A3", "B3", "C4"]
    types = ["maj", "min", "min", "maj", "maj", "min", "dim", "maj"]
    cols = [COLORS["primary"], COLORS["secondary"], COLORS["accent"], COLORS["success"], COLORS["warning"], COLORS["error"], "#ff00ff", "#00ff00"]
    for i in range(8):
        x = 10 + i * 39
        s.rect(x, 66, 37, 94, COLORS["bg"], cols[i], 5)
        s.text(names[i], x + 18.5, 91, 12, COLORS["text"], "middle", "700")
        s.text(roots[i], x + 18.5, 112, 8, COLORS["dim"], "middle")
        s.text(types[i], x + 18.5, 132, 8, COLORS["dim"], "middle")
    for args in [
        (8, 182, 28, 18, "O-", COLORS["secondary"]),
        (42, 182, 28, 18, "O+", COLORS["secondary"]),
        (78, 182, 44, 18, "TRI", COLORS["accent"]),
        (130, 182, 34, 18, "INV", COLORS["warning"]),
        (172, 182, 42, 18, "TAP", COLORS["secondary"]),
        (222, 182, 42, 18, "ROOT", COLORS["secondary"]),
        (272, 182, 38, 18, "CLR", COLORS["error"]),
        (8, 204, 34, 18, "K-", COLORS["secondary"]),
        (50, 204, 34, 18, "K+", COLORS["secondary"]),
        (94, 204, 52, 18, "SCALE", COLORS["accent"]),
    ]:
        button(s, *args)
    s.rect(154, 204, 156, 18, COLORS["panel"], COLORS["border"], 4)
    s.text("Oct 4 Inv 0", 232, 216, 8, COLORS["dim"], "middle")
    return s.finish()


def make_grid():
    s = SVG("GRID")
    header(s, "GRID", "C Major  4TH")
    s.rect(6, 50, 308, 144, COLORS["panel"], COLORS["border"], 6)
    notes = ["G", "G#", "A", "A#", "B", "C", "C#", "D"]
    scale_notes = {"C", "D", "E", "F", "G", "A", "B"}
    for r in range(5):
        for c in range(8):
            x, y = 13 + c * 38, 58 + r * 27
            note = notes[(c + (4 - r) * 5) % 8]
            root = note == "C"
            in_scale = note.replace("#", "") in scale_notes and "#" not in note
            fill = COLORS["accent"] if root else (COLORS["surface"] if in_scale else COLORS["bg"])
            stroke = COLORS["text"] if root else (COLORS["primary"] if in_scale else COLORS["border"])
            text = COLORS["bg"] if root else (COLORS["text"] if in_scale else COLORS["dim"])
            s.rect(x, y, 35, 24, fill, stroke, 4)
            s.text(note, x + 17.5, y + 16, 8, text, "middle")
    for args in [
        (8, 200, 34, 18, "O-", COLORS["secondary"]),
        (48, 200, 34, 18, "O+", COLORS["secondary"]),
        (90, 200, 42, 18, "4TH", COLORS["accent"]),
        (140, 200, 46, 18, "FREE", COLORS["secondary"]),
        (194, 200, 46, 18, "TAP", COLORS["secondary"]),
        (248, 200, 54, 18, "SCALE", COLORS["warning"]),
    ]:
        button(s, *args)
    s.rect(8, 222, 304, 16, COLORS["panel"], COLORS["border"], 4)
    s.text("Oct 3  Vel 100  Ready", 160, 234, 8, COLORS["dim"], "middle")
    return s.finish()


def make_lfo():
    s = SVG("LFO")
    header(s, "LFO MOD", "CC 1")
    s.rect(6, 50, 308, 118, COLORS["panel"], COLORS["border"], 6)
    button(s, 14, 56, 62, 30, "START", COLORS["success"])
    s.text("RATE", 88, 64, 8, COLORS["dim"])
    s.text("1.0Hz", 88, 80, 12, COLORS["text"], weight="700")
    button(s, 156, 56, 30, 30, "-", COLORS["secondary"])
    button(s, 192, 56, 30, 30, "+", COLORS["secondary"])
    button(s, 238, 56, 66, 30, "SINE", COLORS["accent"])
    s.text("AMT", 16, 100, 8, COLORS["dim"])
    s.text("64", 16, 116, 12, COLORS["text"], weight="700")
    s.rect(144, 101, 160, 14, COLORS["bg"], COLORS["border"], 3)
    s.rect(145, 102, 78, 12, COLORS["primary"], r=2)
    s.text("TARGET", 16, 136, 8, COLORS["dim"])
    s.text("CC1", 16, 152, 12, COLORS["text"], weight="700")
    button(s, 90, 128, 30, 30, "-", COLORS["secondary"])
    button(s, 126, 128, 30, 30, "+", COLORS["secondary"])
    button(s, 174, 128, 76, 30, "PITCH", COLORS["warning"])
    s.rect(260, 128, 44, 30, COLORS["bg"], r=5)
    s.text("64", 282, 148, 12, COLORS["accent"], "middle", "700")
    s.rect(10, 178, 300, 48, COLORS["panel"], COLORS["border"], 6)
    s.line(18, 202, 302, 202, COLORS["dim"])
    import math
    pts = [(18 + i * 6, 202 - math.sin(i * 0.35) * 15) for i in range(48)]
    s.polyline(pts, COLORS["primary"], 2)
    return s.finish()


def make_rng():
    s = SVG("RNG")
    header(s, "RNG JAMS", "C Major  120 BPM")
    s.rect(6, 50, 308, 178, COLORS["panel"], COLORS["border"], 6)
    y = 56
    for args in [
        (14, y, 50, 24, "PLAY", COLORS["success"]),
        (70, y, 42, 24, "NEW", COLORS["accent"]),
        (118, y, 42, 24, "VAR", COLORS["warning"]),
        (168, y, 56, 24, "MID", COLORS["primary"]),
        (232, y, 32, 24, "LEN", COLORS["secondary"]),
    ]:
        button(s, *args)
    s.rect(272, y, 34, 24, COLORS["bg"], COLORS["border"], 5)
    s.text("8", 289, y + 16, 8, COLORS["text"], "middle", "700")
    y += 30
    s.text("ROOT", 16, y + 16, 8, COLORS["dim"])
    button(s, 52, y, 24, 24, "-", COLORS["secondary"])
    s.rect(82, y, 34, 24, COLORS["bg"], r=5)
    s.text("C", 99, y + 16, 8, COLORS["text"], "middle", "700")
    button(s, 122, y, 24, 24, "+", COLORS["secondary"])
    button(s, 156, y, 66, 24, "Major", COLORS["accent"])
    button(s, 232, y, 28, 24, "B-", COLORS["secondary"])
    button(s, 264, y, 34, 24, "B+", COLORS["secondary"])
    y += 30
    s.text("DENS", 16, y + 16, 8, COLORS["dim"])
    button(s, 54, y, 24, 24, "-", COLORS["secondary"])
    s.text("55", 98, y + 16, 8, COLORS["text"], "middle", "700")
    button(s, 120, y, 24, 24, "+", COLORS["secondary"])
    s.text("VAR", 160, y + 16, 8, COLORS["dim"])
    button(s, 190, y, 24, 24, "-", COLORS["secondary"])
    s.text("25", 232, y + 16, 8, COLORS["text"], "middle", "700")
    button(s, 256, y, 24, 24, "+", COLORS["secondary"])
    y += 30
    s.text("GATE", 16, y + 16, 8, COLORS["dim"])
    button(s, 54, y, 24, 24, "-", COLORS["secondary"])
    s.text("65", 98, y + 16, 8, COLORS["text"], "middle", "700")
    button(s, 120, y, 24, 24, "+", COLORS["secondary"])
    s.text("BEAT", 160, y + 16, 8, COLORS["dim"])
    button(s, 198, y, 24, 24, "<", COLORS["secondary"])
    s.text("1/8", 244, y + 16, 8, COLORS["text"], "middle", "700")
    button(s, 278, y, 24, 24, ">", COLORS["secondary"])
    y += 31
    s.rect(14, y, 292, 34, COLORS["bg"], r=5)
    phrase = [1, 5, None, 3, 6, 5, None, 1]
    for i in range(16):
        x = 20 + i * 18
        active = i < 8
        rest = i < 8 and phrase[i] is None
        fill = COLORS["panel"] if not active else (COLORS["bg"] if rest else COLORS["primary"])
        stroke = COLORS["panel"] if not active else COLORS["border"]
        s.rect(x, y + 5, 16, 20, fill, stroke, 3)
        if active and not rest:
            s.text(str(phrase[i]), x + 8, y + 19, 8, COLORS["bg"], "middle", "700")
    return s.finish()


def make_zen():
    s = SVG("ZEN")
    header(s, "ZEN", "Ambient Generative")
    s.rect(44, 54, 238, 132, COLORS["panel"], COLORS["border"], 6)
    for x in range(50, 274, 28):
        s.rect(x, 60, 28, 3, COLORS["primary"])
        s.rect(x, 177, 28, 3, COLORS["accent"])
    for y in range(63, 175, 28):
        s.rect(50, y, 3, 28, COLORS["warning"])
        s.rect(272, y, 3, 28, COLORS["secondary"])
    s.circle(130, 116, 6, "#5abfff", COLORS["text"])
    s.circle(194, 92, 5, "#b2ff7a", COLORS["text"])
    s.rect(8, 188, 304, 12, COLORS["panel"], r=3)
    s.text("C Major  D65 L240  B2", 160, 198, 8, COLORS["dim"], "middle")
    for args in [
        (8, 202, 36, 16, "ADD", COLORS["success"]),
        (50, 202, 44, 16, "CALM", COLORS["primary"]),
        (100, 202, 48, 16, "CHAOS", COLORS["error"]),
        (156, 202, 26, 16, "D-", COLORS["secondary"]),
        (188, 202, 26, 16, "D+", COLORS["secondary"]),
        (222, 202, 34, 16, "L-", COLORS["secondary"]),
        (264, 202, 34, 16, "L+", COLORS["secondary"]),
        (8, 222, 44, 16, "RESET", COLORS["warning"]),
        (60, 222, 46, 16, "SCALE", COLORS["accent"]),
        (114, 222, 34, 16, "KEY-", COLORS["secondary"]),
        (156, 222, 34, 16, "KEY+", COLORS["secondary"]),
        (198, 222, 28, 16, "O", COLORS["primary"]),
    ]:
        button(s, *args)
    s.rect(236, 222, 76, 16, COLORS["panel"], COLORS["border"], 4)
    s.text("Oct 4", 274, 234, 8, COLORS["dim"], "middle")
    return s.finish()


def make_drop():
    s = SVG("DROP")
    header(s, "DROP", "Tap to Drop")
    s.rect(6, 50, 308, 134, COLORS["panel"], COLORS["border"], 6)
    s.rect(72, 160, 68, 8, COLORS["primary"], COLORS["border"])
    s.rect(178, 140, 58, 8, COLORS["secondary"], COLORS["border"])
    s.rect(118, 118, 48, 8, COLORS["accent"], COLORS["border"])
    s.circle(110, 86, 5, "#7bcfff", COLORS["text"])
    s.circle(170, 105, 4, "#ffcf5a", COLORS["text"])
    s.rect(8, 187, 304, 12, COLORS["panel"], r=3)
    s.text("C Major  Oct 4  G15 L180", 160, 197, 8, COLORS["dim"], "middle")
    for args in [
        (8, 202, 40, 16, "EDIT", COLORS["warning"]),
        (54, 202, 42, 16, "PRE", COLORS["accent"]),
        (104, 202, 34, 16, "CLR", COLORS["error"]),
        (146, 202, 26, 16, "G-", COLORS["secondary"]),
        (178, 202, 26, 16, "G+", COLORS["secondary"]),
        (212, 202, 34, 16, "L-", COLORS["secondary"]),
        (254, 202, 34, 16, "L+", COLORS["secondary"]),
        (8, 222, 46, 16, "SCALE", COLORS["accent"]),
        (62, 222, 34, 16, "K-", COLORS["secondary"]),
        (104, 222, 34, 16, "K+", COLORS["secondary"]),
        (146, 222, 28, 16, "O", COLORS["primary"]),
    ]:
        button(s, *args)
    s.rect(184, 222, 128, 16, COLORS["panel"], COLORS["border"], 4)
    s.text("RISE  B2 P3", 248, 234, 8, COLORS["dim"], "middle")
    return s.finish()


def make_arp():
    s = SVG("ARP")
    header(s, "ARP", "C Major  120 BPM")
    s.rect(6, 48, 308, 108, COLORS["panel"], COLORS["border"], 6)
    y = 52
    s.text("PAT", 12, y + 16, 8, COLORS["dim"])
    button(s, 40, y, 58, 24, "UP", COLORS["warning"])
    button(s, 104, y, 24, 24, "<", COLORS["secondary"])
    button(s, 132, y, 24, 24, ">", COLORS["secondary"])
    s.text("CHD", 178, y + 16, 8, COLORS["dim"])
    button(s, 208, y, 48, 24, "DIA", COLORS["accent"])
    button(s, 264, y, 42, 24, "HOLD", COLORS["success"])
    y += 26
    s.text("OCT", 12, y + 16, 8, COLORS["dim"])
    button(s, 42, y, 24, 24, "-", COLORS["secondary"])
    s.text("2", 82, y + 16, 8, COLORS["text"], "middle", "700")
    button(s, 98, y, 24, 24, "+", COLORS["secondary"])
    s.text("RATE", 142, y + 16, 8, COLORS["dim"])
    button(s, 180, y, 24, 24, "-", COLORS["secondary"])
    s.text("1/8", 226, y + 16, 8, COLORS["text"], "middle", "700")
    button(s, 264, y, 24, 24, "+", COLORS["secondary"])
    y += 26
    s.text("BPM", 12, y + 16, 8, COLORS["dim"])
    button(s, 42, y, 24, 24, "-", COLORS["secondary"])
    s.text("120", 86, y + 16, 8, COLORS["text"], "middle", "700")
    button(s, 112, y, 24, 24, "+", COLORS["secondary"])
    s.text("GATE", 158, y + 16, 8, COLORS["dim"])
    button(s, 198, y, 24, 24, "-", COLORS["secondary"])
    s.text("70", 244, y + 16, 8, COLORS["text"], "middle", "700")
    button(s, 276, y, 24, 24, "+", COLORS["secondary"])
    y += 26
    s.text("ROOT", 12, y + 16, 8, COLORS["dim"])
    button(s, 50, y, 28, 24, "-", COLORS["secondary"])
    s.text("C", 100, y + 16, 8, COLORS["text"], "middle", "700")
    button(s, 124, y, 28, 24, "+", COLORS["secondary"])
    s.text("PAD", 174, y + 16, 8, COLORS["dim"])
    button(s, 206, y, 28, 24, "-", COLORS["secondary"])
    s.text("O4", 256, y + 16, 8, COLORS["text"], "middle", "700")
    button(s, 284, y, 24, 24, "+", COLORS["secondary"])
    s.rect(170, 132, 136, 18, COLORS["bg"], r=4)
    s.text("READY", 238, 145, 8, COLORS["dim"], "middle", "700")
    s.rect(6, 158, 308, 72, COLORS["panel"], COLORS["border"], 6)
    names = ["I", "ii", "iii", "IV", "V", "vi", "vii", "I+"]
    notes = ["C4", "D4", "E4", "F4", "G4", "A4", "B4", "C5"]
    cols = [COLORS["primary"], COLORS["secondary"], COLORS["accent"], COLORS["success"],
            COLORS["warning"], COLORS["error"], "#ff00ff", COLORS["primary"]]
    for i in range(8):
        x = 10 + i * 39
        s.rect(x, 164, 37, 58, COLORS["bg"], cols[i], 5)
        s.text(names[i], x + 18.5, 190, 12, COLORS["text"], "middle", "700")
        s.text(notes[i], x + 18.5, 211, 8, COLORS["dim"], "middle")
    return s.finish()


SCREENS = {
    "00-menu.svg": make_menu,
    "01-keys.svg": make_keys,
    "02-beats.svg": make_beats,
    "03-zen.svg": make_zen,
    "04-drop.svg": make_drop,
    "05-rng.svg": make_rng,
    "06-xy-pad.svg": make_xy,
    "07-arp.svg": make_arp,
    "08-grid.svg": make_grid,
    "09-chord.svg": make_chord,
    "10-lfo.svg": make_lfo,
}


def main():
    OUT.mkdir(parents=True, exist_ok=True)
    for name, maker in SCREENS.items():
        (OUT / name).write_text(maker(), encoding="utf-8")


if __name__ == "__main__":
    main()
