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

trap uistart EXIT
uistop
sh "$es_dir/es-library.sh" /tmp/es-library
while true; do
  rm -f /tmp/es_next /tmp/es_resume
  HOME=$es_dir/home LD_LIBRARY_PATH=$es_dir "$es_dir/emulationstation" "$@"
  [ -f /tmp/es_next ] || break
  sh "$es_dir/es-game.sh" "$(cat /tmp/es_next)" "$(cat /tmp/es_resume 2>/dev/null)"
  if [ -n "$(pidof clover-mcp)" ]; then
    for _ in $(seq 1 25); do
      [ -n "$(pidof ReedPlayer-Clover)" ] && break
      usleep 200000
    done
    uistop
  fi
done
