import argparse
import os
import shutil

from PIL import Image, ImageDraw, ImageFont

SYSTEMS = [
    ("snes", "Super Nintendo", "snes", None),
    ("nes", "Nintendo (NES)", "nes", "NES"),
    ("fds", "Famicom Disk System", "fds", "FAMICOM DISK"),
    ("gb", "Game Boy", "gb", "GAME BOY"),
    ("gbc", "Game Boy Color", "gbc", "GAME BOY COLOR"),
    ("gba", "Game Boy Advance", "gba", "GAME BOY ADVANCE"),
    ("nds", "Nintendo DS", "nds", "NINTENDO DS"),
    ("n64", "Nintendo 64", "n64", "NINTENDO 64"),
    ("virtualboy", "Virtual Boy", "virtualboy", "VIRTUAL BOY"),
    ("pokemini", "Pokemon Mini", "pokemini", "POKEMON MINI"),
    ("megadrive", "Sega Mega Drive", "megadrive,genesis", "MEGA DRIVE"),
    ("sega32x", "Sega 32X", "sega32x", "32X"),
    ("segacd", "Sega CD", "segacd", "SEGA CD"),
    ("mastersystem", "Sega Master System", "mastersystem", "MASTER SYSTEM"),
    ("gamegear", "Sega Game Gear", "gamegear", "GAME GEAR"),
    ("sg-1000", "Sega SG-1000", "sg-1000", "SG-1000"),
    ("saturn", "Sega Saturn", "saturn", "SATURN"),
    ("dreamcast", "Sega Dreamcast", "dreamcast", "DREAMCAST"),
    ("pcengine", "PC Engine", "pcengine", "PC ENGINE"),
    ("pcfx", "PC-FX", "pcfx", "PC-FX"),
    ("pc98", "NEC PC-98", "pc98", "PC-98"),
    ("pc88", "NEC PC-88", "pc88", "PC-88"),
    ("x68000", "Sharp X68000", "x68000", "X68000"),
    ("x1", "Sharp X1", "x1", "SHARP X1"),
    ("neogeo", "Neo Geo", "neogeo", "NEO GEO"),
    ("ngp", "Neo Geo Pocket", "ngp,ngpc", "NEO GEO POCKET"),
    ("psx", "PlayStation", "psx", "PLAYSTATION"),
    ("psp", "PlayStation Portable", "psp", "PSP"),
    ("atari2600", "Atari 2600", "atari2600", "ATARI 2600"),
    ("atari5200", "Atari 5200", "atari5200", "ATARI 5200"),
    ("atari7800", "Atari 7800", "atari7800", "ATARI 7800"),
    ("atari800", "Atari 800", "atari800", "ATARI 800"),
    ("atarilynx", "Atari Lynx", "atarilynx", "ATARI LYNX"),
    ("atarijaguar", "Atari Jaguar", "atarijaguar", "ATARI JAGUAR"),
    ("atarist", "Atari ST", "atarist", "ATARI ST"),
    ("3do", "3DO", "3do", "3DO"),
    ("amiga", "Amiga", "amiga", "AMIGA"),
    ("c64", "Commodore 64", "c64", "COMMODORE 64"),
    ("msx", "MSX", "msx", "MSX"),
    ("amstradcpc", "Amstrad CPC", "amstradcpc", "AMSTRAD CPC"),
    ("zxspectrum", "ZX Spectrum", "zxspectrum", "ZX SPECTRUM"),
    ("colecovision", "ColecoVision", "colecovision", "COLECOVISION"),
    ("intellivision", "Intellivision", "intellivision", "INTELLIVISION"),
    ("channelf", "Fairchild Channel F", "channelf", "CHANNEL F"),
    ("videopac", "Odyssey 2", "videopac", "ODYSSEY 2"),
    ("vectrex", "Vectrex", "vectrex", "VECTREX"),
    ("wonderswan", "WonderSwan", "wonderswan,wonderswancolor", "WONDERSWAN"),
    ("gameandwatch", "Game & Watch", "gameandwatch", "GAME & WATCH"),
    ("arcade", "Arcade", "arcade", "ARCADE"),
    ("pc", "DOS", "pc", "DOS"),
    ("scummvm", "ScummVM", "scummvm", "SCUMMVM"),
    ("moto", "Thomson", "moto", "THOMSON"),
    ("tic80", "TIC-80", "tic80", "TIC-80"),
    ("openbor", "OpenBOR", "openbor", "OPENBOR"),
    ("ports", "Ports", "ignore", "PORTS"),
    ("other", "Other", "snes,nes,genesis,megadrive", "OTHER"),
]

THEME = """<theme>
\t<formatVersion>4</formatVersion>
\t<include>../theme.xml</include>

\t<view name="system">
\t\t<image name="logo">
\t\t\t<path>{logo}</path>
\t\t</image>
\t</view>
\t<view name="grid">
\t\t<image name="logo">
\t\t\t<path>{logo}</path>
\t\t</image>
\t</view>
</theme>
"""

LOGO_WIDTH = 640
LOGO_HEIGHT = 203
LOGO_MARGIN = 40
LOGO_COLOR = (200, 200, 217, 255)


def drawLogo(label, font_path, target):
    size = 96
    while size > 20:
        font = ImageFont.truetype(font_path, size)
        box = font.getbbox(label)
        if box[2] - box[0] <= LOGO_WIDTH - 2 * LOGO_MARGIN:
            break
        size -= 2
    image = Image.new("RGBA", (LOGO_WIDTH, LOGO_HEIGHT), (0, 0, 0, 0))
    draw = ImageDraw.Draw(image)
    left, top, right, bottom = draw.textbbox((0, 0), label, font=font)
    draw.text(((LOGO_WIDTH - (right - left)) / 2 - left, (LOGO_HEIGHT - (bottom - top)) / 2 - top), label, font=font, fill=LOGO_COLOR)
    image.save(target, "PNG", optimize=True)


def main():
    parser = argparse.ArgumentParser(description="Generate es_systems.cfg and the per-system themes.")
    parser.add_argument("device", help="path to classicmini/device")
    args = parser.parse_args()

    theme_dir = os.path.join(args.device, "theme", "games")
    font_path = os.path.join(theme_dir, "assets", "fonts", "NotoSans-SemiBold.ttf")

    lines = ["<systemList>"]
    for slug, full, platform, label in SYSTEMS:
        lines += [
            "  <system>", "    <name>%s</name>" % slug, "    <fullname>%s</fullname>" % full.replace("&", "&amp;"),
            "    <path>/tmp/es-library/%s</path>" % slug, "    <extension>.desktop</extension>",
            "    <command>/var/lib/hakchi/emulationstation/es-launch.sh %ROM%</command>", "    <platform>%s</platform>" % platform,
            "    <theme>%s</theme>" % slug, "  </system>"]
        folder = os.path.join(theme_dir, slug)
        os.makedirs(folder, exist_ok=True)
        logo = "./logo.png"
        if label is None:
            shutil.copyfile(os.path.join(theme_dir, "assets", "logo_snes.png"), os.path.join(folder, "logo.png"))
        else:
            drawLogo(label, font_path, os.path.join(folder, "logo.png"))
        with open(os.path.join(folder, "theme.xml"), "w", encoding="utf-8", newline="\n") as handle:
            handle.write(THEME.format(logo=logo))
    lines.append("</systemList>")
    with open(os.path.join(args.device, "es_systems.cfg"), "w", encoding="utf-8", newline="\n") as handle:
        handle.write("\n".join(lines) + "\n")


if __name__ == "__main__":
    main()
