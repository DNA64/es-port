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
gamelist=$out/gamelist.xml
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

buildMenu() {
  local menu="$1" dir="$2" depth="$3" trail="$4 $1" game_dir code desktop name cmd icon cover rel entry sub desc year players released genre small
  mkdir -p "$out/$dir"
  for game_dir in "$store/$menu"/CLV-*; do
    code=${game_dir##*/}
    desktop=$game_dir/$code.desktop
    [ -f "$desktop" ] || continue

    name=$(readField "$desktop" Name)
    cmd=$(readField "$desktop" Exec)
    icon=$(readField "$desktop" Icon)
    case $icon in /var/games/*) icon=$store/$menu/${icon#/var/games/} ;; esac
    [ -f "$icon" ] || icon=
    cover=$(findCover "$code" "$name")
    [ -n "$cover" ] && icon=$cover
    rel=./${dir:+$dir/}$code

    case $cmd in
      "/bin/chmenu "*)
        sub=${cmd#/bin/chmenu }; sub=${sub%% *}
        case " $trail " in *" $sub "*) continue ;; esac
        [ "$depth" -lt "$max_depth" ] || continue
        [ -d "$store/$sub" ] || continue
        buildMenu "$sub" "${dir:+$dir/}$code" $((depth + 1)) "$trail"
        entry=folder
        ;;
      *)
        cp "$desktop" "$out/${dir:+$dir/}$code.desktop"
        rel=$rel.desktop
        entry=game
        ;;
    esac

    {
      echo "  <$entry>"
      echo "    <path>$(echo "$rel" | escXml)</path>"
      echo "    <name>$(echo "$name" | escXml)</name>"
      [ -n "$icon" ] && echo "    <image>$(echo "$icon" | escXml)</image>"
      desc=$(readDescription "$desktop")
      [ -n "$desc" ] && echo "    <desc>$(echo "$desc" | escXml)</desc>"
      if [ "$entry" = game ]; then
        released=$(readField "$desktop" ReleaseDate)
        year=$(echo "$released" | tr -d '-')
        players=$(readField "$desktop" Players)
        genre=$(readField "$desktop" Genre)
        [ -n "$genre" ] || genre=$(readField "$desktop" sortRawGenre)
        small=$game_dir/${code}_small.png
        [ ${#year} -eq 8 ] && echo "    <releasedate>${year}T000000</releasedate>"
        [ -n "$players" ] && echo "    <players>$players</players>"
        echo "    <genre>$(joinInfo "$genre" "${released%%-*}" "$(playersLabel "$players")" | escXml)</genre>"
        [ -f "$small" ] || small=$icon
        [ -n "$cover" ] && small=$cover
        [ -n "$small" ] && echo "    <marquee>$(echo "$small" | escXml)</marquee>"
      fi
      echo "  </$entry>"
    } >> "$gamelist"
  done
}

rm -rf "$out"
mkdir -p "$out"
echo '<?xml version="1.0"?>' > "$gamelist"
echo '<gameList>' >> "$gamelist"
buildMenu 000 "" 0 ""
echo '</gameList>' >> "$gamelist"
