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
es_dir=/var/lib/hakchi/es

drawSplash(){
  decodepng "$es_dir/resources/$1.png" > /dev/fb0
}

exec 3< /dev/input/event2 || exit 1
while ev=$(dd bs=16 count=1 <&3 2>/dev/null | od -A n -t x1 -w16); do
  [ -n "$ev" ] || exit 0
  case $ev in
    *" 01 00 74 00 00 00 00 00")
      touch /tmp/es-stopping
      killall emulationstation 2>/dev/null
      usleep 300000
      drawSplash shutdown
      poweroff
      exit 0 ;;
  esac
done
