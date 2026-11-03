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
library=${1:?library dir}
favorites=${2:?favorites file}

[ -s "$favorites" ] || exit 0

for gamelist in "$library"/*/gamelist.xml; do
  [ -f "$gamelist" ] || continue
  awk -v favorites="$favorites" '
    BEGIN { while ((getline code < favorites) > 0) wanted[code] = 1 }
    /<game>/ { in_game = 1; marked = 0; has_favorite = 0 }
    in_game && /<path>/ {
      code = $0
      sub(/.*<path>\.\//, "", code)
      sub(/\.desktop<\/path>.*/, "", code)
      marked = (code in wanted)
    }
    in_game && /<favorite>/ { has_favorite = 1 }
    in_game && /<\/game>/ {
      in_game = 0
      if (marked && !has_favorite) print "    <favorite>true</favorite>"
    }
    { print }
  ' "$gamelist" > "$gamelist.tmp" && mv "$gamelist.tmp" "$gamelist"
done
