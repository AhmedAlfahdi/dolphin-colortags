#!/usr/bin/env python3
"""Generate the hicolor emblem icons used by the Dolphin overlay plugin.

Run this after changing the palette in src/colortag.cpp:

    ./scripts/generate-icons.py

The icons are committed to the repository so a build does not need Python.
"""

from pathlib import Path

# Keep in sync with ColorTag::palette() in src/colortag.cpp.
PALETTE = {
    "red": "#ff0000",
    "orange": "#ffa500",
    "yellow": "#ffff00",
    "green": "#008000",
    "blue": "#0000ff",
    "purple": "#800080",
    "gray": "#b3b3b3",
}

OUTPUT = Path(__file__).resolve().parent.parent / "icons"


def badge(color: str) -> str:
    return f"""<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 32 32" width="32" height="32">
  <circle cx="16" cy="16" r="13" fill="{color}"/>
  <circle cx="16" cy="16" r="13" fill="none" stroke="#000000" stroke-opacity="0.45" stroke-width="2.5"/>
  <ellipse cx="11.5" cy="10.5" rx="4" ry="3" fill="#ffffff" fill-opacity="0.3"/>
</svg>
"""


NO_COLOR = """<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 32 32" width="32" height="32">
  <circle cx="16" cy="16" r="13" fill="#101010" fill-opacity="0.35"/>
  <circle cx="16" cy="16" r="12" fill="none" stroke="#f0f0f0" stroke-width="2.5"
          stroke-dasharray="2.6 3.2" stroke-linecap="round"/>
</svg>
"""

PALETTE_ICON = """<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 32 32" width="32" height="32">
  <rect x="3" y="3" width="26" height="26" rx="7" fill="#3a3a3a" fill-opacity="0.35"
        stroke="#888888" stroke-opacity="0.7" stroke-width="1.6"/>
  <circle cx="12" cy="12" r="4.2" fill="#ff0000"/>
  <circle cx="21" cy="12" r="4.2" fill="#ffd400"/>
  <circle cx="12" cy="21" r="4.2" fill="#008000"/>
  <circle cx="21" cy="21" r="4.2" fill="#0060ff"/>
</svg>
"""


def main() -> None:
    OUTPUT.mkdir(parents=True, exist_ok=True)
    for name, color in PALETTE.items():
        (OUTPUT / f"colortag-{name}.svg").write_text(badge(color))
    (OUTPUT / "colortag-none.svg").write_text(NO_COLOR)
    (OUTPUT / "colortag.svg").write_text(PALETTE_ICON)
    print(f"Wrote {len(PALETTE) + 2} icons to {OUTPUT}")


if __name__ == "__main__":
    main()
