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
here=/var/lib/hakchi/es
source /etc/preinit
script_init

trap uistart EXIT
uistop
sh "$here/es-library.sh" /tmp/es-library
while true; do
  rm -f /tmp/es_next
  HOME=$here/home LD_LIBRARY_PATH=$here "$here/emulationstation" "$@"
  [ -f /tmp/es_next ] || break
  sh "$here/es-game.sh" "$(cat /tmp/es_next)"
  if [ -n "$(pidof clover-mcp)" ]; then
    for _ in $(seq 1 25); do
      [ -n "$(pidof ReedPlayer-Clover)" ] && break
      usleep 200000
    done
    uistop
  fi
done
