#!/bin/sh
# Stops the stock UI, builds the library from the synced games and runs EmulationStation. A game picked in ES is run
# here, with ES closed, and ES is opened again when it ends. The stock UI comes back when ES is left, however it ends.
#
#   es-run.sh
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
  # the RetroArch child ends with uistart: once the stock UI is fully up, put it away again before ES comes back
  if [ -n "$(pidof clover-mcp)" ]; then
    for i in $(seq 1 25); do
      [ -n "$(pidof ReedPlayer-Clover)" ] && break
      usleep 200000
    done
    uistop
  fi
done
