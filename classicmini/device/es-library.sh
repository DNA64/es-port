#!/bin/sh
#  Copyright (C) 2026 DefKorns (https://defkorns.github.io/LICENSE)
#
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <https://www.gnu.org/licenses/>.
#
source /etc/preinit
script_init

out=${1:?output dir}
store=$(findGameStorage) || { echo "no games storage" >&2; exit 1; }
max_depth=8

escXml() { sed -e 's/&/\&amp;/g' -e 's/</\&lt;/g' -e 's/>/\&gt;/g'; }
covers=/media/data/emulationstation/covers

findCover() {
  local cover alias slug
  [ -d "$covers" ] || return 0
  [ -f "$covers/$1.png" ] && { echo "$covers/$1.png"; return 0; }
  alias=$(tr -d '\r' < "$covers/aliases.txt" 2>/dev/null | awk -F'\t' -v code="$1" '$1 == code { print $2; exit }')
  if [ -n "$alias" ]; then
    cover=$(find "$covers" -maxdepth 1 -iname "$alias.png" | head -n 1)
    [ -n "$cover" ] && { echo "$cover"; return 0; }
  fi
  slug=$(echo "$2" | sed 's/([^)]*)//g; s/\[[^]]*\]//g' | tr '[:upper:]' '[:lower:]' | tr -cd 'a-z0-9')
  [ -n "$slug" ] && find "$covers" -maxdepth 1 -iname "$slug.png" | head -n 1
}

readField() { tr -d '\r' < "$1" | sed -n "s/^$2 *= *//p" | head -n 1; }
joinInfo() {
  local joined="" part
  for part in "$@"; do
    [ -n "$part" ] || continue
    joined=${joined:+$joined • }$part
  done
  echo "$joined"
}

playersLabel() {
  case $1 in
    "") ;;
    1) echo "1 Player" ;;
    *) echo "$1 Players" ;;
  esac
}

readDescription() {
  tr -d '\r' < "$1" | awk '/^\[Description\]/ { in_desc = 1; next } /^\[/ { in_desc = 0 }
    in_desc { sub(/^Text *= */, ""); if ($0 != "") { text = text sep $0; sep = " " } } END { print text }'
}

systemOf() {
  local exe rom ext
  exe=${1%% *}; exe=${exe##*/}
  rom=$(echo "$1" | awk '{ for (i = 2; i <= NF; i++) if ($i !~ /^-/) { print $i; exit } }')
  ext=$(echo "${rom##*/}" | tr '[:upper:]' '[:lower:]' | sed -E 's/\.(7z|zip|gz)$//; s/.*\.//')
  case $exe in
    clover-canoe-shvc*|snes|snes05*|snes9x*|snes10*|mednafen-snes-faust-xtreme) echo snes ;;
    nestopia|quicknes|fceumm*|kachikachi*|clover-kachikachi*|nes) [ "$ext" = fds ] && echo fds || echo nes ;;
    gambatte|gearboy|tgbdual|gb|gbc) [ "$ext" = gbc ] || [ "$exe" = gbc ] && echo gbc || echo gb ;;
    mgba|gpsp|gba) echo gba ;;
    melonds|desmume|drastic*|nds) echo nds ;;
    gg) echo gamegear ;;
    sms|smsplus|gearsystem) [ "$ext" = gg ] && echo gamegear || echo mastersystem ;;
    segacd) echo segacd ;;
    32x) echo sega32x ;;
    md) echo megadrive ;;
    genesis*|picodrive*)
      case $ext in
        sms) echo mastersystem ;;
        gg) echo gamegear ;;
        32x) echo sega32x ;;
        sg) echo sg-1000 ;;
        cue|chd|iso) echo segacd ;;
        *) echo megadrive ;;
      esac
      ;;
    mednafen-pce-fast|pce|pcecd|mednafen-supergrafx|sgfx) echo pcengine ;;
    mednafen-pcfx|mednafen_pcfx|pcfx) echo pcfx ;;
    mednafen-ngp|ngp|ngc) echo ngp ;;
    mednafen-wswan|mednafen_wswan|ws|wsc) echo wonderswan ;;
    mednafen-vb|mednafen_vb|vb) echo virtualboy ;;
    mednafen-psx|mednafen_psx|pcsx*|neon) echo psx ;;
    neocd|neo) echo neogeo ;;
    fbalpha*|fba*|fbneo*|cps1|cps2|cps3|mame*) echo arcade ;;
    n64|glupen64*|mupen64*|parallel-n64*|ludicrousn64*) echo n64 ;;
    ppsspp*) echo psp ;;
    yabause*) echo saturn ;;
    morpheuscast*|vemulator|vmu) echo dreamcast ;;
    opera-xtreme) echo 3do ;;
    stella|a26) echo atari2600 ;;
    prosystem|a78) echo atari7800 ;;
    a52) echo atari5200 ;;
    atari800) echo atari800 ;;
    handy|lnx) echo atarilynx ;;
    virtualjaguar|j64) echo atarijaguar ;;
    hatari*) echo atarist ;;
    vice*|c64|c128|vic20) echo c64 ;;
    puae*|uae4arm*|amiga) echo amiga ;;
    col) echo colecovision ;;
    fmsx|bluemsx|msx) echo msx ;;
    caprice32|cap32|crocods) echo amstradcpc ;;
    fuse|z80|zx) echo zxspectrum ;;
    freeintv|int) echo intellivision ;;
    freechaf) echo channelf ;;
    o2em|odyssey) echo videopac ;;
    vecx|vec) echo vectrex ;;
    gw|mgw) echo gameandwatch ;;
    pokemini|min|pokemon) echo pokemini ;;
    dosbox*) echo pc ;;
    scummvm) echo scummvm ;;
    np2*|nekop2|pc98) echo pc98 ;;
    px68k*|sharp) echo x68000 ;;
    quasi88|d88) echo pc88 ;;
    x1|x1-millennium) echo x1 ;;
    theodore) echo moto ;;
    tic80|tic) echo tic80 ;;
    openbor*) echo openbor ;;
    easyrpg|ldb|chai*|lutro|lua|prboom*|wad|nxengine|cavestory|ecwolf|tyrquake|pak|xrick|mrboom|jumpnbump|cannonball|reminiscence|flashback|2048|dinothawr|bk|eighty-one|81|emux*|ch8|uzem|uze|potator) echo ports ;;
    *) echo other ;;
  esac
}

