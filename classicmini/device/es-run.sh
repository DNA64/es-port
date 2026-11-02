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

restore_ui=1
trap '[ "$restore_ui" = 1 ] && { type stockUistart >/dev/null 2>&1 && stockUistart || uistart; }' EXIT
uistop
sh "$es_dir/es-library.sh" /tmp/es-library
while true; do
  rm -f /tmp/es_next /tmp/es_resume /tmp/es-quit /tmp/es-restart /tmp/es-sysrestart /tmp/es-shutdown
  es_started=$(date +%s)
  HOME=$es_dir/home LD_LIBRARY_PATH=$es_dir "$es_dir/emulationstation" "$@"
  es_status=$?
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
  if [ ! -f /tmp/es_next ]; then
    if [ -f /tmp/es-sysrestart ] || [ -f /tmp/es-shutdown ] || { [ "$es_status" -eq 0 ] && [ ! -f /tmp/es-quit ]; }; then
      restore_ui=0
    fi
    break
  fi
  sh "$es_dir/es-game.sh" "$(cat /tmp/es_next)" "$(cat /tmp/es_resume 2>/dev/null)"
  if [ -n "$(pidof clover-mcp)" ]; then
    for _ in $(seq 1 25); do
      [ -n "$(pidof ReedPlayer-Clover)" ] && break
      usleep 200000
    done
    uistop
  fi
done
