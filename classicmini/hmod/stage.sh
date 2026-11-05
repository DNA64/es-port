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

theme_rev=5a6c1daf965b9d410398243c232a34911dad8826
theme_src=$classicmini_dir/device/themes/simple
if [ ! -f "$theme_src/.rev_$theme_rev" ]; then
  rm -rf "$theme_src"
  mkdir -p "$theme_src"
  curl -fsSL "https://github.com/RetroPie/es-theme-simple/archive/$theme_rev.tar.gz" | tar -xz -C "$theme_src" --strip-components=1
  : > "$theme_src/.rev_$theme_rev"
fi

rm -rf "$mod_dir"
mkdir -p "$mod_dir/etc/init.d" "$mod_dir/etc/preinit.d" "$es_payload/defaults/themes/simple" "$es_payload/resources"
cp "$classicmini_dir/device/S8100es" "$mod_dir/etc/init.d/"
cp "$classicmini_dir/device/preinit/b8030_es" "$mod_dir/etc/preinit.d/"
cp "$classicmini_dir/out/emulationstation" "$classicmini_dir/out/libfreetype.so.6" "$classicmini_dir/out/padstate" "$es_payload/"
cp "$classicmini_dir"/device/es-*.sh "$classicmini_dir/device/es_systems.cfg" "$es_payload/"
cp -R "$classicmini_dir"/../resources/. "$es_payload/resources/"
rm -f "$es_payload"/resources/mame*.xml "$es_payload"/resources/*.py "$es_payload"/resources/splash.svg
cp -R "$classicmini_dir"/device/resources/. "$es_payload/resources/"
find "$es_payload/resources" -type f \( -name '*.svg' -o -name '*.xml' -o -name '*.txt' \) -exec sed -i 's/\r$//' {} +
cp -R "$theme_src/." "$es_payload/defaults/themes/simple/"
rm -f "$es_payload"/defaults/themes/simple/.rev_*
for collection in auto-allgames:"All Games" auto-lastplayed:"Last Played" auto-favorites:Favorites auto-random:Random custom-collections:Collections; do
  mkdir -p "$es_payload/defaults/themes/simple/${collection%%:*}"
  sed "s/@TITLE@/${collection#*:}/" "$classicmini_dir/device/collection_theme.xml" > "$es_payload/defaults/themes/simple/${collection%%:*}/theme.xml"
done
favorites_theme=$es_payload/defaults/themes/simple/auto-favorites
mkdir -p "$favorites_theme/art"
cp "$classicmini_dir/device/favorites.svg" "$favorites_theme/art/"
sed -i '$d' "$favorites_theme/theme.xml"
cat "$classicmini_dir/device/favorites_logo.xml" >> "$favorites_theme/theme.xml"
echo "</theme>" >> "$favorites_theme/theme.xml"
awk '/<fullname>/ { gsub(/.*<fullname>|<\/fullname>.*/, ""); name = $0 } /<theme>/ { gsub(/.*<theme>|<\/theme>.*/, ""); print $0 "|" name }' "$classicmini_dir/device/es_systems.cfg" | sort -u | while IFS='|' read -r folder title; do
  [ -d "$es_payload/defaults/themes/simple/$folder" ] && continue
  mkdir -p "$es_payload/defaults/themes/simple/$folder"
  sed "s/@TITLE@/$(printf '%s' "$title" | sed 's/[&\\]/\\&/g')/" "$classicmini_dir/device/collection_theme.xml" > "$es_payload/defaults/themes/simple/$folder/theme.xml"
done
tar -C "$es_payload/defaults/themes/simple" --owner=0 --group=0 --numeric-owner -cf "$es_payload/defaults/theme_simple.tar" .
rm -rf "$es_payload/defaults/themes"
cp "$classicmini_dir/device/es_settings.cfg" "$classicmini_dir/device/es_input.cfg" "$es_payload/defaults/"
cp "$script_dir"/files/install "$script_dir"/files/uninstall "$script_dir"/files/readme.md "$mod_dir/"