buildMenu() {
  local menu="$1" depth="$2" trail="$3 $1" game_dir code desktop name cmd icon cover system sub desc year players released genre small
  for game_dir in "$store/$menu"/CLV-*; do
    code=${game_dir##*/}
    desktop=$game_dir/$code.desktop
    [ -f "$desktop" ] || continue

    cmd=$(readField "$desktop" Exec)
    case $cmd in
      "/bin/chmenu "*)
        sub=${cmd#/bin/chmenu }; sub=${sub%% *}
        case " $trail " in *" $sub "*) continue ;; esac
        [ "$depth" -lt "$max_depth" ] || continue
        [ -d "$store/$sub" ] || continue
        buildMenu "$sub" $((depth + 1)) "$trail"
        continue
        ;;
    esac

    system=$(systemOf "$cmd")
    [ -f "$out/$system/$code.desktop" ] && continue
    mkdir -p "$out/$system"
    cp "$desktop" "$out/$system/$code.desktop"

    name=$(readField "$desktop" Name)
    icon=$(readField "$desktop" Icon)
    case $icon in /var/games/*) icon=$store/$menu/${icon#/var/games/} ;; esac
    [ -f "$icon" ] || icon=
    cover=$(findCover "$code" "$name")
    [ -n "$cover" ] && icon=$cover

    {
      echo "  <game>"
      echo "    <path>./$code.desktop</path>"
      echo "    <name>$(echo "$name" | escXml)</name>"
      [ -n "$icon" ] && echo "    <image>$(echo "$icon" | escXml)</image>"
      desc=$(readDescription "$desktop")
      [ -n "$desc" ] && echo "    <desc>$(echo "$desc" | escXml)</desc>"
      released=$(readField "$desktop" ReleaseDate)
      year=$(echo "$released" | tr -d '-')
      players=$(readField "$desktop" Players)
      genre=$(readField "$desktop" Genre)
      [ -n "$genre" ] || genre=$(readField "$desktop" sortRawGenre)
      small=$game_dir/${code}_small.png
      [ ${#year} -eq 8 ] && echo "    <releasedate>${year}T000000</releasedate>"
      [ -n "$players" ] && echo "    <players>$players</players>"
      echo "    <genre>$(joinInfo "$genre" "${released%%-*}" "$(playersLabel "$players")" | escXml)</genre>"
      echo "    <developer>$code</developer>"
      [ -f "$small" ] || small=$icon
      [ -n "$cover" ] && small=$cover
      [ -n "$small" ] && echo "    <marquee>$(echo "$small" | escXml)</marquee>"
      echo "  </game>"
    } >> "$parts/$system.xml"
  done
}

parts=$(mktemp -d)
rm -rf "$out"
mkdir -p "$out"
buildMenu 000 0 ""
for part in "$parts"/*.xml; do
  [ -f "$part" ] || continue
  system=${part##*/}; system=${system%.xml}
  {
    echo '<?xml version="1.0"?>'
    echo '<gameList>'
    cat "$part"
    echo '</gameList>'
  } > "$out/$system/gamelist.xml"
done
rm -rf "$parts"
