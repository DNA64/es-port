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
# ES's launch command. The wrappers hakchi uses for games call uistop/uistart, which terminate every process holding
# /dev/fb0, and ES keeps it open even with its window released. So the game does not run under ES: this records which one
# to run and closes ES; es-run.sh runs it and then opens ES again.
#
#   es-launch.sh <file.desktop>
echo "$1" > /tmp/es_next
kill $(pidof emulationstation)
