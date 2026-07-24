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
    header(s, "KEYS", "Major Key C-1")
    for row in range(2):
        for key in range(10):
            x, y, w, h = key * 32, 52 + row * 60, 29, 56
            fill = COLORS["surface"] if row == 0 else COLORS["panel"]
            s.rect(x + 1, y + 1, w, h, fill, COLORS["border"], 4)
            s.text(["C3", "D3", "E3", "F3", "G3", "A3", "B3", "C4", "D4", "E4"][key], x + 16, y + 28, 10, COLORS["text"], "middle", "700")
            s.text("LOW" if row == 0 else "HIGH", x + 16, y + 47, 6, COLORS["dim"], "middle")
    for args in [(8,178,46,30,"OCT-",COLORS["secondary"]), (60,178,46,30,"OCT+",COLORS["secondary"]), (116,178,66,30,"SCALE",COLORS["accent"]), (190,178,46,30,"KEY-",COLORS["warning"]), (242,178,46,30,"KEY+",COLORS["warning"])]:
        button(s, *args)
    s.rect(8, 214, 304, 18, COLORS["panel"], r=4)
    s.text("Oct 4  Major  Root C-1", 160, 227, 9, COLORS["dim"], "middle")
    return s.finish()


def make_beats():
    s = SVG("BEATS")
    header(s, "BEATS", "FOUR  120 BPM")
    s.rect(6, 46, 308, 114, COLORS["panel"], COLORS["border"], 6)
    labels = ["KCK", "SNR", "HAT", "OPN"]
    cols = [COLORS["error"], COLORS["warning"], COLORS["primary"], COLORS["accent"]]
    pattern = [
        {0: 2, 4: 2, 8: 2, 12: 2},
        {4: 2, 12: 2},
        {2: 1, 6: 1, 10: 1, 14: 1},
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
            s.rect(x, y, 15, 24, fill, stroke, 3)
            if level == 2:
                s.circle(x + 7.5, y + 12, 3, cols[tr])
    for args in [
        (8, 172, 50, 24, "PLAY", COLORS["success"]),
        (64, 172, 44, 24, "PAT", COLORS["accent"]),
        (114, 172, 44, 24, "FILL", COLORS["warning"]),
        (164, 172, 44, 24, "CLR", COLORS["error"]),
        (218, 172, 28, 24, "SW-", COLORS["secondary"]),
        (288, 172, 24, 24, "+", COLORS["secondary"]),
        (8, 206, 46, 26, "BPM-", COLORS["secondary"]),
        (128, 206, 46, 26, "BPM+", COLORS["secondary"]),
    ]:
        button(s, *args)
    s.rect(252, 172, 30, 24, COLORS["panel"], COLORS["border"], 5)
    s.text("50", 267, 188, 8, COLORS["dim"], "middle", "700")
    s.rect(62, 206, 58, 26, COLORS["panel"], COLORS["border"], 5)
    s.text("120", 91, 224, 12, COLORS["text"], "middle", "700")
    s.rect(188, 206, 124, 26, COLORS["panel"], COLORS["border"], 5)
    s.text("CH 10  FOUR", 250, 224, 10, COLORS["dim"], "middle", "700")
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
    header(s, "CHORD MODE", "Major Diatonic")
    s.rect(6, 58, 308, 112, COLORS["panel"], COLORS["border"], 6)
    names = ["I", "ii", "iii", "IV", "V", "vi", "vii°", "I+"]
    roots = ["C3", "D3", "E3", "F3", "G3", "A3", "B3", "C4"]
    cols = [COLORS["primary"], COLORS["secondary"], COLORS["accent"], COLORS["success"], COLORS["warning"], COLORS["error"], "#ff00ff", "#00ff00"]
    for i in range(8):
        x = 10 + i * 39
        s.rect(x, 66, 37, 96, COLORS["bg"], cols[i], 5)
        s.text(names[i], x + 18.5, 103, 18, COLORS["text"], "middle", "700")
        s.text(roots[i], x + 18.5, 132, 10, COLORS["dim"], "middle")
    for args in [(8,184,46,30,"OCT-",COLORS["secondary"]), (60,184,46,30,"OCT+",COLORS["secondary"]), (122,184,68,30,"SCALE",COLORS["accent"]), (202,184,62,30,"CLEAR",COLORS["error"])]:
        button(s, *args)
    s.rect(8, 218, 304, 16, COLORS["panel"], r=4)
    s.text("Oct 4  Classic piano chords", 160, 230, 8, COLORS["dim"], "middle")
    return s.finish()


def make_grid():
    s = SVG("GRID")
    header(s, "GRID PIANO", "4ths Layout")
    s.rect(6, 50, 308, 146, COLORS["panel"], COLORS["border"], 6)
    notes = ["C", "C#", "D", "D#", "E", "F", "F#", "G"]
    for r in range(5):
        for c in range(8):
            x, y = 13 + c * 38, 60 + r * 28
            black = "#" in notes[c]
            s.rect(x, y, 35, 25, COLORS["bg"] if black else COLORS["surface"], COLORS["border"], 4)
            s.text(notes[c], x + 17.5, y + 16, 8, COLORS["dim"] if black else COLORS["text"], "middle")
    button(s, 8, 204, 48, 30, "OCT-", COLORS["secondary"])
    button(s, 64, 204, 48, 30, "OCT+", COLORS["secondary"])
    s.rect(126, 204, 60, 30, COLORS["panel"], r=5)
    s.text("Oct 3", 156, 224, 11, COLORS["text"], "middle", "700")
    s.rect(198, 204, 114, 30, COLORS["panel"], r=5)
    s.text("Ready", 255, 224, 11, COLORS["dim"], "middle")
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
    header(s, "RNG JAMS", "Random Music")
    s.rect(6, 50, 308, 178, COLORS["panel"], COLORS["border"], 6)
    button(s, 14, 56, 62, 30, "PLAY", COLORS["success"])
    s.text("KEY", 88, 64, 8, COLORS["dim"])
    button(s, 116, 56, 42, 30, "C4", COLORS["primary"])
    button(s, 164, 56, 30, 30, "+", COLORS["secondary"])
    button(s, 200, 56, 30, 30, "-", COLORS["secondary"])
    button(s, 238, 56, 66, 30, "Major", COLORS["accent"])
    rows = [("OCT", "3-6", 90), ("CHANCE", "50%", 124), ("BPM", "120", 158)]
    for lab, val, y in rows:
        s.text(lab, 16, y + 8, 8, COLORS["dim"])
        s.text(val, 16, y + 23, 12, COLORS["text"], weight="700")
    s.rect(160, 133, 144, 14, COLORS["bg"], COLORS["border"], 3)
    s.rect(161, 134, 70, 12, COLORS["primary"], r=2)
    s.rect(14, 192, 290, 28, COLORS["bg"], r=5)
    s.text("Idle", 160, 211, 12, COLORS["dim"], "middle")
    return s.finish()


def make_zen():
    s = SVG("ZEN")
    header(s, "ZEN", "Ambient Bouncing")
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
    s.text("C-1 Major  Oct 4  Balls 2", 160, 198, 8, COLORS["dim"], "middle")
    for args in [(8,202,44,30,"ADD",COLORS["success"]), (58,202,54,30,"RESET",COLORS["warning"]), (120,202,58,30,"SCALE",COLORS["accent"]), (186,202,42,30,"KEY-",COLORS["secondary"]), (234,202,42,30,"KEY+",COLORS["secondary"]), (282,202,30,30,"O",COLORS["primary"])]:
        button(s, *args)
    return s.finish()


def make_drop():
    s = SVG("DROP")
    header(s, "DROP", "Tap to Drop")
    s.rect(6, 50, 308, 134, COLORS["panel"], COLORS["border"], 6)
    s.rect(80, 160, 60, 8, COLORS["primary"], COLORS["border"])
    s.rect(180, 140, 50, 8, COLORS["secondary"], COLORS["border"])
    s.rect(120, 120, 40, 8, COLORS["accent"], COLORS["border"])
    s.circle(110, 86, 5, "#7bcfff", COLORS["text"])
    s.circle(170, 105, 4, "#ffcf5a", COLORS["text"])
    s.rect(8, 187, 304, 12, COLORS["panel"], r=3)
    s.text("C-1 Major  Oct 4  Balls 2", 160, 197, 8, COLORS["dim"], "middle")
    for args in [(8,202,48,30,"EDIT",COLORS["warning"]), (62,202,58,30,"CLEAR",COLORS["error"]), (128,202,58,30,"SCALE",COLORS["accent"]), (194,202,42,30,"KEY-",COLORS["secondary"]), (242,202,42,30,"KEY+",COLORS["secondary"]), (290,202,22,30,"O",COLORS["primary"])]:
        button(s, *args)
    return s.finish()


def make_arp():
    s = SVG("ARP")
    header(s, "ARPEGGIATOR", "Piano Chord Arps")
    s.rect(6, 50, 308, 104, COLORS["panel"], COLORS["border"], 6)
    s.text("Pattern:", 10, 71, 8, COLORS["dim"])
    button(s, 65, 55, 60, 25, "UP", COLORS["warning"])
    button(s, 130, 55, 25, 25, "<", COLORS["secondary"])
    button(s, 160, 55, 25, 25, ">", COLORS["secondary"])
    s.text("Type:", 200, 71, 8, COLORS["dim"])
    button(s, 240, 55, 50, 25, "MAJ", COLORS["accent"])
    s.text("Octaves: 2", 10, 96, 8, COLORS["dim"])
    button(s, 90, 80, 25, 25, "-", COLORS["secondary"])
    button(s, 120, 80, 25, 25, "+", COLORS["secondary"])
    s.text("Speed: 8th", 160, 96, 8, COLORS["dim"])
    button(s, 240, 80, 25, 25, "+", COLORS["secondary"])
    button(s, 270, 80, 25, 25, "-", COLORS["secondary"])
    s.text("BPM: 120", 10, 121, 8, COLORS["dim"])
    s.rect(4, 154, 312, 59, COLORS["panel"], COLORS["border"], 6)
    for i in range(12):
        x = i * 26
        sharp = i in [1, 3, 6, 8, 10]
        s.rect(x + 1, 161, 24, 43, COLORS["bg"] if sharp else COLORS["surface"], COLORS["border"], 4)
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
