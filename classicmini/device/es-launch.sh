#!/bin/sh
# ES's launch command. The wrappers hakchi uses for games call uistop/uistart, which terminate every process holding
# /dev/fb0, and ES keeps it open even with its window released. So the game does not run under ES: this records which one
# to run and closes ES; es-run.sh runs it and then opens ES again.
#
#   es-launch.sh <file.desktop>
echo "$1" > /tmp/es_next
kill $(pidof emulationstation)
