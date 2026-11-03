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
es_dir=/var/lib/hakchi/es
source /etc/preinit
script_init

export ES_PAD=snes
[ "$sftype" = snes ] || ES_PAD=nes

drawSplash(){
  decodepng "$es_dir/resources/$1.png" > /dev/fb0
}

watchPowerSwitch(){
  exec 3< /dev/input/event2 || return
  while ev=$(dd bs=16 count=1 <&3 2>/dev/null | od -A n -t x1 -w16); do
    [ -n "$ev" ] || return
    case $ev in
      *" 01 00 74 00 00 00 00 00")
        touch /tmp/es-stopping
        killall emulationstation 2>/dev/null
        usleep 300000
        drawSplash shutdown
        poweroff
        return ;;
    esac
  done
}

restore_ui=1
trap '[ -n "$power_pid" ] && [ ! -f /tmp/es-stopping ] && kill "$power_pid" 2>/dev/null; [ "$restore_ui" = 1 ] && [ ! -f /tmp/es-stopping ] && { type stockUistart >/dev/null 2>&1 && stockUistart || uistart; }' EXIT
uistop
watchPowerSwitch &
power_pid=$!
library_stamp=$es_dir/library.stamp
games_dir=$(findGameStorage)
games_count=$(find "$games_dir" -name '*.desktop' | wc -l)
if [ -d "$es_dir/library" ] && [ "$(cat "$library_stamp.count" 2>/dev/null)" = "$games_count" ]   && [ -z "$(find "$games_dir" /media/data/emulationstation/covers "$es_dir/es-library.sh" \( -name '*.desktop' -o -name '*.png' -o -name 'aliases.txt' -o -name 'es-library.sh' \) -newer "$library_stamp" 2>/dev/null | head -n 1)" ]; then
  rm -rf /tmp/es-library
  cp -a "$es_dir/library" /tmp/es-library
else
  sh "$es_dir/es-library.sh" /tmp/es-library
  rm -rf "$es_dir/library"
  cp -a /tmp/es-library "$es_dir/library"
  echo "$games_count" > "$library_stamp.count"
  touch "$library_stamp"
fi
[ -f /media/data/emulationstation/audio/bgm_home.wav ] || sh "$es_dir/es-tools.sh" audio import >/dev/null 2>&1
while true; do
  rm -f /tmp/es-quit /tmp/es-restart /tmp/es-sysrestart /tmp/es-shutdown
  es_started=$(date +%s)
  HOME=$es_dir/home LD_LIBRARY_PATH=$es_dir "$es_dir/emulationstation" "$@"
  es_status=$?
  case $es_status in 129|143) restore_ui=0; break ;; esac
  if [ "$es_status" -ne 0 ] && [ $(($(date +%s) - es_started)) -lt 15 ]; then
    es_failures=$(($(cat "$es_dir/boot_failures" 2>/dev/null || echo 0) + 1))
    echo "$es_failures" > "$es_dir/boot_failures"
    if [ "$es_failures" -ge 2 ]; then
      echo stock > "$es_dir/boot_default"
      break
    fi
    continue
  fi
  rm -f "$es_dir/boot_failures"
  [ -f /tmp/es-restart ] && continue
  if [ -f /tmp/es-sysrestart ] || [ -f /tmp/es-shutdown ] || { [ "$es_status" -eq 0 ] && [ ! -f /tmp/es-quit ]; }; then
    restore_ui=0
  fi
  if [ -f /tmp/es-sysrestart ]; then
    touch /tmp/es-restarting
    drawSplash restart
    reboot
  elif [ -f /tmp/es-shutdown ]; then
    drawSplash shutdown
    poweroff
  fi
  break
done
