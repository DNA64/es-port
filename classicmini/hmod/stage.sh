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
set -e
script_dir=$(cd "$(dirname "$0")" && pwd)
classicmini_dir=$script_dir/..
mod_dir=$script_dir/mod
es_payload=$mod_dir/usr/share/emulationstation

[ -f "$classicmini_dir/out/emulationstation" ] || { echo "Build EmulationStation first (classicmini/build.sh)." >&2; exit 1; }
[ -f "$classicmini_dir/device/resources/cacert.pem" ] || curl -fsSL -o "$classicmini_dir/device/resources/cacert.pem" https://curl.se/ca/cacert.pem

rm -rf "$mod_dir"
mkdir -p "$mod_dir/etc/init.d" "$mod_dir/etc/preinit.d" "$es_payload/defaults" "$es_payload/resources"
cp "$classicmini_dir/device/S8100es" "$mod_dir/etc/init.d/"
cp "$classicmini_dir/device/preinit/b8030_es" "$mod_dir/etc/preinit.d/"
cp "$classicmini_dir/out/emulationstation" "$classicmini_dir/out/libfreetype.so.6" "$classicmini_dir/out/padstate" "$es_payload/"
cp "$classicmini_dir"/device/es-*.sh "$classicmini_dir/device/es_systems.cfg" "$es_payload/"
cp -R "$classicmini_dir"/../resources/. "$es_payload/resources/"
rm -f "$es_payload"/resources/mame*.xml "$es_payload"/resources/*.py "$es_payload"/resources/splash.svg
cp -R "$classicmini_dir"/device/resources/. "$es_payload/resources/"
find "$es_payload/resources" -type f \( -name '*.svg' -o -name '*.xml' -o -name '*.txt' \) -exec sed -i 's/\r$//' {} +
cp "$classicmini_dir/device/es_settings.cfg" "$classicmini_dir/device/es_input.cfg" "$es_payload/defaults/"
cp "$script_dir"/files/install "$script_dir"/files/uninstall "$script_dir"/files/readme.md "$mod_dir/"
