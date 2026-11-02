#!/bin/sh
# Builds the tree EmulationStation browses from the games hakchi synced: one directory per hakchi menu (folder),
# one <CODE>.desktop per game, and a gamelist.xml with names, icons and descriptions.
#
#   es-library.sh <output dir>
#
# Reads the same storage the stock UI does (findGameStorage): <storage>/000 is the root menu, and an entry whose Exec is
# "/bin/chmenu NNN ..." is a folder that opens <storage>/NNN.
source /etc/preinit
script_init

out=${1:?output dir}
store=$(findGameStorage) || { echo "no games storage" >&2; exit 1; }
gamelist=$out/gamelist.xml
profiles=/var/lib/clover/profiles/0
max_depth=8

esc() { sed -e 's/&/\&amp;/g' -e 's/</\&lt;/g' -e 's/>/\&gt;/g'; }
field() { tr -d '\r' < "$1" | sed -n "s/^$2 *= *//p" | head -n 1; }
text() { tr -d '\r' < "$1" | sed -n '/^\[Description\]/,$ s/^Text *= *//p' | head -n 1; }

# $1 menu code, $2 directory under $out, $3 depth, $4 menus on the way here; the "back" entry hakchi puts in a folder
# opens a menu that is already on that path, ES has its own way back
build_menu() {
  local menu=$1 dir=$2 depth=$3 trail="$4 $1" d code desktop name cmd icon rel entry sub desc year players sp n
  mkdir -p "$out/$dir"
  for d in "$store/$menu"/CLV-*; do
    code=${d##*/}
    desktop=$d/$code.desktop
    [ -f "$desktop" ] || continue

    name=$(field "$desktop" Name)
    cmd=$(field "$desktop" Exec)
    icon=$(field "$desktop" Icon)
    case $icon in /var/games/*) icon=$store/$menu/${icon#/var/games/} ;; esac
    [ -f "$icon" ] || icon=
    rel=./${dir:+$dir/}$code

    case $cmd in
      "/bin/chmenu "*)
        sub=${cmd#/bin/chmenu }; sub=${sub%% *}
        case " $trail " in *" $sub "*) continue ;; esac
        [ "$depth" -lt "$max_depth" ] && [ -d "$store/$sub" ] || continue
        build_menu "$sub" "${dir:+$dir/}$code" $((depth + 1)) "$trail"
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
      echo "    <path>$(echo "$rel" | esc)</path>"
      echo "    <name>$(echo "$name" | esc)</name>"
      [ -n "$icon" ] && echo "    <image>$(echo "$icon" | esc)</image>"
      desc=$(text "$desktop")
      [ -n "$desc" ] && echo "    <desc>$(echo "$desc" | esc)</desc>"
      if [ "$entry" = game ]; then
        year=$(field "$desktop" ReleaseDate | tr -d '-')
        players=$(field "$desktop" Players)
        [ ${#year} -eq 8 ] && echo "    <releasedate>${year}T000000</releasedate>"
        [ -n "$players" ] && echo "    <players>$players</players>"
      fi
      echo "  </$entry>"
    } >> "$gamelist"

    # a suspend point is an entry of its own, with the screenshot the stock UI saved with it
    [ "$entry" = game ] || continue
    for sp in "$profiles/$code"/suspendpoint[1-9]; do
      [ -d "$sp" ] || continue
      n=${sp##*suspendpoint}
      { cat "$desktop"; printf '\nESResume=%s\n' "$n"; } > "$out/${dir:+$dir/}$code.sp$n.desktop"
      {
        echo "  <game>"
        echo "    <path>$(echo "${rel%.desktop}.sp$n.desktop" | esc)</path>"
        echo "    <name>$(echo "$name (suspend point $n)" | esc)</name>"
        [ -f "$sp/state.png" ] && echo "    <image>$(echo "$sp/state.png" | esc)</image>"
        echo "  </game>"
      } >> "$gamelist"
    done
  done
}

rm -rf "$out"
mkdir -p "$out"
echo '<?xml version="1.0"?>' > "$gamelist"
echo '<gameList>' >> "$gamelist"
build_menu 000 "" 0 ""
echo '</gameList>' >> "$gamelist"
