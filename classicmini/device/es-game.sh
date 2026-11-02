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
# Runs the game a generated <CODE>.desktop describes: the Exec of the entry hakchi synced, in its Path when it exists,
# with the flags the stock supervisor adds: the game's save file, and the rollback/screenshot/time files of the suspend
# point it saves on quit. An entry with "ESResume=N" (see es-library.sh) also loads suspend point N.
# The emulator wrappers ask retroarch-clover to start RetroArch, which stops the UI and hands the command to remote-exec
# to run in the background. ES is closed while a game runs and the stock UI is already stopped, so a retroarch-clover that
# runs retroarch-clover-child (the part that does the saves, filters and ROM unpacking) straight away does the job.
#
#   es-game.sh <file.desktop>
shim=/tmp/es-shim
profile=/var/lib/clover/profiles/0
volatile=/var/cache/clover/volatile

desktop=$1
cmd=$(tr -d '\r' < "$desktop" | sed -n 's/^Exec=//p' | head -n 1)
path=$(tr -d '\r' < "$desktop" | sed -n 's/^Path=//p' | head -n 1)
code=$(tr -d '\r' < "$desktop" | sed -n 's/^Code=//p' | head -n 1)
resume=$(tr -d '\r' < "$desktop" | sed -n 's/^ESResume=//p' | head -n 1)
[ -n "$cmd" ] || exit 1
[ -d "$path" ] && cd "$path"

if [ -n "$code" ]; then
  out=$volatile/$code/suspendpoint0
  mkdir -p "$profile/$code" "$out/rollback"
  cmd="$cmd --save-data-backing-file $profile/$code/cartridge.sram --title-code $code --video-mode keep-aspect-ratio"
  cmd="$cmd --save-time-path $out/state.time --save-screenshot-on-quit $out/state.png"
  cmd="$cmd --rollback-mode record --rollback-output-dir $out/rollback/"
  if [ -n "$resume" ]; then
    sp=$profile/$code/suspendpoint$resume
    cmd="$cmd --load-time-path $sp//state.time --rollback-input-dir $sp/rollback/"
  fi
fi

mkdir -p "$shim"
printf '#!/bin/sh\nexec retroarch-clover-child "$@"\n' > "$shim/retroarch-clover"
chmod +x "$shim/retroarch-clover"

PATH="$shim:$PATH" sh -c "$cmd"
